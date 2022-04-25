#include "wrappers.h"

#include "pyplugins.h"

namespace py = pybind11;
using namespace pybind11::literals;
using namespace MOBase;

namespace mo2::python {

    // this one is kind of big so it has its own function
    void add_iplugingame_bindings(pybind11::module_ m)
    {

        py::enum_<MOBase::IPluginGame::LoadOrderMechanism>(m, "LoadOrderMechanism")
            .value("FileTime", MOBase::IPluginGame::LoadOrderMechanism::FileTime)
            .value("PluginsTxt", MOBase::IPluginGame::LoadOrderMechanism::PluginsTxt)

            .value("FILE_TIME", MOBase::IPluginGame::LoadOrderMechanism::FileTime)
            .value("PLUGINS_TXT", MOBase::IPluginGame::LoadOrderMechanism::PluginsTxt);

        py::enum_<MOBase::IPluginGame::SortMechanism>(m, "SortMechanism")
            .value("NONE", MOBase::IPluginGame::SortMechanism::NONE)
            .value("MLOX", MOBase::IPluginGame::SortMechanism::MLOX)
            .value("BOSS", MOBase::IPluginGame::SortMechanism::BOSS)
            .value("LOOT", MOBase::IPluginGame::SortMechanism::LOOT);

        // this does not actually do the conversion, but might be convenient
        // for accessing the names for enum bits
        py::enum_<MOBase::IPluginGame::ProfileSetting>(m, "ProfileSetting")
            .value("mods", MOBase::IPluginGame::MODS)
            .value("configuration", MOBase::IPluginGame::CONFIGURATION)
            .value("savegames", MOBase::IPluginGame::SAVEGAMES)
            .value("preferDefaults", MOBase::IPluginGame::PREFER_DEFAULTS)

            .value("MODS", MOBase::IPluginGame::MODS)
            .value("CONFIGURATION", MOBase::IPluginGame::CONFIGURATION)
            .value("SAVEGAMES", MOBase::IPluginGame::SAVEGAMES)
            .value("PREFER_DEFAULTS", MOBase::IPluginGame::PREFER_DEFAULTS);

        // py::class_<IPluginGameWrapper, py::bases<IPlugin>,
        // boost::noncopyable>("IPluginGame")
        //     .def("localizedName", &MOBase::IPlugin::localizedName,
        //     &IPluginGameWrapper::localizedName_Default) .def("master",
        //     &MOBase::IPlugin::master, &IPluginGameWrapper::master_Default)

        //     .def("detectGame",
        //     py::pure_virtual(&MOBase::IPluginGame::detectGame))
        //     .def("gameName",
        //     py::pure_virtual(&MOBase::IPluginGame::gameName))
        //     .def("initializeProfile",
        //     py::pure_virtual(&MOBase::IPluginGame::initializeProfile),
        //     (py::arg("directory"), "settings")) .def("listSaves",
        //     py::pure_virtual(&MOBase::IPluginGame::listSaves),
        //     py::arg("folder")) .def("isInstalled",
        //     py::pure_virtual(&MOBase::IPluginGame::isInstalled))
        //     .def("gameIcon",
        //     py::pure_virtual(&MOBase::IPluginGame::gameIcon))
        //     .def("gameDirectory",
        //     py::pure_virtual(&MOBase::IPluginGame::gameDirectory))
        //     .def("dataDirectory",
        //     py::pure_virtual(&MOBase::IPluginGame::dataDirectory))
        //     .def("setGamePath",
        //     py::pure_virtual(&MOBase::IPluginGame::setGamePath),
        //     py::arg("path")) .def("documentsDirectory",
        //     py::pure_virtual(&MOBase::IPluginGame::documentsDirectory))
        //     .def("savesDirectory",
        //     py::pure_virtual(&MOBase::IPluginGame::savesDirectory))
        //     .def("executables",
        //     py::pure_virtual(&MOBase::IPluginGame::executables))
        //     .def("executableForcedLoads",
        //     py::pure_virtual(&MOBase::IPluginGame::executableForcedLoads))
        //     .def("steamAPPId",
        //     py::pure_virtual(&MOBase::IPluginGame::steamAPPId))
        //     .def("primaryPlugins",
        //     py::pure_virtual(&MOBase::IPluginGame::primaryPlugins))
        //     .def("gameVariants",
        //     py::pure_virtual(&MOBase::IPluginGame::gameVariants))
        //     .def("setGameVariant",
        //     py::pure_virtual(&MOBase::IPluginGame::setGameVariant),
        //     py::arg("variant")) .def("binaryName",
        //     py::pure_virtual(&MOBase::IPluginGame::binaryName))
        //     .def("gameShortName",
        //     py::pure_virtual(&MOBase::IPluginGame::gameShortName))
        //     .def("primarySources",
        //     py::pure_virtual(&MOBase::IPluginGame::primarySources))
        //     .def("validShortNames",
        //     py::pure_virtual(&MOBase::IPluginGame::validShortNames))
        //     .def("gameNexusName",
        //     py::pure_virtual(&MOBase::IPluginGame::gameNexusName))
        //     .def("iniFiles",
        //     py::pure_virtual(&MOBase::IPluginGame::iniFiles))
        //     .def("DLCPlugins",
        //     py::pure_virtual(&MOBase::IPluginGame::DLCPlugins))
        //     .def("CCPlugins",
        //     py::pure_virtual(&MOBase::IPluginGame::CCPlugins))
        //     .def("loadOrderMechanism",
        //     py::pure_virtual(&MOBase::IPluginGame::loadOrderMechanism))
        //     .def("sortMechanism",
        //     py::pure_virtual(&MOBase::IPluginGame::sortMechanism))
        //     .def("nexusModOrganizerID",
        //     py::pure_virtual(&MOBase::IPluginGame::nexusModOrganizerID))
        //     .def("nexusGameID",
        //     py::pure_virtual(&MOBase::IPluginGame::nexusGameID))
        //     .def("looksValid",
        //     py::pure_virtual(&MOBase::IPluginGame::looksValid),
        //     py::arg("directory")) .def("gameVersion",
        //     py::pure_virtual(&MOBase::IPluginGame::gameVersion))
        //     .def("getLauncherName",
        //     py::pure_virtual(&MOBase::IPluginGame::getLauncherName))

        //     .def("featureList", +[](MOBase::IPluginGame* p) {
        //       // Constructing a dict from class name to actual object:
        //       py::dict dict;
        //       mp11::mp_for_each<
        //         // Must user pointers because mp_for_each construct object:
        //         mp11::mp_transform<std::add_pointer_t, MpGameFeaturesList>
        //       >([&](auto* pt) {
        //         using T = std::remove_pointer_t<decltype(pt)>;
        //         typename py::reference_existing_object::apply<T*>::type
        //         converter;

        //         // Retrieve the python class object:
        //         const py::converter::registration* registration =
        //         py::converter::registry::query(py::type_id<T>()); py::object
        //         key
        //         =
        //         py::object(py::handle<>(py::borrowed(registration->get_class_object())));

        //         // Set the object:
        //         dict[key] = py::handle<>(converter(p->feature<T>()));
        //       });
        //       return dict;
        //     })

        //     .def("feature", +[](MOBase::IPluginGame* p, py::object clsObj) {
        //       py::object feature;
        //       mp11::mp_for_each<
        //         // Must user pointers because mp_for_each construct object:
        //         mp11::mp_transform<std::add_pointer_t, MpGameFeaturesList>
        //       >([&](auto* pt) {
        //         using T = std::remove_pointer_t<decltype(pt)>;
        //         typename py::reference_existing_object::apply<T*>::type
        //         converter;

        //         // Retrieve the python class object:
        //         const py::converter::registration* registration =
        //         py::converter::registry::query(py::type_id<T>());

        //         if (clsObj.ptr() == (PyObject*)
        //         registration->get_class_object())
        //         {
        //           feature =
        //           py::object(py::handle<>(converter(p->feature<T>())));
        //         }
        //         });
        //       return feature;
        //     }, py::arg("feature_type"))
        //     ;
    }

    // multiple installers
    void add_iplugininstaller_bindings(pybind11::module_ m)
    {
        py::enum_<MOBase::IPluginInstaller::EInstallResult>(m, "InstallResult")
            .value("SUCCESS", MOBase::IPluginInstaller::RESULT_SUCCESS)
            .value("FAILED", MOBase::IPluginInstaller::RESULT_FAILED)
            .value("CANCELED", MOBase::IPluginInstaller::RESULT_CANCELED)
            .value("MANUAL_REQUESTED", MOBase::IPluginInstaller::RESULT_MANUALREQUESTED)
            .value("NOT_ATTEMPTED", MOBase::IPluginInstaller::RESULT_NOTATTEMPTED);

        // this is bind but should not be inherited in Python - does not make sense,
        // having it makes it simpler to bind the Simple and Custom installers
        py::class_<IPluginInstaller, PyPluginInstallerBase<IPluginInstaller>, IPlugin,
                   std::unique_ptr<IPluginInstaller, py::nodelete>>(
            m, "IPluginInstaller", py::multiple_inheritance())
            .def("isArchiveSupported", &IPluginInstaller::isArchiveSupported, "tree"_a)
            .def("priority", &IPluginInstaller::priority)
            .def("onInstallationStart", &IPluginInstaller::onInstallationStart,
                 "archive"_a, "reinstallation"_a, "current_mod"_a)
            .def("onInstallationEnd", &IPluginInstaller::onInstallationEnd, "result"_a,
                 "new_mod"_a)
            .def("isManualInstaller", &IPluginInstaller::isManualInstaller)
            .def("setParentWidget", &IPluginInstaller::setParentWidget, "parent"_a)
            .def("setInstallationManager", &IPluginInstaller::setInstallationManager,
                 "manager"_a)
            .def("_parentWidget",
                 &PyPluginInstallerBase<IPluginInstaller>::parentWidget)
            .def("_manager", &PyPluginInstallerBase<IPluginInstaller>::manager,
                 py::return_value_policy::reference);

        py::class_<IPluginInstallerSimple, PyPluginInstallerSimple, IPluginInstaller,
                   std::unique_ptr<IPluginInstallerSimple, py::nodelete>>(
            m, "IPluginInstallerSimple", py::multiple_inheritance())
            .def(py::init<>())

            // note: keeping the variant here even if we always return a tuple
            // to be consistent with the wrapper and have proper stubs generation.
            .def(
                "install",
                [](IPluginInstallerSimple* p, GuessedValue<QString>& modName,
                   std::shared_ptr<IFileTree>& tree, QString& version,
                   int& nexusID) -> PyPluginInstallerSimple::py_install_return_type {
                    auto result = p->install(modName, tree, version, nexusID);
                    return std::make_tuple(result, tree, version, nexusID);
                },
                "name"_a, "tree"_a, "version"_a, "nexus_id"_a);

        py::class_<IPluginInstallerCustom, PyPluginInstallerCustom, IPluginInstaller,
                   std::unique_ptr<IPluginInstallerCustom, py::nodelete>>(
            m, "IPluginInstallerCustom", py::multiple_inheritance())
            .def(py::init<>())
            .def("isArchiveSupported", &IPluginInstallerCustom::isArchiveSupported,
                 "archive_name"_a)
            .def("supportedExtensions", &IPluginInstallerCustom::supportedExtensions)
            .def("install", &IPluginInstallerCustom::install, "mod_name"_a,
                 "game_name"_a, "archive_name"_a, "version"_a, "nexus_id"_a);
    }

    void add_plugins_bindings(pybind11::module_ m)
    {
        py::class_<IPlugin, PyPlugin, std::unique_ptr<IPlugin, py::nodelete>>(
            m, "IPluginBase", py::multiple_inheritance())
            .def(py::init<>())
            .def("init", &MOBase::IPlugin::init, "organizer"_a)
            .def("name", &MOBase::IPlugin::name)
            .def("localizedName", &MOBase::IPlugin::localizedName)
            .def("master", &MOBase::IPlugin::master)
            .def("author", &MOBase::IPlugin::author)
            .def("description", &MOBase::IPlugin::description)
            .def("version", &MOBase::IPlugin::version)
            .def("requirements", &MOBase::IPlugin::requirements)
            .def("settings", &MOBase::IPlugin::settings)
            .def("enabledByDefault", &MOBase::IPlugin::enabledByDefault);

        py::class_<IPyPlugin, PyPlugin, IPlugin,
                   std::unique_ptr<IPyPlugin, py::nodelete>>(m, "IPlugin",
                                                             py::multiple_inheritance())
            .def(py::init<>());

        py::class_<IPyPluginFileMapper, PyPluginFileMapper, IPlugin,
                   std::unique_ptr<IPyPluginFileMapper, py::nodelete>>(
            m, "IPluginFileMapper", py::multiple_inheritance())
            .def(py::init<>())
            .def("mappings", &IPluginFileMapper::mappings);

        py::class_<IPyPluginDiagnose, PyPluginDiagnose, IPlugin,
                   std::unique_ptr<IPyPluginDiagnose, py::nodelete>>(
            m, "IPluginDiagnose", py::multiple_inheritance())
            .def(py::init<>())
            .def("activeProblems", &MOBase::IPluginDiagnose::activeProblems)
            .def("shortDescription", &MOBase::IPluginDiagnose::shortDescription,
                 py::arg("key"))
            .def("fullDescription", &MOBase::IPluginDiagnose::fullDescription,
                 py::arg("key"))
            .def("hasGuidedFix", &MOBase::IPluginDiagnose::hasGuidedFix, py::arg("key"))
            .def("startGuidedFix", &MOBase::IPluginDiagnose::startGuidedFix,
                 py::arg("key"))
            .def("_invalidate", &PyPluginDiagnose::invalidate);

        py::class_<IPluginTool, PyPluginTool, IPlugin,
                   std::unique_ptr<IPluginTool, py::nodelete>>(
            m, "IPluginTool", py::multiple_inheritance())
            .def(py::init<>())
            .def("displayName", &IPluginTool::displayName)
            .def("tooltip", &IPluginTool::tooltip)
            .def("icon", &IPluginTool::icon)
            .def("display", &IPluginTool::display)
            .def("setParentWidget", &IPluginTool::setParentWidget)
            .def("_parentWidget", &PyPluginTool::parentWidget);

        py::class_<IPluginPreview, PyPluginPreview, IPlugin,
                   std::unique_ptr<IPluginPreview, py::nodelete>>(
            m, "IPluginPreview", py::multiple_inheritance())
            .def(py::init<>())
            .def("supportedExtensions", &IPluginPreview::supportedExtensions)
            .def("genFilePreview", &IPluginPreview::genFilePreview, "filename"_a,
                 "max_size"_a);

        py::class_<IPluginModPage, PyPluginModPage, IPluginModPage,
                   std::unique_ptr<IPluginModPage, py::nodelete>>(
            m, "IPluginModPage", py::multiple_inheritance())
            .def(py::init<>())
            .def("displayName", &IPluginModPage::displayName)
            .def("icon", &IPluginModPage::icon)
            .def("pageURL", &IPluginModPage::pageURL)
            .def("useIntegratedBrowser", &IPluginModPage::useIntegratedBrowser)
            .def("handlesDownload", &IPluginModPage::handlesDownload, "page_url"_a,
                 "download_url"_a, "fileinfo"_a)
            .def("setParentWidget", &IPluginModPage::setParentWidget, "parent"_a)
            .def("_parentWidget", &PyPluginModPage::parentWidget);

        add_iplugingame_bindings(m);
        add_iplugininstaller_bindings(m);
    }

    struct extract_plugins_helper {
        QList<QObject*> objects;

        template <class IPluginClass>
        void append_if_instance(pybind11::object plugin_obj)
        {
            if (py::isinstance<IPluginClass>(plugin_obj)) {
                objects.append(plugin_obj.cast<IPluginClass*>());
            }
        }
    };

    QList<QObject*> extract_plugins(pybind11::object plugin_obj)
    {
        extract_plugins_helper helper;

        // we need to check the trampoline class for these since the interfaces do not
        // extend IPlugin
        helper.append_if_instance<IPyPluginFileMapper>(plugin_obj);
        helper.append_if_instance<IPyPluginDiagnose>(plugin_obj);

        helper.append_if_instance<IPluginModPage>(plugin_obj);
        helper.append_if_instance<IPluginPreview>(plugin_obj);
        helper.append_if_instance<IPluginTool>(plugin_obj);

        // helper.append_if_instance<IPluginGame>(plugin_obj);
        helper.append_if_instance<IPluginInstallerSimple>(plugin_obj);
        helper.append_if_instance<IPluginInstallerCustom>(plugin_obj);

        if (helper.objects.isEmpty()) {
            if (py::isinstance<IPyPlugin>(plugin_obj)) {
                helper.objects.append(plugin_obj.cast<IPyPlugin*>());
            }
        }

        return helper.objects;
    }

}  // namespace mo2::python
