#include "wrappers.h"

#include <pybind11/functional.h>
#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "../pybind11_qt/pybind11_qt.h"

#include <iplugin.h>

namespace py = pybind11;
using namespace MOBase;

namespace mo2::python {

    void add_plugins_bindings(pybind11::module_ m)
    {
        // // Note: localizedName, master, requirements and enabledByDefault
        // have to
        // go in all the plugin wrappers declaration,
        // // since the default functions are specific to each wrapper,
        // otherwise in turns into an
        // // infinite recursion mess.
        // py::class_<IPluginWrapper, boost::noncopyable>("IPlugin")
        //   .def("init", py::pure_virtual(&MOBase::IPlugin::init),
        //   py::arg("organizer")) .def("name",
        //   py::pure_virtual(&MOBase::IPlugin::name)) .def("localizedName",
        //   &MOBase::IPlugin::localizedName,
        //   &IPluginWrapper::localizedName_Default) .def("master",
        //   &MOBase::IPlugin::master, &IPluginWrapper::master_Default)
        //   .def("author", py::pure_virtual(&MOBase::IPlugin::author))
        //   .def("description",
        //   py::pure_virtual(&MOBase::IPlugin::description)) .def("version",
        //   py::pure_virtual(&MOBase::IPlugin::version)) .def("requirements",
        //   &MOBase::IPlugin::requirements,
        //   &IPluginWrapper::requirements_Default) .def("settings",
        //   py::pure_virtual(&MOBase::IPlugin::settings))
        //   .def("enabledByDefault", &MOBase::IPlugin::enabledByDefault,
        //   &IPluginWrapper::enabledByDefault_Default)
        //   ;

        // py::class_<IPluginDiagnoseWrapper, py::bases<IPlugin>,
        // boost::noncopyable>("IPluginDiagnose")
        //     .def("localizedName", &MOBase::IPlugin::localizedName,
        //     &IPluginDiagnoseWrapper::localizedName_Default) .def("master",
        //     &MOBase::IPlugin::master,
        //     &IPluginDiagnoseWrapper::master_Default) .def("requirements",
        //     &MOBase::IPlugin::requirements,
        //     &IPluginDiagnoseWrapper::requirements_Default)
        //     .def("enabledByDefault", &MOBase::IPlugin::enabledByDefault,
        //     &IPluginDiagnoseWrapper::enabledByDefault_Default)

        //     .def("activeProblems",
        //     py::pure_virtual(&MOBase::IPluginDiagnose::activeProblems))
        //     .def("shortDescription",
        //     py::pure_virtual(&MOBase::IPluginDiagnose::shortDescription),
        //     py::arg("key")) .def("fullDescription",
        //     py::pure_virtual(&MOBase::IPluginDiagnose::fullDescription),
        //     py::arg("key")) .def("hasGuidedFix",
        //     py::pure_virtual(&MOBase::IPluginDiagnose::hasGuidedFix),
        //     py::arg("key")) .def("startGuidedFix",
        //     py::pure_virtual(&MOBase::IPluginDiagnose::startGuidedFix),
        //     py::arg("key")) .def("_invalidate",
        //     &IPluginDiagnoseWrapper::invalidate)
        //     ;
        // py::class_<IPluginFileMapperWrapper, py::bases<IPlugin>,
        // boost::noncopyable>("IPluginFileMapper")
        //     .def("localizedName", &MOBase::IPlugin::localizedName,
        //     &IPluginFileMapperWrapper::localizedName_Default) .def("master",
        //     &MOBase::IPlugin::master,
        //     &IPluginFileMapperWrapper::master_Default) .def("requirements",
        //     &MOBase::IPlugin::requirements,
        //     &IPluginFileMapperWrapper::requirements_Default)
        //     .def("enabledByDefault", &MOBase::IPlugin::enabledByDefault,
        //     &IPluginFileMapperWrapper::enabledByDefault_Default)

        //     .def("mappings",
        //     py::pure_virtual(&MOBase::IPluginFileMapper::mappings))
        //     ;

        // py::enum_<MOBase::IPluginGame::LoadOrderMechanism>("LoadOrderMechanism")
        //     .value("FileTime",
        //     MOBase::IPluginGame::LoadOrderMechanism::FileTime)
        //     .value("PluginsTxt",
        //     MOBase::IPluginGame::LoadOrderMechanism::PluginsTxt)

        //     .value("FILE_TIME",
        //     MOBase::IPluginGame::LoadOrderMechanism::FileTime)
        //     .value("PLUGINS_TXT",
        //     MOBase::IPluginGame::LoadOrderMechanism::PluginsTxt)
        //     ;

        // py::enum_<MOBase::IPluginGame::SortMechanism>("SortMechanism")
        //     .value("NONE", MOBase::IPluginGame::SortMechanism::NONE)
        //     .value("MLOX", MOBase::IPluginGame::SortMechanism::MLOX)
        //     .value("BOSS", MOBase::IPluginGame::SortMechanism::BOSS)
        //     .value("LOOT", MOBase::IPluginGame::SortMechanism::LOOT)
        //     ;

        // // This doesn't actually do the conversion, but might be convenient
        // for accessing the names for enum bits
        // py::enum_<MOBase::IPluginGame::ProfileSetting>("ProfileSetting")
        //     .value("mods", MOBase::IPluginGame::MODS)
        //     .value("configuration", MOBase::IPluginGame::CONFIGURATION)
        //     .value("savegames", MOBase::IPluginGame::SAVEGAMES)
        //     .value("preferDefaults", MOBase::IPluginGame::PREFER_DEFAULTS)

        //     .value("MODS", MOBase::IPluginGame::MODS)
        //     .value("CONFIGURATION", MOBase::IPluginGame::CONFIGURATION)
        //     .value("SAVEGAMES", MOBase::IPluginGame::SAVEGAMES)
        //     .value("PREFER_DEFAULTS", MOBase::IPluginGame::PREFER_DEFAULTS)
        //     ;

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

        // py::enum_<MOBase::IPluginInstaller::EInstallResult>("InstallResult")
        //     .value("SUCCESS", MOBase::IPluginInstaller::RESULT_SUCCESS)
        //     .value("FAILED", MOBase::IPluginInstaller::RESULT_FAILED)
        //     .value("CANCELED", MOBase::IPluginInstaller::RESULT_CANCELED)
        //     .value("MANUAL_REQUESTED",
        //     MOBase::IPluginInstaller::RESULT_MANUALREQUESTED)
        //     .value("NOT_ATTEMPTED",
        //     MOBase::IPluginInstaller::RESULT_NOTATTEMPTED)
        //     ;

        // py::class_<IPluginInstaller, py::bases<IPlugin>,
        // boost::noncopyable>("IPluginInstaller", py::no_init)
        //   .def("isArchiveSupported", &IPluginInstaller::isArchiveSupported,
        //   py::arg("tree")) .def("priority", &IPluginInstaller::priority)
        //   .def("onInstallationStart", &IPluginInstaller::onInstallationStart,
        //   (py::arg("archive"), py::arg("reinstallation"),
        //   py::arg("current_mod"))) .def("onInstallationEnd",
        //   &IPluginInstaller::onInstallationEnd, (py::arg("result"),
        //   py::arg("new_mod"))) .def("isManualInstaller",
        //   &IPluginInstaller::isManualInstaller) .def("setParentWidget",
        //   &IPluginInstaller::setParentWidget, py::arg("parent"))
        //   .def("setInstallationManager",
        //   &IPluginInstaller::setInstallationManager, py::arg("manager"))
        //   ;

        // py::class_<IPluginInstallerSimpleWrapper,
        // py::bases<IPluginInstaller>,
        // boost::noncopyable>("IPluginInstallerSimple")
        //   .def("onInstallationStart", &IPluginInstaller::onInstallationStart,
        //   (py::arg("archive"), py::arg("reinstallation"),
        //   py::arg("current_mod"))) .def("onInstallationEnd",
        //   &IPluginInstaller::onInstallationEnd, (py::arg("result"),
        //   py::arg("new_mod"))) .def("localizedName",
        //   &MOBase::IPlugin::localizedName,
        //   &IPluginInstallerSimpleWrapper::localizedName_Default)
        //   .def("master", &MOBase::IPlugin::master,
        //   &IPluginInstallerSimpleWrapper::master_Default)
        //   .def("requirements", &MOBase::IPlugin::requirements,
        //   &IPluginInstallerSimpleWrapper::requirements_Default)
        //   .def("enabledByDefault", &MOBase::IPlugin::enabledByDefault,
        //   &IPluginInstallerSimpleWrapper::enabledByDefault_Default)

        //   // Note: Keeping the variant here even if we always return a tuple
        //   to be consistent with the wrapper and
        //   // have proper stubs generation.
        //   .def("install", +[](IPluginInstallerSimple* p,
        //   GuessedValue<QString>& modName, std::shared_ptr<IFileTree>& tree,
        //   QString& version, int& nexusID)
        //     -> std::variant<IPluginInstaller::EInstallResult,
        //     std::shared_ptr<IFileTree>,
        //     std::tuple<IPluginInstaller::EInstallResult,
        //     std::shared_ptr<IFileTree>, QString, int>> {
        //       auto result = p->install(modName, tree, version, nexusID);
        //       return std::make_tuple(result, tree, version, nexusID);
        //     }, (py::arg("name"), "tree", "version", "nexus_id"))
        //   .def("_parentWidget", &IPluginInstallerSimpleWrapper::parentWidget,
        //   py::return_value_policy<py::return_by_value>()) .def("_manager",
        //   &IPluginInstallerSimpleWrapper::manager,
        //   py::return_value_policy<py::reference_existing_object>())
        //   ;

        // py::class_<IPluginInstallerCustomWrapper,
        // py::bases<IPluginInstaller>,
        // boost::noncopyable>("IPluginInstallerCustom")
        //   .def("onInstallationStart", &IPluginInstaller::onInstallationStart,
        //   (py::arg("archive"), py::arg("reinstallation"),
        //   py::arg("current_mod"))) .def("onInstallationEnd",
        //   &IPluginInstaller::onInstallationEnd, (py::arg("result"),
        //   py::arg("new_mod"))) .def("localizedName",
        //   &MOBase::IPlugin::localizedName,
        //   &IPluginInstallerCustomWrapper::localizedName_Default)
        //   .def("master", &MOBase::IPlugin::master,
        //   &IPluginInstallerCustomWrapper::master_Default)
        //   .def("requirements", &MOBase::IPlugin::requirements,
        //   &IPluginInstallerCustomWrapper::requirements_Default)
        //   .def("enabledByDefault", &MOBase::IPlugin::enabledByDefault,
        //   &IPluginInstallerCustomWrapper::enabledByDefault_Default)

        //   // Needs to add both otherwize boost does not understand:
        //   .def("isArchiveSupported", &IPluginInstaller::isArchiveSupported,
        //   py::arg("tree")) .def("isArchiveSupported",
        //   &IPluginInstallerCustom::isArchiveSupported,
        //   py::arg("archive_name")) .def("supportedExtensions",
        //   &IPluginInstallerCustom::supportedExtensions) .def("install",
        //   &IPluginInstallerCustom::install, (py::arg("mod_name"),
        //   "game_name", "archive_name", "version", "nexus_id"))
        //   .def("_parentWidget", &IPluginInstallerSimpleWrapper::parentWidget,
        //   py::return_value_policy<py::return_by_value>()) .def("_manager",
        //   &IPluginInstallerCustomWrapper::manager,
        //   py::return_value_policy<py::reference_existing_object>())
        //   ;

        // py::class_<IPluginModPageWrapper, py::bases<IPlugin>,
        // boost::noncopyable>("IPluginModPage")
        //   .def("localizedName", &MOBase::IPlugin::localizedName,
        //   &IPluginModPageWrapper::localizedName_Default) .def("master",
        //   &MOBase::IPlugin::master, &IPluginModPageWrapper::master_Default)
        //   .def("requirements", &MOBase::IPlugin::requirements,
        //   &IPluginModPageWrapper::requirements_Default)
        //   .def("enabledByDefault", &MOBase::IPlugin::enabledByDefault,
        //   &IPluginModPageWrapper::enabledByDefault_Default)

        //   .def("displayName", py::pure_virtual(&IPluginModPage::displayName))
        //   .def("icon", py::pure_virtual(&IPluginModPage::icon))
        //   .def("pageURL", py::pure_virtual(&IPluginModPage::pageURL))
        //   .def("useIntegratedBrowser",
        //   py::pure_virtual(&IPluginModPage::useIntegratedBrowser))
        //   .def("handlesDownload",
        //   py::pure_virtual(&IPluginModPage::handlesDownload),
        //   (py::arg("page_url"), "download_url", "fileinfo"))
        //   .def("setParentWidget", &IPluginModPage::setParentWidget,
        //   &IPluginModPageWrapper::setParentWidget_Default, py::arg("parent"))
        //   .def("_parentWidget", &IPluginModPageWrapper::parentWidget,
        //   py::return_value_policy<py::return_by_value>())
        //   ;

        // py::class_<IPluginPreviewWrapper, py::bases<IPlugin>,
        // boost::noncopyable>("IPluginPreview")
        //   .def("localizedName", &MOBase::IPlugin::localizedName,
        //   &IPluginPreviewWrapper::localizedName_Default) .def("master",
        //   &MOBase::IPlugin::master, &IPluginPreviewWrapper::master_Default)
        //   .def("requirements", &MOBase::IPlugin::requirements,
        //   &IPluginPreviewWrapper::requirements_Default)
        //   .def("enabledByDefault", &MOBase::IPlugin::enabledByDefault,
        //   &IPluginPreviewWrapper::enabledByDefault_Default)

        //   .def("supportedExtensions",
        //   py::pure_virtual(&IPluginPreview::supportedExtensions))
        //   .def("genFilePreview",
        //   py::pure_virtual(&IPluginPreview::genFilePreview),
        //   py::return_value_policy<py::return_by_value>(),
        //     (py::arg("filename"), "max_size"))
        //   ;

        // py::class_<IPluginToolWrapper, py::bases<IPlugin>,
        // boost::noncopyable>("IPluginTool")
        //   .def("localizedName", &MOBase::IPlugin::localizedName,
        //   &IPluginToolWrapper::localizedName_Default) .def("master",
        //   &MOBase::IPlugin::master, &IPluginToolWrapper::master_Default)
        //   .def("requirements", &MOBase::IPlugin::requirements,
        //   &IPluginToolWrapper::requirements_Default) .def("enabledByDefault",
        //   &MOBase::IPlugin::enabledByDefault,
        //   &IPluginToolWrapper::enabledByDefault_Default)

        //   .def("displayName", py::pure_virtual(&IPluginTool::displayName))
        //   .def("tooltip", py::pure_virtual(&IPluginTool::tooltip))
        //   .def("icon", py::pure_virtual(&IPluginTool::icon))
        //   .def("display", py::pure_virtual(&IPluginTool::display))
        //   .def("setParentWidget", &IPluginTool::setParentWidget,
        //   &IPluginToolWrapper::setParentWidget_Default, py::arg("parent"))
        //   .def("_parentWidget", &IPluginToolWrapper::parentWidget,
        //   py::return_value_policy<py::return_by_value>())
        //   ;
    }

}  // namespace mo2::python
