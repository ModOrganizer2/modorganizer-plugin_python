#pragma warning(disable : 4100)
#pragma warning(disable : 4996)

#include <tuple>
#include <variant>

#include "pybind11_all.h"

#include "wrappers/pyfiletree.h"
#include "wrappers/wrappers.h"

// TODO: remove these include (only for testing)
#include <QDir>
#include <QFile>
#include <QWidget>
#include <fmt/format.h>
#include <iplugin.h>
#include <iplugindiagnose.h>
#include <ipluginfilemapper.h>
#include <iplugingame.h>
#include <iplugininstaller.h>
#include <iplugininstallersimple.h>
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

    // game features must be added before plugins
    mo2::python::add_game_feature_bindings(m);

    mo2::python::add_plugins_bindings(m);

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

    // private stuff for debugging/test
    py::module_ moprivate(
        py::reinterpret_borrow<py::module_>(PyImport_AddModule("mobase.private")));
    m.attr("private") = moprivate;

    // expose a function to create a particular tree, only for debugging
    // purpose, not in mobase.
    mo2::python::add_make_tree_function(moprivate);
    moprivate.def("extract_plugins", &mo2::python::extract_plugins);

    // == BEGIN TESTS ==

#ifdef MOBASE_TESTS
    py::module_ motests(
        py::reinterpret_borrow<py::module_>(PyImport_AddModule("mobase.tests")));
    m.attr("tests") = motests;

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
            // std::cout << fmt::format(
            //     "    installer?: {}\n",
            //     (void*)qobject_cast<IPluginInstaller*>(qobjects[i]));
            // std::cout << fmt::format(
            //     "    installer simple?: {}\n",
            //     (void*)qobject_cast<IPluginInstallerSimple*>(qobjects[i]));

            if (IPluginGame* game = dynamic_cast<IPluginGame*>(plugin)) {
                auto saves = game->listSaves(QDir());
                std::cout << "    saves: " << saves.size() << "\n";
                for (auto& save : saves) {
                    std::cout << "      save: " << (void*)save.get() << ", "
                              << py::reinterpret_borrow<py::object>(py::cast(save))
                              << ", " << save->getFilepath().toStdString() << "\n";
                }
            }
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

#endif
}
