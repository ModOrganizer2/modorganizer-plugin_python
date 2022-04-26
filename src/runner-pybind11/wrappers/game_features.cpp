#include "wrappers.h"

#include <tuple>

#include <pybind11/functional.h>
#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "../pybind11_qt/pybind11_qt.h"

#include <ipluginlist.h>
#include <isavegameinfowidget.h>

#include <bsainvalidation.h>
#include <dataarchives.h>
#include <gameplugins.h>
#include <localsavegames.h>
#include <moddatachecker.h>
#include <moddatacontent.h>
#include <savegameinfo.h>
#include <scriptextender.h>
#include <unmanagedmods.h>

#include "pyfiletree.h"

namespace py = pybind11;
using namespace MOBase;
using namespace pybind11::literals;

namespace mo2::python {

    class PyBSAInvalidation : public BSAInvalidation {
    public:
        bool isInvalidationBSA(const QString& bsaName) override
        {
            PYBIND11_OVERRIDE_PURE(bool, BSAInvalidation, isInvalidationBSA, bsaName);
        }
        void deactivate(MOBase::IProfile* profile) override
        {
            PYBIND11_OVERRIDE_PURE(void, BSAInvalidation, deactivate, profile);
        }
        void activate(MOBase::IProfile* profile) override
        {
            PYBIND11_OVERRIDE_PURE(void, BSAInvalidation, activate, profile);
        }
        bool prepareProfile(MOBase::IProfile* profile) override
        {
            PYBIND11_OVERRIDE_PURE(bool, BSAInvalidation, prepareProfile, profile);
        }
    };

    class PyDataArchives : public DataArchives {
    public:
        QStringList vanillaArchives() const override
        {
            PYBIND11_OVERRIDE_PURE(QStringList, DataArchives, vanillaArchives, );
        }
        QStringList archives(const MOBase::IProfile* profile) const override
        {
            PYBIND11_OVERRIDE_PURE(QStringList, DataArchives, archives, profile);
        }
        void addArchive(MOBase::IProfile* profile, int index,
                        const QString& archiveName) override
        {
            PYBIND11_OVERRIDE_PURE(void, DataArchives, addArchive, profile, index,
                                   archiveName);
        }
        void removeArchive(MOBase::IProfile* profile,
                           const QString& archiveName) override
        {
            PYBIND11_OVERRIDE_PURE(void, DataArchives, removeArchive, profile,
                                   archiveName);
        }
    };

    class PyGamePlugins : public GamePlugins {
    public:
        void writePluginLists(const MOBase::IPluginList* pluginList) override
        {
            PYBIND11_OVERRIDE_PURE(void, GamePlugins, writePluginLists, pluginList);
        }
        void readPluginLists(MOBase::IPluginList* pluginList) override
        {
            // TODO: cannot update plugin list or create one from Python so this is
            // useless
            PYBIND11_OVERRIDE_PURE(void, GamePlugins, readPluginLists, pluginList);
        }
        QStringList getLoadOrder() override
        {
            PYBIND11_OVERRIDE_PURE(QStringList, GamePlugins, getLoadOrder, );
        }
        bool lightPluginsAreSupported() override
        {
            PYBIND11_OVERRIDE_PURE(bool, GamePlugins, lightPluginsAreSupported, );
        }
    };

    class PyLocalSavegames : public LocalSavegames {
    public:
        MappingType mappings(const QDir& profileSaveDir) const override
        {
            PYBIND11_OVERRIDE_PURE(MappingType, LocalSavegames, mappings,
                                   profileSaveDir);
        }
        bool prepareProfile(MOBase::IProfile* profile) override
        {
            PYBIND11_OVERRIDE_PURE(bool, LocalSavegames, prepareProfile, profile);
        }
    };

    class PyModDataChecker : public ModDataChecker {
    public:
        CheckReturn
        dataLooksValid(std::shared_ptr<const MOBase::IFileTree> fileTree) const override
        {
            PYBIND11_OVERRIDE_PURE(CheckReturn, ModDataChecker, dataLooksValid,
                                   fileTree);
        }

        std::shared_ptr<MOBase::IFileTree>
        fix(std::shared_ptr<MOBase::IFileTree> fileTree) const override
        {
            PYBIND11_OVERRIDE(std::shared_ptr<MOBase::IFileTree>, ModDataChecker, fix,
                              fileTree);
        }
    };

    class PyModDataContent : public ModDataContent {
    public:
        std::vector<Content> getAllContents() const override
        {
            PYBIND11_OVERRIDE_PURE(std::vector<Content>, ModDataContent,
                                   getAllContents, );
            ;
        }
        std::vector<int>
        getContentsFor(std::shared_ptr<const MOBase::IFileTree> fileTree) const override
        {
            PYBIND11_OVERRIDE_PURE(std::vector<int>, ModDataContent, getContentsFor,
                                   fileTree);
        }
    };

    class PySaveGameInfo : public SaveGameInfo {
    public:
        MissingAssets getMissingAssets(MOBase::ISaveGame const& save) const override
        {
            PYBIND11_OVERRIDE_PURE(MissingAssets, SaveGameInfo, getMissingAssets,
                                   &save);
        }
        ISaveGameInfoWidget* getSaveGameWidget(QWidget* parent = 0) const override
        {
            // TODO: transfer ownership
            PYBIND11_OVERRIDE_PURE(ISaveGameInfoWidget*, SaveGameInfo,
                                   getSaveGameWidget, parent);
        }
    };

    class PyScriptExtender : public ScriptExtender {
    public:
        QString BinaryName() const override
        {
            PYBIND11_OVERRIDE_PURE(QString, ScriptExtender, BinaryName, );
        }

        QString PluginPath() const override
        {
            PYBIND11_OVERRIDE_PURE(QString, ScriptExtender, PluginPath, );
        }

        QString loaderName() const override
        {
            PYBIND11_OVERRIDE_PURE(QString, ScriptExtender, loaderName, );
        }

        QString loaderPath() const override
        {
            PYBIND11_OVERRIDE_PURE(QString, ScriptExtender, loaderPath, );
        }

        QString savegameExtension() const override
        {
            PYBIND11_OVERRIDE_PURE(QString, ScriptExtender, savegameExtension, );
        }

        bool isInstalled() const override
        {
            PYBIND11_OVERRIDE_PURE(bool, ScriptExtender, isInstalled, );
        }

        QString getExtenderVersion() const override
        {
            PYBIND11_OVERRIDE_PURE(QString, ScriptExtender, getExtenderVersion, );
        }

        WORD getArch() const override
        {
            PYBIND11_OVERRIDE_PURE(WORD, ScriptExtender, getArch, );
        }
    };

    class PyPyUnmanagedMods : public UnmanagedMods {
    public:
        QStringList mods(bool onlyOfficial) const override
        {
            PYBIND11_OVERRIDE_PURE(QStringList, UnmanagedMods, mods, onlyOfficial);
        }
        QString displayName(const QString& modName) const override
        {
            PYBIND11_OVERRIDE_PURE(QString, UnmanagedMods, displayName, modName);
        }
        QFileInfo referenceFile(const QString& modName) const override
        {
            PYBIND11_OVERRIDE_PURE(QFileInfo, UnmanagedMods, referenceFile, modName);
        }
        QStringList secondaryFiles(const QString& modName) const override
        {
            PYBIND11_OVERRIDE_PURE(QStringList, UnmanagedMods, secondaryFiles, modName);
        }
    };

    void add_game_feature_bindings(pybind11::module_ m)
    {
        // BSAInvalidation

        py::class_<BSAInvalidation, PyBSAInvalidation>(m, "BSAInvalidation")
            .def(py::init<>())
            .def("isInvalidationBSA", &BSAInvalidation::isInvalidationBSA, "name"_a)
            .def("deactivate", &BSAInvalidation::deactivate, "profile"_a)
            .def("activate", &BSAInvalidation::activate, "profile"_a);

        // DataArchives

        py::class_<DataArchives, PyDataArchives>(m, "DataArchives")
            .def(py::init<>())
            .def("vanillaArchives", &DataArchives::vanillaArchives)
            .def("archives", &DataArchives::archives, "profile"_a)
            .def("addArchive", &DataArchives::addArchive,
                 ("profile"_a, "index", "name"))
            .def("removeArchive", &DataArchives::removeArchive, "profile"_a, "name"_a);

        // GamePlugins

        py::class_<GamePlugins, PyGamePlugins>(m, "GamePlugins")
            .def(py::init<>())
            .def("writePluginLists", &GamePlugins::writePluginLists, "plugin_list"_a)
            .def("readPluginLists", &GamePlugins::readPluginLists, "plugin_list"_a)
            .def("getLoadOrder", &GamePlugins::getLoadOrder)
            .def("lightPluginsAreSupported", &GamePlugins::lightPluginsAreSupported);

        // LocalSavegames

        py::class_<LocalSavegames, PyLocalSavegames>(m, "LocalSavegames")
            .def(py::init<>())
            .def("mappings", &LocalSavegames::mappings, "profile_save_dir"_a)
            .def("prepareProfile", &LocalSavegames::prepareProfile, "profile"_a);

        // ModDataChecker

        py::class_<ModDataChecker, PyModDataChecker> pyModDataChecker(m,
                                                                      "ModDataChecker");

        py::enum_<ModDataChecker::CheckReturn>(pyModDataChecker, "CheckReturn")
            .value("INVALID", ModDataChecker::CheckReturn::INVALID)
            .value("FIXABLE", ModDataChecker::CheckReturn::FIXABLE)
            .value("VALID", ModDataChecker::CheckReturn::VALID)
            .export_values();

        pyModDataChecker.def(py::init<>())
            .def("dataLooksValid", &ModDataChecker::dataLooksValid, "filetree"_a)
            .def("fix", &ModDataChecker::fix, "filetree"_a);

        // ModDataContent
        py::class_<ModDataContent, PyModDataContent> pyModDataContent(m,
                                                                      "ModDataContent");

        py::class_<ModDataContent::Content>(pyModDataContent, "Content")
            .def(py::init<int, QString, QString, bool>(), "id"_a, "name"_a, "icon"_a,
                 "filter_only"_a = false)
            .def_property_readonly("id", &ModDataContent::Content::id)
            .def_property_readonly("name", &ModDataContent::Content::name)
            .def_property_readonly("icon", &ModDataContent::Content::icon)
            .def("isOnlyForFilter", &ModDataContent::Content::isOnlyForFilter);

        pyModDataContent.def(py::init<>())
            .def("getAllContents", &ModDataContent::getAllContents)
            .def("getContentsFor", &ModDataContent::getContentsFor, "filetree"_a);

        // SaveGameInfo

        py::class_<SaveGameInfo, PySaveGameInfo>(m, "SaveGameInfo")
            .def(py::init<>())
            .def("getMissingAssets", &SaveGameInfo::getMissingAssets, "save"_a)
            .def("getSaveGameWidget", &SaveGameInfo::getSaveGameWidget,
                 py::return_value_policy::reference, "parent"_a, "[optional]");

        // ScriptExtender

        py::class_<ScriptExtender, PyScriptExtender>(m, "ScriptExtender")
            .def(py::init<>())
            .def("BinaryName", &ScriptExtender::BinaryName)
            .def("PluginPath", &ScriptExtender::PluginPath)
            .def("loaderName", &ScriptExtender::loaderName)
            .def("loaderPath", &ScriptExtender::loaderPath)
            .def("savegameExtension", &ScriptExtender::savegameExtension)
            .def("isInstalled", &ScriptExtender::isInstalled)
            .def("getExtenderVersion", &ScriptExtender::getExtenderVersion)
            .def("getArch", &ScriptExtender::getArch);

        // UnmanagedMods

        py::class_<UnmanagedMods, PyPyUnmanagedMods>(m, "UnmanagedMods")
            .def(py::init<>())
            .def("mods", &UnmanagedMods::mods, "official_only"_a)
            .def("displayName", &UnmanagedMods::displayName, "mod_name"_a)
            .def("referenceFile", &UnmanagedMods::referenceFile, "mod_name"_a)
            .def("secondaryFiles", &UnmanagedMods::secondaryFiles, "mod_name"_a);
    }

}  // namespace mo2::python

namespace mo2::python {

    class GameFeaturesHelper {
        using GameFeatures = std::tuple<
            // BSAInvalidation, DataArchives, GamePlugins, LocalSavegames,
            ModDataChecker,
            // ModDataContent, SaveGameInfo,
            ScriptExtender
            // , UnmanagedMods
            >;

        template <class F, std::size_t... Is>
        static void helper(F&& f, std::index_sequence<Is...>)
        {
            (f(static_cast<std::tuple_element_t<Is, GameFeatures>*>(nullptr)), ...);
        }

    public:
        // apply the function f on a null-pointer of type Feature* for each game
        // feature
        template <class F>
        static void apply(F&& f)
        {
            helper(f, std::make_index_sequence<std::tuple_size_v<GameFeatures>>{});
        }
    };

    pybind11::object extract_feature(IPluginGame const& game, pybind11::object type)
    {
        py::object py_feature = py::none();
        GameFeaturesHelper::apply([&]<class Feature>(Feature* feature) {
            if (py::type::of<Feature>().is(type)) {
                py_feature = py::cast(game.feature<Feature>(),
                                      py::return_value_policy::reference);
            }
        });
        return py_feature;
    }

    pybind11::dict extract_feature_list(IPluginGame const& game)
    {
        // constructing a dict from class name to actual object
        py::dict dict;
        GameFeaturesHelper::apply([&]<class Feature>(Feature* feature) {
            dict[py::type::of<Feature>()] =
                py::cast(game.feature<Feature>(), py::return_value_policy::reference);
        });
        return dict;
    }

    std::map<std::type_index, std::any>
    convert_feature_list(py::dict const& py_features)
    {
        std::map<std::type_index, std::any> features;
        GameFeaturesHelper::apply([&]<class Feature>(Feature* feature) {
            const auto py_type = py::type::of<Feature>();
            if (py_features.contains(py_type)) {
                features[std::type_index(typeid(Feature))] =
                    py_features[py_type].cast<Feature*>();
            }
        });
        return features;
    }

}  // namespace mo2::python
