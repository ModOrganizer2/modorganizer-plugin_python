#include "pythonrunner.h"

#pragma warning(disable : 4100)
#pragma warning(disable : 4996)

#include <tuple>
#include <variant>

#include <Windows.h>

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QWidget>

#include <pybind11/embed.h>
#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl/filesystem.h>

#include "pybind11_qt/pybind11_qt.h"

#include <log.h>
#include <utility.h>

#include "error.h"
#include "pythonutils.h"
#include "wrappers/pyfiletree.h"
#include "wrappers/wrappers.h"

// TODO: remove these include (only for testing)
#include <iplugin.h>
#include <iplugindiagnose.h>
#include <ipluginfilemapper.h>
#include <iplugingame.h>
#include <iplugininstaller.h>
#include <ipluginlist.h>
#include <ipluginmodpage.h>
#include <ipluginpreview.h>
#include <iplugintool.h>
#include <isavegame.h>
#include <isavegameinfowidget.h>

using namespace MOBase;
namespace py = pybind11;

PYBIND11_MODULE(mobase, m)
{
    using namespace mo2::python;

    py::module_::import("PyQt6.QtCore");
    py::module_::import("PyQt6.QtWidgets");

    // bindings
    //
    mo2::python::add_basic_bindings(m);
    mo2::python::add_wrapper_bindings(m);
    mo2::python::add_plugins_bindings(m);
    mo2::python::add_game_feature_bindings(m);

    // widgets
    //
    py::module_ widgets(
        py::reinterpret_borrow<py::module_>(PyImport_AddModule("mobase.widgets")));
    m.attr("widgets") = widgets;
    mo2::python::add_widget_bindings(widgets);

    // functions
    //
    m.def("getFileVersion", &MOBase::getFileVersion, py::arg("filepath"));
    m.def("getProductVersion", &MOBase::getProductVersion, py::arg("executable"));
    m.def("getIconForExecutable", &MOBase::iconForExecutable, py::arg("executable"));

    // expose MoVariant - MoVariant is a fake object whose only purpose is to be
    // used as a type-hint on the python side (e.g., def foo(x:
    // mobase.MoVariant))
    //
    // the real MoVariant is defined in the generated stubs, since it's only
    // relevant when doing type-checking, but this needs to be defined,
    // otherwise MoVariant is not found when actually running plugins through
    // MO2, making them crash
    m.attr("MoVariant") = py::none();

    // == BEGIN TESTS ==

    m.def("testPlugin", [](py::object pyobj) {
        py::scoped_ostream_redirect s{std::cout};
        std::cout << "type: " << pyobj.get_type().attr("__name__").cast<std::string>()
                  << "\n";
        auto qobjects = mo2::python::extract_plugins(pyobj);
        std::cout << "  found " << qobjects.size() << " plugins\n";

        // cast as IPlugin
        for (int i = 0; i < qobjects.size(); ++i) {
            IPlugin* plugin = qobject_cast<IPlugin*>(qobjects[i]);
            std::cout << fmt::format("  plugin {}: {} -> {}\n", i, (void*)qobjects[i],
                                     (void*)plugin);
            std::cout << fmt::format("    name: {}\n", plugin->name().toStdString());
        }
    });

    py::detail::type_caster<QString> t1;
    py::detail::type_caster<QVariant> t2;

    m.def("testQStringList", [](QStringList const& list) {
        QStringList res = list;
        for (QString& value : res) {
            value = value + "_CPP";
        }
        return res;
    });

    m.def("testGuessedString", [](GuessedValue<QString> const& value) {
        return std::make_tuple(value.operator const QString&(), value.variants());
    });

    m.def("testQStringList", [](QStringList const& list) {
        QStringList res = list;
        for (QString& value : res) {
            value = value + "_CPP";
        }
        return res;
    });

    m.def("testQMap1", [](QMap<QString, QString> const& map) {
        QMap<QString, int> res;
        for (auto it = map.begin(); it != map.end(); ++it) {
            res[it.key()] = it.value().size();
        }
        return res;
    });

    m.def("testQMap2", [](QMap<QString, int> const& map) {
        QMap<QString, QString> res;
        for (auto it = map.begin(); it != map.end(); ++it) {
            res[it.key()] = QString::number(it.value());
        }
        return res;
    });

    m.def("testDateTime1", []() {
        return QDateTime::fromString("2022-02-15T12:33:45", Qt::ISODate);
    });

    m.def("testDateTime2", [](QDateTime const& datetime) {
        return datetime.toString();
    });

    m.def("testEnum0", []() {
        return Qt::GlobalColor::darkRed;
    });

    m.def("testEnum", [](Qt::GlobalColor color) {
        MOBase::log::error("In C++: {}", color);
        return py::make_tuple(color, QMessageBox::Icon::Information);
    });

    m.def("testPixmap", [](QPixmap const& pixmap) {
        return pixmap.size();
    });

    m.def("createSaveGame", []() -> ISaveGame* {
        class SaveGame : public ISaveGame {
            QString getFilepath() const override { return "filepath"; }
            QDateTime getCreationTime() const override
            {
                return QDateTime::fromString("2022-02-15T12:33:45", Qt::ISODate);
            }
            QString getName() const override { return "name"; }
            QString getSaveGroupIdentifier() const override { return "group"; }
            QStringList allFiles() const override { return {"file1", "file2"}; }
        };
        return new SaveGame();
    });

    m.def("testSaveGameWidget", [](ISaveGameInfoWidget* widget) {
        class SaveGame : public ISaveGame {
            QString getFilepath() const override { return "filepath-c++"; }
            QDateTime getCreationTime() const override
            {
                return QDateTime::fromString("2022-02-15T12:33:45", Qt::ISODate);
            }
            QString getName() const override { return "name"; }
            QString getSaveGroupIdentifier() const override { return "group"; }
            QStringList allFiles() const override { return {"file1", "file2"}; }
        };
        static SaveGame s;
        widget->setSave(s);
    });

    m.def("testSaveGameRef", [](const ISaveGame& game) {
        std::cout << "getFilepath(): " << game.getFilepath().toStdString() << "\n";
        std::cout << "getCreationTime(): "
                  << game.getCreationTime().toString().toStdString() << "\n";
        std::cout << "getName(): " << game.getName().toStdString() << "\n";
        std::cout << "getSaveGroupIdentifier(): "
                  << game.getSaveGroupIdentifier().toStdString() << "\n";
        std::cout << "allFiles(): [ " << game.allFiles().join(" ").toStdString()
                  << " ]\n";
    });

    m.def("testSaveGamePtr", [](const ISaveGame* game) {
        std::cout << "getFilepath(): " << game->getFilepath().toStdString() << "\n";
        std::cout << "getCreationTime(): "
                  << game->getCreationTime().toString().toStdString() << "\n";
        std::cout << "getName(): " << game->getName().toStdString() << "\n";
        std::cout << "getSaveGroupIdentifier(): "
                  << game->getSaveGroupIdentifier().toStdString() << "\n";
        std::cout << "allFiles(): [ " << game->allFiles().join(" ").toStdString()
                  << " ]\n";
    });

    m.def("testWidget1", [](QWidget* w) {
        if (w) {
            std::cout << "background role: " << w->backgroundRole() << "\n";
        }
        else {
            std::cout << "null widget\n";
        }
    });

    m.def("testWidget2", []() {
        QWidget* w = new QWidget();
        w->setBackgroundRole(QPalette::ColorRole::HighlightedText);
        return w;
    });

    m.def("testFlags1", [](IPluginList::PluginStates states) {
        std::vector<std::string> res;
        if (states.testFlag(IPluginList::STATE_MISSING)) {
            res.push_back("missing");
        }
        if (states.testFlag(IPluginList::STATE_INACTIVE)) {
            res.push_back("inactive");
        }
        if (states.testFlag(IPluginList::STATE_ACTIVE)) {
            res.push_back("active");
        }
        return res;
    });

    m.def("testFlags2", [](QStringList const& states) {
        IPluginList::PluginStates res;
        if (states.contains("missing")) {
            res |= IPluginList::STATE_MISSING;
        }
        if (states.contains("inactive")) {
            res |= IPluginList::STATE_INACTIVE;
        }
        if (states.contains("active")) {
            res |= IPluginList::STATE_ACTIVE;
        }
        return res;
    });
}

/**
 *
 */
class PythonRunner : public IPythonRunner {

public:
    PythonRunner();
    ~PythonRunner();

    bool initPython();

    QList<QObject*> load(const QString& identifier);
    void unload(const QString& identifier);

    bool isPythonInitialized() const;
    bool isPythonVersionSupported() const;

private:
    void initPath();

    /**
     * @brief Ensure that the given folder is in sys.path.
     */
    void ensureFolderInPath(QString folder);

private:
    // For each "identifier" (python file or python module folder), contains the
    // list of python objects to keep "alive" during the execution.
    std::unordered_map<QString, std::vector<py::object>> m_PythonObjects;
};

IPythonRunner* CreatePythonRunner()
{
    std::unique_ptr<PythonRunner> result = std::make_unique<PythonRunner>();
    if (result->initPython()) {
        return result.release();
    }
    else {
        return nullptr;
    }
}

PythonRunner::PythonRunner() {}

PythonRunner::~PythonRunner()
{
    // We need the GIL lock when destroying Python objects.
    py::gil_scoped_acquire lock;
    // m_Interpreter.reset();

    // Boost.Python does not handle cyclic garbace collection, so we need to
    // release everything hold by the objects before deleting the objects
    // themselves: for (auto& [name, objects] : m_PythonObjects) {
    //   for (auto& obj : objects) {
    //     obj.attr("__dict__").attr("clear")();
    //   }
    // }

    // m_PythonObjects.clear();
}

// ErrWrapper is in error.h

PYBIND11_MODULE(moprivate, m)
{
    // expose a function to create a particular tree, only for debugging
    // purpose, not in mobase.
    mo2::python::add_make_tree_function(m);
}

bool PythonRunner::initPython()
{
    if (Py_IsInitialized())
        return true;
    try {
        static const char* argv0 = "ModOrganizer.exe";

        // we initialize the interpreter "the old way" because
        // scoped_interpreter does not seem to work well with PyQt
        wchar_t argBuffer[MAX_PATH];
        const size_t cSize = strlen(argv0) + 1;
        mbstowcs(argBuffer, argv0, MAX_PATH);
        Py_SetProgramName(argBuffer);

        PyImport_AppendInittab("mobase", &PyInit_mobase);
        PyImport_AppendInittab("moprivate", &PyInit_moprivate);

        Py_OptimizeFlag = 2;
        Py_NoSiteFlag   = 1;

        initPath();
        Py_InitializeEx(0);

        if (!Py_IsInitialized()) {
            return false;
        }

        py::module_ mainModule     = py::module_::import("__main__");
        py::object mainNamespace   = mainModule.attr("__dict__");
        mainNamespace["sys"]       = py::module_::import("sys");
        mainNamespace["moprivate"] = py::module_::import("moprivate");
        mainNamespace["mobase"]    = py::module_::import("mobase");

        mo2::python::configure_python_stream();
        mo2::python::configure_python_logging(mainNamespace["mobase"]);

        return true;
    }
    catch (const py::error_already_set& ex) {
        MOBase::log::error("failed to init python: {}", ex.what());
        return false;
    }
}

void PythonRunner::initPath()
{
    static QStringList paths = {QCoreApplication::applicationDirPath() +
                                    "/pythoncore.zip",
                                QCoreApplication::applicationDirPath() + "/pythoncore",
                                IOrganizer::getPluginDataPath()};

    Py_SetPath(paths.join(';').toStdWString().c_str());
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

        // Dictionary that will contain createPlugin() or createPlugins().
        py::dict moduleDict;

        if (identifier.endsWith(".py")) {
            py::object mainModule    = py::module_::import("__main__");
            py::dict moduleNamespace = mainModule.attr("__dict__");

            std::string temp = ToString(identifier);
            py::eval_file(temp.c_str(), moduleNamespace).is_none();
            moduleDict = moduleNamespace;
        }
        else {
            // Retrieve the module name:
            QStringList parts      = identifier.split("/");
            std::string moduleName = ToString(parts.takeLast());
            ensureFolderInPath(parts.join("/"));
            moduleDict = py::module_::import(moduleName.c_str()).attr("__dict__");
        }

        if (py::len(moduleDict) == 0) {
            MOBase::log::error("No plugins found in {}.", identifier);
            return {};
        }

        // Create the plugins:
        std::vector<py::object> plugins;

        if (moduleDict.contains("createPlugin")) {
            plugins.push_back(moduleDict["createPlugin"]());

            // Clear for future call
            PyDict_DelItemString(moduleDict.ptr(), "createPlugin");
        }
        else if (moduleDict.contains("createPlugins")) {
            py::object pyPlugins = moduleDict["createPlugins"]();
            if (!PySequence_Check(pyPlugins.ptr())) {
                MOBase::log::error("Plugin {}: createPlugins must return a list.",
                                   identifier);
            }
            else {
                py::list pyList(pyPlugins);
                int nPlugins = py::len(pyList);
                for (int i = 0; i < nPlugins; ++i) {
                    plugins.push_back(pyList[i]);
                }
            }

            // Clear for future call
            PyDict_DelItemString(moduleDict.ptr(), "createPlugins");
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

            // Add the plugin to keep it alive:
            m_PythonObjects[identifier].push_back(pluginObj);

            QList<QObject*> interfaceList = mo2::python::extract_plugins(pluginObj);

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
                    QString mpath = mod.attr("__path__")[0].cast<QString>();

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

bool PythonRunner::isPythonInitialized() const
{
    return Py_IsInitialized() != 0;
}
