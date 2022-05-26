#include "pythonrunner.h"

#pragma warning(disable : 4100)
#pragma warning(disable : 4996)

#include <Windows.h>

#include <algorithm>

#include <QCoreApplication>
#include <QDir>
#include <QFile>

#include "pybind11_qt/pybind11_qt.h"
#include <pybind11/embed.h>
#include <pybind11/functional.h>
#include <pybind11/stl/filesystem.h>

#include <log.h>
#include <utility.h>

#include "error.h"
#include "pythonutils.h"

using namespace MOBase;
namespace py = pybind11;

namespace mo2::python {

    /**
     *
     */
    class PythonRunner : public IPythonRunner {

    public:
        PythonRunner()  = default;
        ~PythonRunner() = default;

        QList<QList<QObject*>> load(std::string_view moduleName,
                                    std::filesystem::path const& modulePath) override;
        void unload(std::string_view moduleName,
                    std::filesystem::path const& modulePath) override;

        bool initialize(std::vector<std::filesystem::path> const& pythonPaths) override;
        void addDllSearchPath(std::filesystem::path const& dllPath) override;
        bool isInitialized() const override;
    };

    std::unique_ptr<IPythonRunner> createPythonRunner()
    {
        return std::make_unique<PythonRunner>();
    }

    bool PythonRunner::initialize(std::vector<std::filesystem::path> const& pythonPaths)
    {
        // we only initialize Python once for the whole lifetime of the program, even if
        // MO2 is restarted and the proxy or PythonRunner objects are deleted and
        // recreated, Python is not re-initialized
        //
        // in an ideal world, we would initialize Python here (or in the constructor)
        // and then finalize it in the destructor
        //
        // unfortunately, many library, including PyQt6, do not handle properly
        // re-initializing the Python interpreter, so we cannot do that and we keep the
        // interpreter alive
        //

        if (Py_IsInitialized()) {
            return true;
        }

        try {
            static const char* argv0 = "ModOrganizer.exe";

            // initialize the core Path of Python, this must be done before
            // initialization
            //
            if (!pythonPaths.empty()) {
                QStringList paths;
                for (auto const& p : pythonPaths) {
                    paths.append(QString::fromStdWString(absolute(p).native()));
                }
                Py_SetPath(paths.join(';').toStdWString().c_str());
            }

            Py_OptimizeFlag = 2;
            Py_NoSiteFlag   = 1;

            py::initialize_interpreter(false, 1, &argv0);

            if (!Py_IsInitialized()) {
                MOBase::log::error(
                    "failed to init python: failed to initialize interpreter.");

                if (PyGILState_Check()) {
                    PyEval_SaveThread();
                }

                return false;
            }

            py::module_ mainModule   = py::module_::import("__main__");
            py::object mainNamespace = mainModule.attr("__dict__");
            mainNamespace["sys"]     = py::module_::import("sys");
            mainNamespace["mobase"]  = py::module_::import("mobase");

            mo2::python::configure_python_stream();
            mo2::python::configure_python_logging(mainNamespace["mobase"]);

            // we need to release the GIL here - which is what this does
            //
            // when Python is initialized, the GIl is acquired, and if it is not
            // release, trying to acquire it on a different thread will deadlock
            PyEval_SaveThread();

            return true;
        }
        catch (const py::error_already_set& ex) {
            MOBase::log::error("failed to init python: {}", ex.what());
            return false;
        }
    }

    void PythonRunner::addDllSearchPath(std::filesystem::path const& dllPath)
    {
        py::gil_scoped_acquire lock;
        py::module_::import("os").attr("add_dll_directory")(absolute(dllPath));
    }

    QList<QList<QObject*>> PythonRunner::load(std::string_view name,
                                              const std::filesystem::path& pythonModule)
    {
        py::gil_scoped_acquire lock;

        try {

            // some needed import
            auto sys            = py::module_::import("sys");
            auto importlib_util = py::module_::import("importlib.util");

            // check if the module is already loaded
            py::dict modules = sys.attr("modules");
            py::module_ pymodule;
            if (modules.contains(name)) {
                pymodule = modules[py::str(name)];
                pymodule.reload();
            }
            else {
                // load the module
                auto spec =
                    importlib_util.attr("spec_from_file_location")(name, pythonModule);
                pymodule = importlib_util.attr("module_from_spec")(spec);
                sys.attr("modules")[py::str(name)] = pymodule;
                spec.attr("loader").attr("exec_module")(pymodule);
            }

            py::dict moduleDict = pymodule.attr("__dict__");

            if (py::len(moduleDict) == 0) {
                MOBase::log::error("no plugins found in {}", pythonModule);
                return {};
            }

            // Create the plugins:
            std::vector<py::object> plugins;

            if (moduleDict.contains("createPlugin")) {
                plugins.push_back(moduleDict["createPlugin"]());
            }
            else if (moduleDict.contains("createPlugins")) {
                py::object pyPlugins = moduleDict["createPlugins"]();
                if (!py::isinstance<py::sequence>(pyPlugins)) {
                    MOBase::log::error("{}: createPlugins must return a sequence",
                                       pythonModule);
                }
                else {
                    py::sequence pyList(pyPlugins);
                    size_t nPlugins = pyList.size();
                    for (size_t i = 0; i < nPlugins; ++i) {
                        plugins.push_back(pyList[i]);
                    }
                }
            }
            else {
                MOBase::log::error("{}: missing createPlugin(s) function",
                                   pythonModule);
            }

            // if we have no plugins, there was an issue, and we already logged the
            // problem
            if (plugins.empty()) {
                return {};
            }

            QList<QList<QObject*>> allInterfaceList;

            for (py::object pluginObj : plugins) {
                QList<QObject*> interfaceList = py::module_::import("mobase.private")
                                                    .attr("extract_plugins")(pluginObj)
                                                    .cast<QList<QObject*>>();

                if (interfaceList.isEmpty()) {
                    MOBase::log::error("{}: no plugin interface implemented.",
                                       pythonModule);
                }

                // Append the plugins to the main list:
                allInterfaceList.append(interfaceList);
            }

            return allInterfaceList;
        }
        catch (const py::error_already_set& ex) {
            MOBase::log::error("failed to import plugin from {}", pythonModule);
            throw pyexcept::PythonError(ex);
        }
    }

    void PythonRunner::unload(std::string_view moduleName,
                              std::filesystem::path const& modulePath)
    {
        py::gil_scoped_acquire lock;

        // At this point, the identifier is the full path to the module.
        QDir folder(modulePath);

        // we want to "unload" (remove from sys.modules) modules that come
        // from this plugin (whose __path__ points under this module,
        // including the module of the plugin itself)
        py::object sys   = py::module_::import("sys");
        py::dict modules = sys.attr("modules");
        py::list keys    = modules.attr("keys")();
        for (std::size_t i = 0; i < py::len(keys); ++i) {
            py::object mod = modules[keys[i]];
            if (PyObject_HasAttrString(mod.ptr(), "__path__")) {
                QString mpath = mod.attr("__path__")[py::int_(0)].cast<QString>();

                if (!folder.relativeFilePath(mpath).startsWith("..")) {
                    // if the path is under identifier, we need to unload it
                    log::debug("unloading module {} from {}",
                               keys[i].cast<std::string>(), mpath);

                    PyDict_DelItem(modules.ptr(), keys[i].ptr());
                }
            }
        }
    }

    bool PythonRunner::isInitialized() const
    {
        return Py_IsInitialized() != 0;
    }

}  // namespace mo2::python
