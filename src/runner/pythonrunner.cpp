#include "pythonrunner.h"

#pragma warning(disable : 4100)
#pragma warning(disable : 4996)

#include <Windows.h>

#include <QCoreApplication>
#include <QDir>
#include <QFile>

#include "pybind11_qt/pybind11_qt.h"
#include <pybind11/embed.h>
#include <pybind11/functional.h>

#include <log.h>
#include <utility.h>

#include "error.h"
#include "pythonutils.h"

using namespace MOBase;
namespace py = pybind11;

/**
 *
 */
class PythonRunner : public IPythonRunner {

public:
    PythonRunner()  = default;
    ~PythonRunner() = default;

    QList<QObject*> load(const QString& identifier) override;
    void unload(const QString& identifier) override;

    bool initialize(QStringList const& paths) override;
    bool isInitialized() const override;

private:
    /**
     * @brief Ensure that the given folder is in sys.path.
     */
    void ensureFolderInPath(QString folder);

private:
    // for each "identifier" (python file or python module folder), contains the
    // list of python objects - this does not keep the objects alive, it simply used to
    // unload plugins
    std::unordered_map<QString, std::vector<py::handle>> m_PythonObjects;
};

IPythonRunner* CreatePythonRunner()
{
    return new PythonRunner();
}

bool PythonRunner::initialize(QStringList const& paths)
{
    // we only initialize Python once for the whole lifetime of the program, even if MO2
    // is restarted and the proxy or PythonRunner objects are deleted and recreated,
    // Python is not re-initialized
    //
    // in an ideal world, we would initialize Python here (or in the constructor) and
    // then finalize it in the destructor
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

        // initialize the core Path of Python, this must be done before initialization
        //
        if (!paths.isEmpty()) {
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
        // when Python is initialized, the GIl is acquired, and if it is not release,
        // trying to acquire it on a different thread will deadlock
        PyEval_SaveThread();

        return true;
    }
    catch (const py::error_already_set& ex) {
        MOBase::log::error("failed to init python: {}", ex.what());
        return false;
    }
}

void PythonRunner::ensureFolderInPath(QString folder)
{
    py::module_ sys  = py::module_::import("sys");
    py::list sysPath = sys.attr("path");

    // Converting to QStringList for Qt::CaseInsensitive and because .index()
    // raise an exception:
    const QStringList currentPath = sysPath.cast<QStringList>();
    if (!currentPath.contains(folder, Qt::CaseInsensitive)) {
        sysPath.insert(0, folder);
    }
}

QList<QObject*> PythonRunner::load(const QString& identifier)
{
    py::gil_scoped_acquire lock;

    // `pluginName` can either be a python file (single-file plugin or a folder
    // (whole module).
    //
    // For whole module, we simply add the parent folder to path, then we load
    // the module with a simple py::import, and we retrieve the associated
    // __dict__ from which we extract either createPlugin or createPlugins.
    //
    // For single file, we need to use py::eval_file, and we will use the
    // context (global variables) from __main__ (already contains mobase, and
    // other required module). Since the context is shared between called of
    // `instantiate`, we need to make sure to remove createPlugin(s) from
    // previous call.
    try {

        // dictionary that will contain createPlugin() or createPlugins().
        py::dict moduleDict;

        if (identifier.endsWith(".py")) {
            py::object mainModule = py::module_::import("__main__");

            // make a copy, otherwise we might end up calling the createPlugin() or
            // createPlugins() function multiple time
            py::dict moduleNamespace = mainModule.attr("__dict__").attr("copy")();

            std::string temp = ToString(identifier);
            py::eval_file(temp, moduleNamespace).is_none();
            moduleDict = moduleNamespace;
        }
        else {
            // Retrieve the module name:
            QStringList parts      = identifier.split("/");
            std::string moduleName = ToString(parts.takeLast());
            ensureFolderInPath(parts.join("/"));

            // check if the module is already loaded
            py::dict modules = py::module_::import("sys").attr("modules");
            if (modules.contains(moduleName)) {
                py::module_ prev = modules[py::str(moduleName)];
                py::module_(prev).reload();
                moduleDict = prev.attr("__dict__");
            }
            else {
                moduleDict = py::module_::import(moduleName.c_str()).attr("__dict__");
            }
        }

        if (py::len(moduleDict) == 0) {
            MOBase::log::error("No plugins found in {}.", identifier);
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
                MOBase::log::error("Plugin {}: createPlugins must return a sequence.",
                                   identifier);
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
            MOBase::log::error("Plugin {}: missing a createPlugin(s) function.",
                               identifier);
        }

        // If we have no plugins, there was an issue, and we already logged the
        // problem:
        if (plugins.empty()) {
            return QList<QObject*>();
        }

        QList<QObject*> allInterfaceList;

        for (py::object pluginObj : plugins) {

            // save to be able to unload it
            m_PythonObjects[identifier].push_back(pluginObj);

            QList<QObject*> interfaceList = py::module_::import("mobase.private")
                                                .attr("extract_plugins")(pluginObj)
                                                .cast<QList<QObject*>>();

            if (interfaceList.isEmpty()) {
                MOBase::log::error("Plugin {}: no plugin interface implemented.",
                                   identifier);
            }

            // Append the plugins to the main list:
            allInterfaceList.append(interfaceList);
        }

        return allInterfaceList;
    }
    catch (const py::error_already_set& ex) {
        MOBase::log::error("Failed to import plugin from {}.", identifier);
        throw pyexcept::PythonError(ex);
    }
}

void PythonRunner::unload(const QString& identifier)
{
    auto it = m_PythonObjects.find(identifier);
    if (it != m_PythonObjects.end()) {

        py::gil_scoped_acquire lock;

        if (!identifier.endsWith(".py")) {

            // At this point, the identifier is the full path to the module.
            QDir folder(identifier);

            // We want to "unload" (remove from sys.modules) modules that come
            // from this plugin (whose __path__ points under this module,
            // including the module of the plugin itself).
            py::object sys   = py::module_::import("sys");
            py::dict modules = sys.attr("modules");
            py::list keys    = modules.attr("keys")();
            for (std::size_t i = 0; i < py::len(keys); ++i) {
                py::object mod = modules[keys[i]];
                if (PyObject_HasAttrString(mod.ptr(), "__path__")) {
                    QString mpath = mod.attr("__path__")[py::int_(0)].cast<QString>();

                    if (!folder.relativeFilePath(mpath).startsWith("..")) {
                        // If the path is under identifier, we need to unload
                        // it.
                        log::debug("Unloading module {} from {} for {}.",
                                   keys[i].cast<std::string>(), mpath, identifier);

                        PyDict_DelItem(modules.ptr(), keys[i].ptr());
                    }
                }
            }
        }

        // Boost.Python does not handle cyclic garbace collection, so we need to
        // release everything hold by the objects before deleting the objects
        // themselves (done when erasing from m_PythonObjects).
        for (auto& obj : it->second) {
            obj.attr("__dict__").attr("clear")();
        }

        log::debug("Deleting {} python objects for {}.", it->second.size(), identifier);
        m_PythonObjects.erase(it);
    }
}

bool PythonRunner::isInitialized() const
{
    return Py_IsInitialized() != 0;
}
