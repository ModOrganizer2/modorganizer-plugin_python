#include "pythonrunner.h"

#pragma warning( disable : 4100 )
#pragma warning( disable : 4996 )

#include <idownloadmanager.h>
#include <ifiletree.h>
#include <iinstallationmanager.h>
#include <imodinterface.h>
#include <imodlist.h>
#include <imodrepositorybridge.h>
#include <iplugin.h>
#include <iplugingame.h>
#include <iplugininstaller.h>
#include <ipluginlist.h>
#include <iplugintool.h>
#include <iprofile.h>
#include <log.h>
#include <report.h>
#include <utility.h>

#include <Windows.h>
#include <QDir>
#include <QFile>
#include <QCoreApplication>
#include <QWidget>

#include <variant>
#include <tuple>

#ifndef Q_MOC_RUN
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
// #include <boost/python.hpp>
// #include <boost/mp11.hpp>
#endif

#include "converters.h"
#include "error.h"
// #include "gamefeatureswrappers.h"
// #include "proxypluginwrappers.h"
#include "pyfiletree.h"
#include "pythonutils.h"
// #include "shared_ptr_converter.h"
// #include "sipApiAccess.h"
// #include "tuple_helper.h"
// #include "uibasewrappers.h"
// #include "variant_helper.h"
// #include "widgets.h"

using namespace MOBase;

// namespace bpy = boost::python;
// namespace mp11 = boost::mp11;
namespace py = pybind11;

/**
 * This macro should be used within a py::class_ declaration and will define two
 * methods: __getattr__ and Name, where Name will simply return the object as a QClass*
 * object, while __getattr__ will delegate to the underlying QClass object when required.
 *
 * This allow access to Qt interface for object exposed using boost::python (e.g., signals,
 * methods from QObject or QWidget, etc.).
 */
#define Q_DELEGATE(Class, QClass, Name) \
  .def(Name, +[](Class* w) -> QClass* { return w; }, py::return_value_policy<py::reference_existing_object>())          \
  .def("__getattr__", +[](Class* w, py::str str) -> py::object {  \
    return py::object{ (QClass*)w }.attr(str);                               \
  })

PYBIND11_MODULE(mobase, m)
{
  py::module_::import("PyQt6.QtCore");
  py::module_::import("PyQt6.QtWidgets");

  py::detail::type_caster<QString> t1;
  py::detail::type_caster<QVariant> t2;

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

  m.def("testEnum", [](QMessageBox::StandardButton button) {
    return py::make_tuple(button, QMessageBox::Icon::Information);
  });

  m.def("testPixmap", [](QPixmap const& pixmap) {
    return pixmap.size();
  });

  // utils::register_qstring_converter();
  // utils::register_qvariant_converter();

  // utils::register_qclass_converter<QObject>();
  // utils::register_qclass_converter<QDateTime>();
  // utils::register_qclass_converter<QDir>();
  // utils::register_qclass_converter<QFileInfo>();
  // utils::register_qclass_converter<QWidget>();
  // utils::register_qclass_converter<QMainWindow>();
  // utils::register_qclass_converter<QIcon>();
  // utils::register_qclass_converter<QSize>();
  // utils::register_qclass_converter<QUrl>();

  // // QFlags:
  // utils::register_qflags_converter<IPluginList::PluginStates>();
  // utils::register_qflags_converter<IPluginGame::ProfileSettings>();
  // utils::register_qflags_converter<IModList::ModStates>();

  // // Enums:
  // utils::register_enum_converter<QMessageBox::StandardButton>();
  // utils::register_enum_converter<QMessageBox::Icon>();

  // // Pointers:
  // py::register_ptr_to_python<std::shared_ptr<FileTreeEntry>>();
  // py::register_ptr_to_python<std::shared_ptr<const FileTreeEntry>>();
  // py::implicitly_convertible<std::shared_ptr<FileTreeEntry>, std::shared_ptr<const FileTreeEntry>>();
  // py::register_ptr_to_python<std::shared_ptr<IFileTree>>();
  // py::register_ptr_to_python<std::shared_ptr<const IFileTree>>();
  // py::implicitly_convertible<std::shared_ptr<IFileTree>, std::shared_ptr<const IFileTree>>();

  // utils::shared_ptr_from_python<std::shared_ptr<const ISaveGame>>();
  // py::register_ptr_to_python<std::shared_ptr<const ISaveGame>>();

  // utils::shared_ptr_from_python<std::shared_ptr<const IPluginRequirement>>();
  // py::register_ptr_to_python<std::shared_ptr<const IPluginRequirement>>();

  // // Containers:
  // utils::register_sequence_container<std::vector<int>>();
  // utils::register_sequence_container<std::vector<unsigned int>>();
  // utils::register_sequence_container<QList<ExecutableInfo>>();
  // utils::register_sequence_container<QList<ExecutableForcedLoadSetting>>();
  // utils::register_sequence_container<QList<PluginSetting>>();
  // utils::register_sequence_container<QList<ModRepositoryFileInfo>>();
  // utils::register_sequence_container<QStringList>();
  // utils::register_sequence_container<QList<QString>>();
  // utils::register_sequence_container<QList<QFileInfo>>();
  // utils::register_sequence_container<QList<IOrganizer::FileInfo>>();
  // utils::register_sequence_container<QList<QVariant>>(); // Required for QVariant since this is QVariantList.
  // utils::register_sequence_container<std::vector<std::shared_ptr<const FileTreeEntry>>>();
  // utils::register_sequence_container<std::vector<std::shared_ptr<const ISaveGame>>>();
  // utils::register_sequence_container<std::vector<std::shared_ptr<const IPluginRequirement>>>();
  // utils::register_sequence_container<std::vector<ModDataContent::Content>>();
  // utils::register_sequence_container<std::vector<Mapping>>();
  // utils::register_sequence_container<std::vector<TaskDialogButton>>();

  // utils::register_set_container<std::set<QString>>();

  // utils::register_associative_container<QMap<QString, QVariant>>(); // Required for QVariant since this is QVariantMap.
  // utils::register_associative_container<QMap<QString, QStringList>>();
  // utils::register_associative_container<std::map<QString, IModList::ModStates>>();
  // utils::register_associative_container<std::map<QString, QVariant>>();

  // utils::register_associative_container<IFileTree::OverwritesType>();

  // utils::register_optional<std::optional<IPluginRequirement::Problem>>();

  // // Tuple:
  // py::register_tuple<std::tuple<bool, DWORD>>(); // IOrganizer::waitForApplication
  // py::register_tuple<std::tuple<bool, bool>>();  // IProfile::invalidationActive
  // py::register_tuple<std::tuple<IPluginInstaller::EInstallResult, QString, int>>();
  // py::register_tuple<std::tuple<IPluginInstaller::EInstallResult, std::shared_ptr<IFileTree>, QString, int>>();
  // py::register_tuple<std::tuple<QString, QString>>();

  // // Variants:
  // py::register_variant<std::variant<
  //   IPluginInstaller::EInstallResult,
  //   std::shared_ptr<IFileTree>,
  //   std::tuple<IPluginInstaller::EInstallResult, std::shared_ptr<IFileTree>, QString, int>>>();
  // py::register_variant<std::variant<IFileTree::OverwritesType, std::size_t>>();
  // py::register_variant<std::variant<QString, bool>>();
  // py::register_variant<std::variant<QString, std::tuple<QString, QString>>>();
  // py::register_variant<std::variant<QString, GuessedValue<QString>>>();

  // // Functions:
  // utils::register_functor_converter<void()>(); // converter for the onRefreshed-callback
  // utils::register_functor_converter<void(const QString&)>();
  // utils::register_functor_converter<void(int)>();
  // utils::register_functor_converter<void(const QString&, unsigned int)>();
  // utils::register_functor_converter<void(const QString&, int, int)>(); // converter for the onModMoved-callback and onPluginMoved callbacks
  // utils::register_functor_converter<void(const std::map<QString, IModList::ModStates>&)>(); // converter for the onModStateChanged-callback (IModList)
  // utils::register_functor_converter<void(const std::map<QString, IPluginList::PluginStates>&)>(); // converter for the onPluginStateChanged-callback (IPluginList)
  // utils::register_functor_converter<void(const QString&, const QString&, const QVariant&, const QVariant&)>();
  // utils::register_functor_converter<void(QMainWindow*)>();
  // utils::register_functor_converter<void(IProfile*), py::pointer_wrapper<IProfile*>>();
  // utils::register_functor_converter<void(IProfile*, const QString&, const QString&), py::pointer_wrapper<IProfile*>>();
  // utils::register_functor_converter<void(IProfile*, IProfile*), py::pointer_wrapper<IProfile*>>();
  // utils::register_functor_converter<bool(const QString&)>();
  // utils::register_functor_converter<IFileTree::WalkReturn(const QString&, std::shared_ptr<const FileTreeEntry>)>();
  // utils::register_functor_converter<bool(const IOrganizer::FileInfo&), boost::reference_wrapper<const IOrganizer::FileInfo&>>();
  // utils::register_functor_converter<bool(std::shared_ptr<FileTreeEntry> const&)>();
  // utils::register_functor_converter<std::variant<QString, bool>(QString const&)>();
  // utils::register_functor_converter<void(IModInterface *), py::pointer_wrapper<IModInterface*>>();
  // utils::register_functor_converter<bool(IOrganizer*), py::pointer_wrapper<IOrganizer*>>();
  // utils::register_functor_converter<void(const IPlugin*), py::pointer_wrapper<const IPlugin*>>();

  // // This one is kept for backward-compatibility while we deprecate onModStateChanged for singl mod.
  // utils::register_functor_converter<void(const QString&, IModList::ModStates)>(); // converter for the onModStateChanged-callback (IModList).
  // utils::register_functor_converter<void(const QString&, IPluginList::PluginStates)>(); // converter for the onPluginStateChanged-callback (IPluginList).

  // //
  // // Class declarations:
  // //

  // py::enum_<MOBase::VersionInfo::ReleaseType>("ReleaseType")
  //     .value("final", MOBase::VersionInfo::RELEASE_FINAL)
  //     .value("candidate", MOBase::VersionInfo::RELEASE_CANDIDATE)
  //     .value("beta", MOBase::VersionInfo::RELEASE_BETA)
  //     .value("alpha", MOBase::VersionInfo::RELEASE_ALPHA)
  //     .value("prealpha", MOBase::VersionInfo::RELEASE_PREALPHA)

  //     .value("FINAL", MOBase::VersionInfo::RELEASE_FINAL)
  //     .value("CANDIDATE", MOBase::VersionInfo::RELEASE_CANDIDATE)
  //     .value("BETA", MOBase::VersionInfo::RELEASE_BETA)
  //     .value("ALPHA", MOBase::VersionInfo::RELEASE_ALPHA)
  //     .value("PRE_ALPHA", MOBase::VersionInfo::RELEASE_PREALPHA)
  //     ;

  // py::enum_<MOBase::VersionInfo::VersionScheme>("VersionScheme")
  //     .value("discover", MOBase::VersionInfo::SCHEME_DISCOVER)
  //     .value("regular", MOBase::VersionInfo::SCHEME_REGULAR)
  //     .value("decimalmark", MOBase::VersionInfo::SCHEME_DECIMALMARK)
  //     .value("numbersandletters", MOBase::VersionInfo::SCHEME_NUMBERSANDLETTERS)
  //     .value("date", MOBase::VersionInfo::SCHEME_DATE)
  //     .value("literal", MOBase::VersionInfo::SCHEME_LITERAL)

  //     .value("DISCOVER", MOBase::VersionInfo::SCHEME_DISCOVER)
  //     .value("REGULAR", MOBase::VersionInfo::SCHEME_REGULAR)
  //     .value("DECIMAL_MARK", MOBase::VersionInfo::SCHEME_DECIMALMARK)
  //     .value("NUMBERS_AND_LETTERS", MOBase::VersionInfo::SCHEME_NUMBERSANDLETTERS)
  //     .value("DATE", MOBase::VersionInfo::SCHEME_DATE)
  //     .value("LITERAL", MOBase::VersionInfo::SCHEME_LITERAL)
  //     ;

  // py::class_<VersionInfo>("VersionInfo")
  //     .def(py::init<QString, VersionInfo::VersionScheme>(
  //       (py::arg("value"), py::arg("scheme") = VersionInfo::SCHEME_DISCOVER)))
  //     // Note: Order of the two init<> below is important because ReleaseType is a simple enum with an
  //     // implicit int conversion.
  //     .def(py::init<int, int, int, int, VersionInfo::ReleaseType>(
  //       (py::arg("major"), "minor", "subminor", "subsubminor", py::arg("release_type") = VersionInfo::RELEASE_FINAL)))
  //     .def(py::init<int, int, int, VersionInfo::ReleaseType>(
  //       (py::arg("major"), "minor", "subminor", py::arg("release_type") = VersionInfo::RELEASE_FINAL)))
  //     .def("clear", &VersionInfo::clear)
  //     .def("parse", &VersionInfo::parse,
  //       (py::arg("value"), py::arg("scheme") = VersionInfo::SCHEME_DISCOVER, py::arg("is_manual") = false))
  //     .def("canonicalString", &VersionInfo::canonicalString)
  //     .def("displayString", &VersionInfo::displayString, py::arg("forced_segments") = 2)
  //     .def("isValid", &VersionInfo::isValid)
  //     .def("scheme", &VersionInfo::scheme)
  //     .def("__str__", &VersionInfo::canonicalString)
  //     .def(py::self < py::self)
  //     .def(py::self > py::self)
  //     .def(py::self <= py::self)
  //     .def(py::self >= py::self)
  //     .def(py::self != py::self)
  //     .def(py::self == py::self)
  //     ;

  // py::class_<PluginSetting>(
  //   "PluginSetting", py::init<const QString&, const QString&, const QVariant&>(
  //     (py::arg("key"), "description", "default_value")))
  //   .def_readwrite("key", &PluginSetting::key)
  //   .def_readwrite("description", &PluginSetting::description)
  //   .def_readwrite("default_value", &PluginSetting::defaultValue);

  // py::class_<ExecutableInfo>("ExecutableInfo",
  //   py::init<const QString&, const QFileInfo&>((py::arg("title"), "binary")))
  //     .def("withArgument", &ExecutableInfo::withArgument, py::return_self<>(), py::arg("argument"))
  //     .def("withWorkingDirectory", &ExecutableInfo::withWorkingDirectory, py::return_self<>(), py::arg("directory"))
  //     .def("withSteamAppId", &ExecutableInfo::withSteamAppId, py::return_self<>(), py::arg("app_id"))
  //     .def("asCustom", &ExecutableInfo::asCustom, py::return_self<>())
  //     .def("isValid", &ExecutableInfo::isValid)
  //     .def("title", &ExecutableInfo::title)
  //     .def("binary", &ExecutableInfo::binary)
  //     .def("arguments", &ExecutableInfo::arguments)
  //     .def("workingDirectory", &ExecutableInfo::workingDirectory)
  //     .def("steamAppID", &ExecutableInfo::steamAppID)
  //     .def("isCustom", &ExecutableInfo::isCustom)
  //     ;

  // py::class_<ExecutableForcedLoadSetting>("ExecutableForcedLoadSetting",
  //   py::init<const QString&, const QString&>((py::arg("process"), "library")))
  //     .def("withForced", &ExecutableForcedLoadSetting::withForced, py::return_self<>(), py::arg("forced"))
  //     .def("withEnabled", &ExecutableForcedLoadSetting::withEnabled, py::return_self<>(), py::arg("enabled"))
  //     .def("enabled", &ExecutableForcedLoadSetting::enabled)
  //     .def("forced", &ExecutableForcedLoadSetting::forced)
  //     .def("library", &ExecutableForcedLoadSetting::library)
  //     .def("process", &ExecutableForcedLoadSetting::process)
  //     ;

  // py::class_<ISaveGameWrapper, py::bases<>, boost::noncopyable>("ISaveGame")
  //     .def("getFilepath", py::pure_virtual(&ISaveGame::getFilepath))
  //     .def("getCreationTime", py::pure_virtual(&ISaveGame::getCreationTime))
  //     .def("getName", py::pure_virtual(&ISaveGame::getName))
  //     .def("getSaveGroupIdentifier", py::pure_virtual(&ISaveGame::getSaveGroupIdentifier))
  //     .def("allFiles", py::pure_virtual(&ISaveGame::allFiles))
  //     ;

  // // See Q_DELEGATE for more details.
  // py::class_<ISaveGameInfoWidgetWrapper, py::bases<>, ISaveGameInfoWidgetWrapper*, boost::noncopyable>(
  //   "ISaveGameInfoWidget", py::init<py::optional<QWidget*>>(py::arg("parent")))
  //   .def("setSave", py::pure_virtual(&ISaveGameInfoWidget::setSave), py::arg("save"))

  //   Q_DELEGATE(ISaveGameInfoWidget, QWidget, "_widget")
  //   ;

  // // Plugin requirements:
  // auto iPluginRequirementClass = py::class_<
  //   IPluginRequirementWrapper, py::bases<>, boost::noncopyable>("IPluginRequirement");
  // {
  //   py::scope scope = iPluginRequirementClass;

  //   py::class_<IPluginRequirement::Problem>("Problem",
  //     py::init<QString, QString>((py::arg("short_description"), py::arg("long_description") = "")))
  //     .def("shortDescription", &IPluginRequirement::Problem::shortDescription)
  //     .def("longDescription", &IPluginRequirement::Problem::longDescription);

  //   iPluginRequirementClass
  //     .def("check", py::pure_virtual(&IPluginRequirement::check), py::arg("organizer"))
  //     ;
  // }

  // py::class_<PluginRequirementFactory, boost::noncopyable>("PluginRequirementFactory")
  //   // pluginDependency
  //   .def("pluginDependency", +[](QStringList const& pluginNames) {
  //     return PluginRequirementFactory::pluginDependency(pluginNames);
  //   }, py::arg("plugins"))
  //   .def("pluginDependency", +[](QString const& pluginName) {
  //     return PluginRequirementFactory::pluginDependency(pluginName);
  //   }, py::arg("plugin"))
  //   .staticmethod("pluginDependency")
  //   // gameDependency
  //   .def("gameDependency", +[](QStringList const& gameNames) {
  //   return PluginRequirementFactory::gameDependency(gameNames);
  //     }, py::arg("games"))
  //   .def("gameDependency", +[](QString const& gameNames) {
  //       return PluginRequirementFactory::gameDependency(gameNames);
  //     }, py::arg("game"))
  //   .staticmethod("gameDependency")
  //   // diagnose
  //   .def("diagnose", &PluginRequirementFactory::diagnose, py::arg("diagnose"))
  //   .staticmethod("diagnose")
  //   // basic
  //   .def("basic", &PluginRequirementFactory::basic, (py::arg("checker"), "description"))
  //   .staticmethod("basic");

  // py::class_<IOrganizer::FileInfo>("FileInfo", py::init<>())
  //   .add_property("filePath",
  //     +[](const IOrganizer::FileInfo& info) { return info.filePath; },
  //     +[](IOrganizer::FileInfo& info, QString value) { info.filePath = value; })
  //   .add_property("archive",
  //     +[](const IOrganizer::FileInfo& info) { return info.archive; },
  //     +[](IOrganizer::FileInfo& info, QString value) { info.archive = value; })
  //   .add_property("origins",
  //     +[](const IOrganizer::FileInfo& info) { return info.origins; },
  //     +[](IOrganizer::FileInfo& info, QStringList value) { info.origins = value; })
  //   ;

  // py::class_<IOrganizer, boost::noncopyable>("IOrganizer", py::no_init)
  //     .def("createNexusBridge", &IOrganizer::createNexusBridge, py::return_value_policy<py::reference_existing_object>())
  //     .def("profileName", &IOrganizer::profileName)
  //     .def("profilePath", &IOrganizer::profilePath)
  //     .def("downloadsPath", &IOrganizer::downloadsPath)
  //     .def("overwritePath", &IOrganizer::overwritePath)
  //     .def("basePath", &IOrganizer::basePath)
  //     .def("modsPath", &IOrganizer::modsPath)
  //     .def("appVersion", &IOrganizer::appVersion)
  //     .def("createMod", &IOrganizer::createMod, py::return_value_policy<py::reference_existing_object>(), py::arg("name"))
  //     .def("getGame", &IOrganizer::getGame, py::return_value_policy<py::reference_existing_object>(), py::arg("name"))
  //     .def("modDataChanged", &IOrganizer::modDataChanged, py::arg("mod"))
  //     .def("isPluginEnabled", +[](IOrganizer* o, IPlugin* plugin) { return o->isPluginEnabled(plugin); }, py::arg("plugin"))
  //     .def("isPluginEnabled", +[](IOrganizer* o, QString const& plugin) { return o->isPluginEnabled(plugin); }, py::arg("plugin"))
  //     .def("pluginSetting", &IOrganizer::pluginSetting, (py::arg("plugin_name"), "key"))
  //     .def("setPluginSetting", &IOrganizer::setPluginSetting, (py::arg("plugin_name"), "key", "value"))
  //     .def("persistent", &IOrganizer::persistent, (py::arg("plugin_name"), "key", py::arg("default") = QVariant()))
  //     .def("setPersistent", &IOrganizer::setPersistent, (py::arg("plugin_name"), "key", "value", py::arg("sync") = true))
  //     .def("pluginDataPath", &IOrganizer::pluginDataPath)
  //     .def("installMod", &IOrganizer::installMod, py::return_value_policy<py::reference_existing_object>(), (py::arg("filename"), py::arg("name_suggestion") = ""))
  //     .def("resolvePath", &IOrganizer::resolvePath, py::arg("filename"))
  //     .def("listDirectories", &IOrganizer::listDirectories, py::arg("directory"))

  //     // Provide multiple overloads of findFiles:
  //     .def("findFiles", +[](const IOrganizer* o, QString const& p, std::function<bool(QString const&)> f) { return o->findFiles(p, f); },
  //       (py::arg("path"), "filter"))

  //     // In C++, it is possible to create a QStringList implicitly from a single QString. This is not possible with the current
  //     // converters in python (and I do not think it is a good idea to have it everywhere), but here it is nice to be able to
  //     // pass a single string, so we add an extra overload.
  //     // Important: the order matters, because a Python string can be converted to a QStringList since it is a sequence of
  //     // single-character strings:
  //     .def("findFiles", +[](const IOrganizer* o, QString const& p, const QStringList& gf) { return o->findFiles(p, gf); },
  //       (py::arg("path"), "patterns"))
  //     .def("findFiles", +[](const IOrganizer* o, QString const& p, const QString& f) { return o->findFiles(p, QStringList{ f }); },
  //       (py::arg("path"), "pattern"))

  //     .def("getFileOrigins", &IOrganizer::getFileOrigins, py::arg("filename"))
  //     .def("findFileInfos", &IOrganizer::findFileInfos, (py::arg("path"), "filter"))

  //     .def("virtualFileTree", &IOrganizer::virtualFileTree)

  //     .def("downloadManager", &IOrganizer::downloadManager, py::return_value_policy<py::reference_existing_object>())
  //     .def("pluginList", &IOrganizer::pluginList, py::return_value_policy<py::reference_existing_object>())
  //     .def("modList", &IOrganizer::modList, py::return_value_policy<py::reference_existing_object>())
  //     .def("profile", &IOrganizer::profile, py::return_value_policy<py::reference_existing_object>())

  //     // Custom implementation for startApplication and waitForApplication because 1) HANDLE (= void*) is not properly
  //     // converted from/to python, and 2) we need to convert the by-ptr argument to a return-tuple for waitForApplication:
  //     .def("startApplication",
  //       +[](IOrganizer* o, const QString& executable, const QStringList& args, const QString& cwd, const QString& profile,
  //         const QString& forcedCustomOverwrite, bool ignoreCustomOverwrite) {
  //           return (std::uintptr_t) o->startApplication(executable, args, cwd, profile, forcedCustomOverwrite, ignoreCustomOverwrite);
  //       }, (py::arg("executable"), (py::arg("args") = QStringList()), (py::arg("cwd") = ""), (py::arg("profile") = ""),
  //           (py::arg("forcedCustomOverwrite") = ""), (py::arg("ignoreCustomOverwrite") = false)), py::return_value_policy<py::return_by_value>())
  //     .def("waitForApplication", +[](IOrganizer *o, std::uintptr_t handle, bool refresh) {
  //         DWORD returnCode;
  //         bool result = o->waitForApplication((HANDLE)handle, refresh, &returnCode);
  //         return std::make_tuple(result, returnCode);
  //       }, (py::arg("handle"), py::arg("refresh") = true))
  //     .def("refresh", &IOrganizer::refresh, (py::arg("save_changes") = true))
  //     .def("managedGame", &IOrganizer::managedGame, py::return_value_policy<py::reference_existing_object>())

  //     .def("onAboutToRun", &IOrganizer::onAboutToRun, py::arg("callback"))
  //     .def("onFinishedRun", &IOrganizer::onFinishedRun, py::arg("callback"))
  //     .def("onUserInterfaceInitialized", &IOrganizer::onUserInterfaceInitialized, py::arg("callback"))
  //     .def("onProfileCreated", &IOrganizer::onProfileCreated, py::arg("callback"))
  //     .def("onProfileRenamed", &IOrganizer::onProfileRenamed, py::arg("callback"))
  //     .def("onProfileRemoved", &IOrganizer::onProfileRemoved, py::arg("callback"))
  //     .def("onProfileChanged", &IOrganizer::onProfileChanged, py::arg("callback"))

  //     .def("onPluginSettingChanged", &IOrganizer::onPluginSettingChanged, py::arg("callback"))
  //     .def("onPluginEnabled", +[](IOrganizer* o, std::function<void(const IPlugin*)> const& func) {
  //         o->onPluginEnabled(func);
  //       }, py::arg("callback"))
  //     .def("onPluginEnabled", +[](IOrganizer* o, QString const& name, std::function<void()> const& func) {
  //         o->onPluginEnabled(name, func);
  //       }, (py::arg("name"), py::arg("callback")))
  //     .def("onPluginDisabled", +[](IOrganizer* o, std::function<void(const IPlugin*)> const& func) {
  //         o->onPluginDisabled(func);
  //       }, py::arg("callback"))
  //     .def("onPluginDisabled", +[](IOrganizer* o, QString const& name, std::function<void()> const& func) {
  //         o->onPluginDisabled(name, func);
  //       }, (py::arg("name"), py::arg("callback")))

  //     // DEPRECATED:
  //     .def("getMod", +[](IOrganizer* o, QString const& name) {
  //         utils::show_deprecation_warning("getMod",
  //           "IOrganizer::getMod(str) is deprecated, use IModList::getMod(str) instead.");
  //         return o->modList()->getMod(name);
  //     }, py::return_value_policy<py::reference_existing_object>(), py::arg("name"))
  //     .def("removeMod", +[](IOrganizer* o, IModInterface *mod) {
  //         utils::show_deprecation_warning("removeMod",
  //           "IOrganizer::removeMod(IModInterface) is deprecated, use IModList::removeMod(IModInterface) instead.");
  //         return o->modList()->removeMod(mod);
  //     }, py::arg("mod"))
  //     .def("modsSortedByProfilePriority", +[](IOrganizer* o) {
  //         utils::show_deprecation_warning("modsSortedByProfilePriority",
  //           "IOrganizer::modsSortedByProfilePriority() is deprecated, use IModList::allModsByProfilePriority() instead.");
  //         return o->modList()->allModsByProfilePriority();
  //     })
  //     .def("refreshModList", +[](IOrganizer* o, bool s) {
  //       utils::show_deprecation_warning("refreshModList",
  //         "IOrganizer::refreshModList(bool) is deprecated, use IOrganizer::refresh(bool) instead.");
  //       o->refresh(s);
  //     }, (py::arg("save_changes") = true))
  //     .def("onModInstalled", +[](IOrganizer* organizer, const std::function<void(QString const&)>& func) {
  //       utils::show_deprecation_warning("onModInstalled",
  //         "IOrganizer::onModInstalled(Callable[[str], None]) is deprecated, "
  //         "use IModList::onModInstalled(Callable[[IModInterface], None]) instead.");
  //       return organizer->modList()->onModInstalled([func](MOBase::IModInterface* m) { func(m->name()); });;
  //     }, py::arg("callback"))

  //     .def("getPluginDataPath", &IOrganizer::getPluginDataPath)
  //     .staticmethod("getPluginDataPath")

  //     ;

  mo2::python::add_ifiletree_bindings(m);

  py::class_<IProfile>(m, "IProfile")
      .def("name", &IProfile::name)
      .def("absolutePath", &IProfile::absolutePath)
      .def("localSavesEnabled", &IProfile::localSavesEnabled)
      .def("localSettingsEnabled", &IProfile::localSettingsEnabled)
      .def("invalidationActive", [](const IProfile* p) {
        bool supported;
        bool active = p->invalidationActive(&supported);
        return py::make_tuple(active, supported);
      })
      .def("absoluteIniFilePath", &IProfile::absoluteIniFilePath, py::arg("inifile"))
      ;

  // py::class_<IModRepositoryBridge, boost::noncopyable>("IModRepositoryBridge", py::no_init)
  //     .def("requestDescription", &IModRepositoryBridge::requestDescription, (py::arg("game_name"), "mod_id", "user_data"))
  //     .def("requestFiles", &IModRepositoryBridge::requestFiles, (py::arg("game_name"), "mod_id", "user_data"))
  //     .def("requestFileInfo", &IModRepositoryBridge::requestFileInfo, (py::arg("game_name"), "mod_id", "file_id", "user_data"))
  //     .def("requestDownloadURL", &IModRepositoryBridge::requestDownloadURL, (py::arg("game_name"), "mod_id", "file_id", "user_data"))
  //     .def("requestToggleEndorsement", &IModRepositoryBridge::requestToggleEndorsement, (py::arg("game_name"), "mod_id", "mod_version", "endorse", "user_data"))

  //     Q_DELEGATE(IModRepositoryBridge, QObject, "_object")
  //     ;

  // py::class_<ModRepositoryFileInfo>("ModRepositoryFileInfo", py::no_init)
  //     .def(py::init<const ModRepositoryFileInfo &>(py::arg("other")))
  //     .def(py::init<py::optional<QString, int, int>>((py::arg("game_name"), "mod_id", "file_id")))
  //     .def("__str__", &ModRepositoryFileInfo::toString)
  //     .def("createFromJson", &ModRepositoryFileInfo::createFromJson, py::arg("data")).staticmethod("createFromJson")
  //     .def_readwrite("name", &ModRepositoryFileInfo::name)
  //     .def_readwrite("uri", &ModRepositoryFileInfo::uri)
  //     .def_readwrite("description", &ModRepositoryFileInfo::description)
  //     .def_readwrite("version", &ModRepositoryFileInfo::version)
  //     .def_readwrite("newestVersion", &ModRepositoryFileInfo::newestVersion)
  //     .def_readwrite("categoryID", &ModRepositoryFileInfo::categoryID)
  //     .def_readwrite("modName", &ModRepositoryFileInfo::modName)
  //     .def_readwrite("gameName", &ModRepositoryFileInfo::gameName)
  //     .def_readwrite("modID", &ModRepositoryFileInfo::modID)
  //     .def_readwrite("fileID", &ModRepositoryFileInfo::fileID)
  //     .def_readwrite("fileSize", &ModRepositoryFileInfo::fileSize)
  //     .def_readwrite("fileName", &ModRepositoryFileInfo::fileName)
  //     .def_readwrite("fileCategory", &ModRepositoryFileInfo::fileCategory)
  //     .def_readwrite("fileTime", &ModRepositoryFileInfo::fileTime)
  //     .def_readwrite("repository", &ModRepositoryFileInfo::repository)
  //     .def_readwrite("userData", &ModRepositoryFileInfo::userData)
  //     ;

  // py::class_<IDownloadManager, boost::noncopyable>("IDownloadManager", py::no_init)
  //     .def("startDownloadURLs", &IDownloadManager::startDownloadURLs, py::arg("urls"))
  //     .def("startDownloadNexusFile", &IDownloadManager::startDownloadNexusFile, (py::arg("mod_id"), "file_id"))
  //     .def("downloadPath", &IDownloadManager::downloadPath, py::arg("id"))
  //     .def("onDownloadComplete", &IDownloadManager::onDownloadComplete, py::arg("callback"))
  //     .def("onDownloadPaused", &IDownloadManager::onDownloadPaused, py::arg("callback"))
  //     .def("onDownloadFailed", &IDownloadManager::onDownloadFailed, py::arg("callback"))
  //     .def("onDownloadRemoved", &IDownloadManager::onDownloadRemoved, py::arg("callback"))
  //     ;

  // py::class_<IInstallationManager, boost::noncopyable>("IInstallationManager", py::no_init)
  //   .def("getSupportedExtensions", &IInstallationManager::getSupportedExtensions)
  //   .def("extractFile", &IInstallationManager::extractFile, (py::arg("entry"), py::arg("silent") = false))
  //   .def("extractFiles", &IInstallationManager::extractFiles, (py::arg("entries"), py::arg("silent") = false))
  //   .def("createFile", +[](IInstallationManager* m, std::shared_ptr<const MOBase::FileTreeEntry> entry) {
  //       return m->createFile(utils::clean_shared_ptr(entry));
  //     }, py::arg("entry"))

  //   // accept both QString and GuessedValue<QString> since the conversion is not automatic in Python, and
  //   // return a tuple to get back the mod name and the mod ID
  //   .def("installArchive", +[](IInstallationManager* m, std::variant<QString, GuessedValue<QString>> modName, QString archive, int modId) {
  //     GuessedValue<QString> tmp;
  //     if (auto* p = std::get_if<QString>(&modName)) {
  //       tmp = *p;
  //     }
  //     else {
  //       tmp = std::get<GuessedValue<QString>>(modName);
  //     }
  //     auto result = m->installArchive(tmp, archive, modId);
  //     return std::make_tuple(result, static_cast<QString>(tmp), modId);
  //   }, (py::arg("mod_name"), "archive", py::arg("mod_id") = 0))
  //   ;

  py::enum_<EndorsedState>(m, "EndorsedState")
    .value("ENDORSED_FALSE", EndorsedState::ENDORSED_FALSE)
    .value("ENDORSED_TRUE", EndorsedState::ENDORSED_TRUE)
    .value("ENDORSED_UNKNOWN", EndorsedState::ENDORSED_UNKNOWN)
    .value("ENDORSED_NEVER", EndorsedState::ENDORSED_NEVER)
    ;

  py::enum_<TrackedState>(m, "TrackedState")
    .value("TRACKED_FALSE", TrackedState::TRACKED_FALSE)
    .value("TRACKED_TRUE", TrackedState::TRACKED_TRUE)
    .value("TRACKED_UNKNOWN", TrackedState::TRACKED_UNKNOWN)
    ;

  // py::class_<IModInterface, boost::noncopyable>("IModInterface", py::no_init)
  //     .def("name", &IModInterface::name)
  //     .def("absolutePath", &IModInterface::absolutePath)

  //     .def("comments", &IModInterface::comments)
  //     .def("notes", &IModInterface::notes)
  //     .def("gameName", &IModInterface::gameName)
  //     .def("repository", &IModInterface::repository)
  //     .def("nexusId", &IModInterface::nexusId)
  //     .def("version", &IModInterface::version)
  //     .def("newestVersion", &IModInterface::newestVersion)
  //     .def("ignoredVersion", &IModInterface::ignoredVersion)
  //     .def("installationFile", &IModInterface::installationFile)
  //     .def("converted", &IModInterface::converted)
  //     .def("validated", &IModInterface::validated)
  //     .def("color", &IModInterface::color)
  //     .def("url", &IModInterface::url)
  //     .def("primaryCategory", &IModInterface::primaryCategory)
  //     .def("categories", &IModInterface::categories)
  //     .def("trackedState", &IModInterface::trackedState)
  //     .def("endorsedState", &IModInterface::endorsedState)
  //     .def("fileTree", &IModInterface::fileTree)
  //     .def("isOverwrite", &IModInterface::isOverwrite)
  //     .def("isBackup", &IModInterface::isBackup)
  //     .def("isSeparator", &IModInterface::isSeparator)
  //     .def("isForeign", &IModInterface::isForeign)

  //     .def("setVersion", &IModInterface::setVersion, py::arg("version"))
  //     .def("setNewestVersion", &IModInterface::setNewestVersion, py::arg("version"))
  //     .def("setIsEndorsed", &IModInterface::setIsEndorsed, py::arg("endorsed"))
  //     .def("setNexusID", &IModInterface::setNexusID, py::arg("nexus_id"))
  //     .def("addNexusCategory", &IModInterface::addNexusCategory, py::arg("category_id"))
  //     .def("addCategory", &IModInterface::addCategory, py::arg("name"))
  //     .def("removeCategory", &IModInterface::removeCategory, py::arg("name"))
  //     .def("setGameName", &IModInterface::setGameName, py::arg("name"))
  //     .def("setUrl", &IModInterface::setUrl, py::arg("url"))
  //     .def("pluginSetting", &IModInterface::pluginSetting, (py::arg("plugin_name"), "key", py::arg("default") = QVariant()))
  //     .def("pluginSettings", &IModInterface::pluginSettings, py::arg("plugin_name"))
  //     .def("setPluginSetting", &IModInterface::setPluginSetting, (py::arg("plugin_name"), "key", py::arg("value")))
  //     .def("clearPluginSettings", &IModInterface::clearPluginSettings, py::arg("plugin_name"))

  //     ;

  py::enum_<MOBase::EGuessQuality>(m, "GuessQuality")
      .value("INVALID", MOBase::GUESS_INVALID)
      .value("FALLBACK", MOBase::GUESS_FALLBACK)
      .value("GOOD", MOBase::GUESS_GOOD)
      .value("META", MOBase::GUESS_META)
      .value("PRESET", MOBase::GUESS_PRESET)
      .value("USER", MOBase::GUESS_USER)
      ;

  // py::class_<MOBase::GuessedValue<QString>, boost::noncopyable>("GuessedString")
  //     .def(py::init<>())
  //     .def(py::init<QString const&, EGuessQuality>((py::arg("value"), py::arg("quality") = EGuessQuality::GUESS_USER)))
  //     .def("update",
  //          static_cast<GuessedValue<QString>& (GuessedValue<QString>::*)(const QString&)>(&GuessedValue<QString>::update),
  //          py::return_self<>(), py::arg("value"))
  //     .def("update",
  //          static_cast<GuessedValue<QString>& (GuessedValue<QString>::*)(const QString&, EGuessQuality)>(&GuessedValue<QString>::update),
  //          py::return_self<>(), (py::arg("value"), "quality"))

  //     // Methods to simulate the assignment operator:
  //     .def("reset", +[](GuessedValue<QString>* gv) {
  //       *gv = GuessedValue<QString>(); }, py::return_self<>())
  //     .def("reset", +[](GuessedValue<QString>* gv, const QString& value, EGuessQuality eq) {
  //       *gv = GuessedValue<QString>(value, eq); }, py::return_self<>(), (py::arg("value"), "quality"))
  //     .def("reset", +[](GuessedValue<QString>* gv, const GuessedValue<QString>& other) {
  //       *gv = other; }, py::return_self<>(), py::arg("other"))

  //     // Use an intermediate lambda to avoid having to register the std::function conversion:
  //     .def("setFilter", +[](GuessedValue<QString>* gv, std::function<std::variant<QString, bool>(QString const&)> fn) {
  //       gv->setFilter([fn](QString& s) {
  //         auto ret = fn(s);
  //         return std::visit([&s](auto v) {
  //           if constexpr (std::is_same_v<decltype(v), QString>) {
  //             s = v;
  //             return true;
  //           }
  //           else if constexpr (std::is_same_v<decltype(v), bool>) {
  //             return v;
  //           }
  //         }, ret);
  //       });
  //     }, py::arg("filter"))

  //     // This makes a copy in python but it more practical than exposing an iterator:
  //     .def("variants", &GuessedValue<QString>::variants, py::return_value_policy<py::copy_const_reference>())
  //     .def("__str__", &MOBase::GuessedValue<QString>::operator const QString&, py::return_value_policy<py::copy_const_reference>())
  //     ;

  py::enum_<IPluginList::PluginState>(m, "PluginState")
      .value("missing", IPluginList::STATE_MISSING)
      .value("inactive", IPluginList::STATE_INACTIVE)
      .value("active", IPluginList::STATE_ACTIVE)

      .value("MISSING", IPluginList::STATE_MISSING)
      .value("INACTIVE", IPluginList::STATE_INACTIVE)
      .value("ACTIVE", IPluginList::STATE_ACTIVE)
      ;

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

  // py::class_<IPluginList, boost::noncopyable>("IPluginList", py::no_init)
  //     .def("state", &MOBase::IPluginList::state, py::arg("name"))
  //     .def("priority", &MOBase::IPluginList::priority, py::arg("name"))
  //     .def("setPriority", &MOBase::IPluginList::setPriority, (py::arg("name"), "priority"))
  //     .def("loadOrder", &MOBase::IPluginList::loadOrder, py::arg("name"))
  //     .def("isMaster", &MOBase::IPluginList::isMaster, py::arg("name"))
  //     .def("masters", &MOBase::IPluginList::masters, py::arg("name"))
  //     .def("origin", &MOBase::IPluginList::origin, py::arg("name"))
  //     .def("onRefreshed", &MOBase::IPluginList::onRefreshed, py::arg("callback"))
  //     .def("onPluginMoved", &MOBase::IPluginList::onPluginMoved, py::arg("callback"))

  //     // Kept but deprecated for backward compatibility:
  //     .def("onPluginStateChanged", +[](IPluginList* modList, const std::function<void(const QString&, IPluginList::PluginStates)>& fn) {
  //       utils::show_deprecation_warning("onPluginStateChanged",
  //         "onPluginStateChanged(Callable[[str, IPluginList.PluginStates], None]) is deprecated, "
  //         "use onPluginStateChanged(Callable[[Dict[str, IPluginList.PluginStates], None]) instead.");
  //       return modList->onPluginStateChanged([fn](auto const& map) {
  //         for (const auto& entry : map) {
  //           fn(entry.first, entry.second);
  //         }
  //         });
  //         }, py::arg("callback"))
  //     .def("onPluginStateChanged", &MOBase::IPluginList::onPluginStateChanged, py::arg("callback"))
  //     .def("pluginNames", &MOBase::IPluginList::pluginNames)
  //     .def("setState", &MOBase::IPluginList::setState, (py::arg("name"), "state"))
  //     .def("setLoadOrder", &MOBase::IPluginList::setLoadOrder, py::arg("loadorder"))
  //     ;

  // py::enum_<IModList::ModState>("ModState")
  //     .value("exists", IModList::STATE_EXISTS)
  //     .value("active", IModList::STATE_ACTIVE)
  //     .value("essential", IModList::STATE_ESSENTIAL)
  //     .value("empty", IModList::STATE_EMPTY)
  //     .value("endorsed", IModList::STATE_ENDORSED)
  //     .value("valid", IModList::STATE_VALID)
  //     .value("alternate", IModList::STATE_ALTERNATE)

  //     .value("EXISTS", IModList::STATE_EXISTS)
  //     .value("ACTIVE", IModList::STATE_ACTIVE)
  //     .value("ESSENTIAL", IModList::STATE_ESSENTIAL)
  //     .value("EMPTY", IModList::STATE_EMPTY)
  //     .value("ENDORSED", IModList::STATE_ENDORSED)
  //     .value("VALID", IModList::STATE_VALID)
  //     .value("ALTERNATE", IModList::STATE_ALTERNATE)
  //     ;

  // py::class_<IModList, boost::noncopyable>("IModList", py::no_init)
  //     .def("displayName", &MOBase::IModList::displayName, py::arg("name"))
  //     .def("allMods", &MOBase::IModList::allMods)
  //     .def("allModsByProfilePriority", &MOBase::IModList::allModsByProfilePriority, py::arg("profile") = py::ptr((IProfile*)nullptr))

  //     .def("getMod", &MOBase::IModList::getMod, py::return_value_policy<py::reference_existing_object>(), py::arg("name"))
  //     .def("removeMod", &MOBase::IModList::removeMod, py::arg("mod"))
  //     .def("renameMod", &MOBase::IModList::renameMod, py::return_value_policy<py::reference_existing_object>(), (py::arg("mod"), py::arg("name")))

  //     .def("state", &MOBase::IModList::state, py::arg("name"))
  //     .def("setActive",
  //       static_cast<int(IModList::*)(QStringList const&, bool)>(&MOBase::IModList::setActive), (py::arg("names"), "active"))
  //     .def("setActive",
  //       static_cast<bool(IModList::*)(QString const&, bool)>(&MOBase::IModList::setActive), (py::arg("name"), "active"))
  //     .def("priority", &MOBase::IModList::priority, py::arg("name"))
  //     .def("setPriority", &MOBase::IModList::setPriority, (py::arg("name"), "priority"))

  //     // Kept but deprecated for backward compatibility:
  //     .def("onModStateChanged", +[](IModList* modList, const std::function<void(const QString&, IModList::ModStates)>& fn) {
  //       utils::show_deprecation_warning("onModStateChanged",
  //         "onModStateChanged(Callable[[str, IModList.ModStates], None]) is deprecated, "
  //         "use onModStateChanged(Callable[[Dict[str, IModList.ModStates], None]) instead.");
  //       return modList->onModStateChanged([fn](auto const& map) {
  //         for (const auto& entry : map) {
  //           fn(entry.first, entry.second);
  //         }
  //         });
  //       }, py::arg("callback"))

  //     .def("onModInstalled", &MOBase::IModList::onModInstalled, py::arg("callback"))
  //     .def("onModRemoved", &MOBase::IModList::onModRemoved, py::arg("callback"))
  //     .def("onModStateChanged", &MOBase::IModList::onModStateChanged, py::arg("callback"))
  //     .def("onModMoved", &MOBase::IModList::onModMoved, py::arg("callback"))
  //     ;

  // // Note: localizedName, master, requirements and enabledByDefault have to go in all the plugin wrappers declaration,
  // // since the default functions are specific to each wrapper, otherwise in turns into an
  // // infinite recursion mess.
  // py::class_<IPluginWrapper, boost::noncopyable>("IPlugin")
  //   .def("init", py::pure_virtual(&MOBase::IPlugin::init), py::arg("organizer"))
  //   .def("name", py::pure_virtual(&MOBase::IPlugin::name))
  //   .def("localizedName", &MOBase::IPlugin::localizedName, &IPluginWrapper::localizedName_Default)
  //   .def("master", &MOBase::IPlugin::master, &IPluginWrapper::master_Default)
  //   .def("author", py::pure_virtual(&MOBase::IPlugin::author))
  //   .def("description", py::pure_virtual(&MOBase::IPlugin::description))
  //   .def("version", py::pure_virtual(&MOBase::IPlugin::version))
  //   .def("requirements", &MOBase::IPlugin::requirements, &IPluginWrapper::requirements_Default)
  //   .def("settings", py::pure_virtual(&MOBase::IPlugin::settings))
  //   .def("enabledByDefault", &MOBase::IPlugin::enabledByDefault, &IPluginWrapper::enabledByDefault_Default)
  //   ;

  // py::class_<IPluginDiagnoseWrapper, py::bases<IPlugin>, boost::noncopyable>("IPluginDiagnose")
  //     .def("localizedName", &MOBase::IPlugin::localizedName, &IPluginDiagnoseWrapper::localizedName_Default)
  //     .def("master", &MOBase::IPlugin::master, &IPluginDiagnoseWrapper::master_Default)
  //     .def("requirements", &MOBase::IPlugin::requirements, &IPluginDiagnoseWrapper::requirements_Default)
  //     .def("enabledByDefault", &MOBase::IPlugin::enabledByDefault, &IPluginDiagnoseWrapper::enabledByDefault_Default)

  //     .def("activeProblems", py::pure_virtual(&MOBase::IPluginDiagnose::activeProblems))
  //     .def("shortDescription", py::pure_virtual(&MOBase::IPluginDiagnose::shortDescription), py::arg("key"))
  //     .def("fullDescription", py::pure_virtual(&MOBase::IPluginDiagnose::fullDescription), py::arg("key"))
  //     .def("hasGuidedFix", py::pure_virtual(&MOBase::IPluginDiagnose::hasGuidedFix), py::arg("key"))
  //     .def("startGuidedFix", py::pure_virtual(&MOBase::IPluginDiagnose::startGuidedFix), py::arg("key"))
  //     .def("_invalidate", &IPluginDiagnoseWrapper::invalidate)
  //     ;

  // py::class_<Mapping>("Mapping", py::init<>())
  //     .def("__init__", py::make_constructor(+[](QString src, QString dst, bool dir, bool crt) -> Mapping* {
  //       return new Mapping{ src, dst, dir, crt };
  //       }, py::default_call_policies(),
  //         (py::arg("source"), py::arg("destination"), py::arg("is_directory"), py::arg("create_target") = false)))
  //     .def_readwrite("source", &Mapping::source)
  //     .def_readwrite("destination", &Mapping::destination)
  //     .def_readwrite("isDirectory", &Mapping::isDirectory)
  //     .def_readwrite("createTarget", &Mapping::createTarget)
  //     .def("__str__", +[](Mapping * m) {
  //       return fmt::format(L"Mapping({}, {}, {}, {})", m->source.toStdWString(), m->destination.toStdWString(), m->isDirectory, m->createTarget);
  //     })
  //     ;

  // py::class_<IPluginFileMapperWrapper, py::bases<IPlugin>, boost::noncopyable>("IPluginFileMapper")
  //     .def("localizedName", &MOBase::IPlugin::localizedName, &IPluginFileMapperWrapper::localizedName_Default)
  //     .def("master", &MOBase::IPlugin::master, &IPluginFileMapperWrapper::master_Default)
  //     .def("requirements", &MOBase::IPlugin::requirements, &IPluginFileMapperWrapper::requirements_Default)
  //     .def("enabledByDefault", &MOBase::IPlugin::enabledByDefault, &IPluginFileMapperWrapper::enabledByDefault_Default)

  //     .def("mappings", py::pure_virtual(&MOBase::IPluginFileMapper::mappings))
  //     ;

  // py::enum_<MOBase::IPluginGame::LoadOrderMechanism>("LoadOrderMechanism")
  //     .value("FileTime", MOBase::IPluginGame::LoadOrderMechanism::FileTime)
  //     .value("PluginsTxt", MOBase::IPluginGame::LoadOrderMechanism::PluginsTxt)

  //     .value("FILE_TIME", MOBase::IPluginGame::LoadOrderMechanism::FileTime)
  //     .value("PLUGINS_TXT", MOBase::IPluginGame::LoadOrderMechanism::PluginsTxt)
  //     ;

  // py::enum_<MOBase::IPluginGame::SortMechanism>("SortMechanism")
  //     .value("NONE", MOBase::IPluginGame::SortMechanism::NONE)
  //     .value("MLOX", MOBase::IPluginGame::SortMechanism::MLOX)
  //     .value("BOSS", MOBase::IPluginGame::SortMechanism::BOSS)
  //     .value("LOOT", MOBase::IPluginGame::SortMechanism::LOOT)
  //     ;

  // // This doesn't actually do the conversion, but might be convenient for accessing the names for enum bits
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

  // py::class_<IPluginGameWrapper, py::bases<IPlugin>, boost::noncopyable>("IPluginGame")
  //     .def("localizedName", &MOBase::IPlugin::localizedName, &IPluginGameWrapper::localizedName_Default)
  //     .def("master", &MOBase::IPlugin::master, &IPluginGameWrapper::master_Default)

  //     .def("detectGame", py::pure_virtual(&MOBase::IPluginGame::detectGame))
  //     .def("gameName", py::pure_virtual(&MOBase::IPluginGame::gameName))
  //     .def("initializeProfile", py::pure_virtual(&MOBase::IPluginGame::initializeProfile), (py::arg("directory"), "settings"))
  //     .def("listSaves", py::pure_virtual(&MOBase::IPluginGame::listSaves), py::arg("folder"))
  //     .def("isInstalled", py::pure_virtual(&MOBase::IPluginGame::isInstalled))
  //     .def("gameIcon", py::pure_virtual(&MOBase::IPluginGame::gameIcon))
  //     .def("gameDirectory", py::pure_virtual(&MOBase::IPluginGame::gameDirectory))
  //     .def("dataDirectory", py::pure_virtual(&MOBase::IPluginGame::dataDirectory))
  //     .def("setGamePath", py::pure_virtual(&MOBase::IPluginGame::setGamePath), py::arg("path"))
  //     .def("documentsDirectory", py::pure_virtual(&MOBase::IPluginGame::documentsDirectory))
  //     .def("savesDirectory", py::pure_virtual(&MOBase::IPluginGame::savesDirectory))
  //     .def("executables", py::pure_virtual(&MOBase::IPluginGame::executables))
  //     .def("executableForcedLoads", py::pure_virtual(&MOBase::IPluginGame::executableForcedLoads))
  //     .def("steamAPPId", py::pure_virtual(&MOBase::IPluginGame::steamAPPId))
  //     .def("primaryPlugins", py::pure_virtual(&MOBase::IPluginGame::primaryPlugins))
  //     .def("gameVariants", py::pure_virtual(&MOBase::IPluginGame::gameVariants))
  //     .def("setGameVariant", py::pure_virtual(&MOBase::IPluginGame::setGameVariant), py::arg("variant"))
  //     .def("binaryName", py::pure_virtual(&MOBase::IPluginGame::binaryName))
  //     .def("gameShortName", py::pure_virtual(&MOBase::IPluginGame::gameShortName))
  //     .def("primarySources", py::pure_virtual(&MOBase::IPluginGame::primarySources))
  //     .def("validShortNames", py::pure_virtual(&MOBase::IPluginGame::validShortNames))
  //     .def("gameNexusName", py::pure_virtual(&MOBase::IPluginGame::gameNexusName))
  //     .def("iniFiles", py::pure_virtual(&MOBase::IPluginGame::iniFiles))
  //     .def("DLCPlugins", py::pure_virtual(&MOBase::IPluginGame::DLCPlugins))
  //     .def("CCPlugins", py::pure_virtual(&MOBase::IPluginGame::CCPlugins))
  //     .def("loadOrderMechanism", py::pure_virtual(&MOBase::IPluginGame::loadOrderMechanism))
  //     .def("sortMechanism", py::pure_virtual(&MOBase::IPluginGame::sortMechanism))
  //     .def("nexusModOrganizerID", py::pure_virtual(&MOBase::IPluginGame::nexusModOrganizerID))
  //     .def("nexusGameID", py::pure_virtual(&MOBase::IPluginGame::nexusGameID))
  //     .def("looksValid", py::pure_virtual(&MOBase::IPluginGame::looksValid), py::arg("directory"))
  //     .def("gameVersion", py::pure_virtual(&MOBase::IPluginGame::gameVersion))
  //     .def("getLauncherName", py::pure_virtual(&MOBase::IPluginGame::getLauncherName))

  //     .def("featureList", +[](MOBase::IPluginGame* p) {
  //       // Constructing a dict from class name to actual object:
  //       py::dict dict;
  //       mp11::mp_for_each<
  //         // Must user pointers because mp_for_each construct object:
  //         mp11::mp_transform<std::add_pointer_t, MpGameFeaturesList>
  //       >([&](auto* pt) {
  //         using T = std::remove_pointer_t<decltype(pt)>;
  //         typename py::reference_existing_object::apply<T*>::type converter;

  //         // Retrieve the python class object:
  //         const py::converter::registration* registration = py::converter::registry::query(py::type_id<T>());
  //         py::object key = py::object(py::handle<>(py::borrowed(registration->get_class_object())));

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
  //         typename py::reference_existing_object::apply<T*>::type converter;

  //         // Retrieve the python class object:
  //         const py::converter::registration* registration = py::converter::registry::query(py::type_id<T>());

  //         if (clsObj.ptr() == (PyObject*) registration->get_class_object()) {
  //           feature = py::object(py::handle<>(converter(p->feature<T>())));
  //         }
  //         });
  //       return feature;
  //     }, py::arg("feature_type"))
  //     ;

  // py::enum_<MOBase::IPluginInstaller::EInstallResult>("InstallResult")
  //     .value("SUCCESS", MOBase::IPluginInstaller::RESULT_SUCCESS)
  //     .value("FAILED", MOBase::IPluginInstaller::RESULT_FAILED)
  //     .value("CANCELED", MOBase::IPluginInstaller::RESULT_CANCELED)
  //     .value("MANUAL_REQUESTED", MOBase::IPluginInstaller::RESULT_MANUALREQUESTED)
  //     .value("NOT_ATTEMPTED", MOBase::IPluginInstaller::RESULT_NOTATTEMPTED)
  //     ;

  // py::class_<IPluginInstaller, py::bases<IPlugin>, boost::noncopyable>("IPluginInstaller", py::no_init)
  //   .def("isArchiveSupported", &IPluginInstaller::isArchiveSupported, py::arg("tree"))
  //   .def("priority", &IPluginInstaller::priority)
  //   .def("onInstallationStart", &IPluginInstaller::onInstallationStart, (py::arg("archive"), py::arg("reinstallation"), py::arg("current_mod")))
  //   .def("onInstallationEnd", &IPluginInstaller::onInstallationEnd, (py::arg("result"), py::arg("new_mod")))
  //   .def("isManualInstaller", &IPluginInstaller::isManualInstaller)
  //   .def("setParentWidget", &IPluginInstaller::setParentWidget, py::arg("parent"))
  //   .def("setInstallationManager", &IPluginInstaller::setInstallationManager, py::arg("manager"))
  //   ;

  // py::class_<IPluginInstallerSimpleWrapper, py::bases<IPluginInstaller>, boost::noncopyable>("IPluginInstallerSimple")
  //   .def("onInstallationStart", &IPluginInstaller::onInstallationStart, (py::arg("archive"), py::arg("reinstallation"), py::arg("current_mod")))
  //   .def("onInstallationEnd", &IPluginInstaller::onInstallationEnd, (py::arg("result"), py::arg("new_mod")))
  //   .def("localizedName", &MOBase::IPlugin::localizedName, &IPluginInstallerSimpleWrapper::localizedName_Default)
  //   .def("master", &MOBase::IPlugin::master, &IPluginInstallerSimpleWrapper::master_Default)
  //   .def("requirements", &MOBase::IPlugin::requirements, &IPluginInstallerSimpleWrapper::requirements_Default)
  //   .def("enabledByDefault", &MOBase::IPlugin::enabledByDefault, &IPluginInstallerSimpleWrapper::enabledByDefault_Default)

  //   // Note: Keeping the variant here even if we always return a tuple to be consistent with the wrapper and
  //   // have proper stubs generation.
  //   .def("install", +[](IPluginInstallerSimple* p, GuessedValue<QString>& modName, std::shared_ptr<IFileTree>& tree, QString& version, int& nexusID)
  //     -> std::variant<IPluginInstaller::EInstallResult, std::shared_ptr<IFileTree>, std::tuple<IPluginInstaller::EInstallResult, std::shared_ptr<IFileTree>, QString, int>> {
  //       auto result = p->install(modName, tree, version, nexusID);
  //       return std::make_tuple(result, tree, version, nexusID);
  //     }, (py::arg("name"), "tree", "version", "nexus_id"))
  //   .def("_parentWidget", &IPluginInstallerSimpleWrapper::parentWidget, py::return_value_policy<py::return_by_value>())
  //   .def("_manager", &IPluginInstallerSimpleWrapper::manager, py::return_value_policy<py::reference_existing_object>())
  //   ;

  // py::class_<IPluginInstallerCustomWrapper, py::bases<IPluginInstaller>, boost::noncopyable>("IPluginInstallerCustom")
  //   .def("onInstallationStart", &IPluginInstaller::onInstallationStart, (py::arg("archive"), py::arg("reinstallation"), py::arg("current_mod")))
  //   .def("onInstallationEnd", &IPluginInstaller::onInstallationEnd, (py::arg("result"), py::arg("new_mod")))
  //   .def("localizedName", &MOBase::IPlugin::localizedName, &IPluginInstallerCustomWrapper::localizedName_Default)
  //   .def("master", &MOBase::IPlugin::master, &IPluginInstallerCustomWrapper::master_Default)
  //   .def("requirements", &MOBase::IPlugin::requirements, &IPluginInstallerCustomWrapper::requirements_Default)
  //   .def("enabledByDefault", &MOBase::IPlugin::enabledByDefault, &IPluginInstallerCustomWrapper::enabledByDefault_Default)

  //   // Needs to add both otherwize boost does not understand:
  //   .def("isArchiveSupported", &IPluginInstaller::isArchiveSupported, py::arg("tree"))
  //   .def("isArchiveSupported", &IPluginInstallerCustom::isArchiveSupported, py::arg("archive_name"))
  //   .def("supportedExtensions", &IPluginInstallerCustom::supportedExtensions)
  //   .def("install", &IPluginInstallerCustom::install, (py::arg("mod_name"), "game_name", "archive_name", "version", "nexus_id"))
  //   .def("_parentWidget", &IPluginInstallerSimpleWrapper::parentWidget, py::return_value_policy<py::return_by_value>())
  //   .def("_manager", &IPluginInstallerCustomWrapper::manager, py::return_value_policy<py::reference_existing_object>())
  //   ;

  // py::class_<IPluginModPageWrapper, py::bases<IPlugin>, boost::noncopyable>("IPluginModPage")
  //   .def("localizedName", &MOBase::IPlugin::localizedName, &IPluginModPageWrapper::localizedName_Default)
  //   .def("master", &MOBase::IPlugin::master, &IPluginModPageWrapper::master_Default)
  //   .def("requirements", &MOBase::IPlugin::requirements, &IPluginModPageWrapper::requirements_Default)
  //   .def("enabledByDefault", &MOBase::IPlugin::enabledByDefault, &IPluginModPageWrapper::enabledByDefault_Default)

  //   .def("displayName", py::pure_virtual(&IPluginModPage::displayName))
  //   .def("icon", py::pure_virtual(&IPluginModPage::icon))
  //   .def("pageURL", py::pure_virtual(&IPluginModPage::pageURL))
  //   .def("useIntegratedBrowser", py::pure_virtual(&IPluginModPage::useIntegratedBrowser))
  //   .def("handlesDownload", py::pure_virtual(&IPluginModPage::handlesDownload), (py::arg("page_url"), "download_url", "fileinfo"))
  //   .def("setParentWidget", &IPluginModPage::setParentWidget, &IPluginModPageWrapper::setParentWidget_Default, py::arg("parent"))
  //   .def("_parentWidget", &IPluginModPageWrapper::parentWidget, py::return_value_policy<py::return_by_value>())
  //   ;

  // py::class_<IPluginPreviewWrapper, py::bases<IPlugin>, boost::noncopyable>("IPluginPreview")
  //   .def("localizedName", &MOBase::IPlugin::localizedName, &IPluginPreviewWrapper::localizedName_Default)
  //   .def("master", &MOBase::IPlugin::master, &IPluginPreviewWrapper::master_Default)
  //   .def("requirements", &MOBase::IPlugin::requirements, &IPluginPreviewWrapper::requirements_Default)
  //   .def("enabledByDefault", &MOBase::IPlugin::enabledByDefault, &IPluginPreviewWrapper::enabledByDefault_Default)

  //   .def("supportedExtensions", py::pure_virtual(&IPluginPreview::supportedExtensions))
  //   .def("genFilePreview", py::pure_virtual(&IPluginPreview::genFilePreview), py::return_value_policy<py::return_by_value>(),
  //     (py::arg("filename"), "max_size"))
  //   ;

  // py::class_<IPluginToolWrapper, py::bases<IPlugin>, boost::noncopyable>("IPluginTool")
  //   .def("localizedName", &MOBase::IPlugin::localizedName, &IPluginToolWrapper::localizedName_Default)
  //   .def("master", &MOBase::IPlugin::master, &IPluginToolWrapper::master_Default)
  //   .def("requirements", &MOBase::IPlugin::requirements, &IPluginToolWrapper::requirements_Default)
  //   .def("enabledByDefault", &MOBase::IPlugin::enabledByDefault, &IPluginToolWrapper::enabledByDefault_Default)

  //   .def("displayName", py::pure_virtual(&IPluginTool::displayName))
  //   .def("tooltip", py::pure_virtual(&IPluginTool::tooltip))
  //   .def("icon", py::pure_virtual(&IPluginTool::icon))
  //   .def("display", py::pure_virtual(&IPluginTool::display))
  //   .def("setParentWidget", &IPluginTool::setParentWidget, &IPluginToolWrapper::setParentWidget_Default, py::arg("parent"))
  //   .def("_parentWidget", &IPluginToolWrapper::parentWidget, py::return_value_policy<py::return_by_value>())
  //   ;

  // registerGameFeaturesPythonConverters();

  m.def("getFileVersion", &MOBase::getFileVersion, py::arg("filepath"));
  m.def("getProductVersion", &MOBase::getProductVersion, py::arg("executable"));
  m.def("getIconForExecutable", &MOBase::iconForExecutable, py::arg("executable"));

  // py::object widgets(py::borrowed(PyImport_AddModule("mobase.widgets")));
  // py::scope().attr("widgets") = widgets;
  // {
  //   py::scope w_ = widgets;
  //   register_widgets();
  // }

  // Expose MoVariant: MoVariant is a fake object whose only purpose is to be used as a type-hint
  // on the python side (e.g., def foo(x: mobase.MoVariant)). The real MoVariant is defined in the
  // generated stubs, since it's only relevant when doing type-checking, but this needs to be defined,
  // otherwise MoVariant is not found when actually running plugins through MO2, making them crash.
  m.attr("MoVariant") = py::none();
}

/**
 *
 */
class PythonRunner : public IPythonRunner
{

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

  /**
   * @brief Append the underlying object of the given python object to the
   *     interface list if it is an instance (pointer) of the given type.
   *
   * @param obj The object to check.
   * @param interfaces The list to append the object to.
   *
   */
  template <class T>
  void appendIfInstance(py::object const& obj, QList<QObject*>& interfaces);

private:

  // For each "identifier" (python file or python module folder), contains the list
  // of python objects to keep "alive" during the execution.
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

PythonRunner::PythonRunner()
{
}

PythonRunner::~PythonRunner() {
  // We need the GIL lock when destroying Python objects.
  py::gil_scoped_acquire lock;
  // m_Interpreter.reset();

  // Boost.Python does not handle cyclic garbace collection, so we need to release
  // everything hold by the objects before deleting the objects themselves:
  // for (auto& [name, objects] : m_PythonObjects) {
  //   for (auto& obj : objects) {
  //     obj.attr("__dict__").attr("clear")();
  //   }
  // }

  // m_PythonObjects.clear();
}

static const char *argv0 = "ModOrganizer.exe";

struct PrintWrapper
{
  void write(const char * message)
  {
    buffer << message;
    if (buffer.tellp() != 0 && buffer.str().back() == '\n')
    {
      // actually put the string in a variable so it doesn't get destroyed as soon as we get a pointer to its data
      std::string string = buffer.str().substr(0, buffer.str().length() - 1);
      qDebug().nospace().noquote() << string.c_str();
      buffer = std::stringstream();
    }
  }

  std::stringstream buffer;
};

// ErrWrapper is in error.h

PYBIND11_MODULE(moprivate, m)
{
  py::class_<PrintWrapper>(m, "PrintWrapper")
    .def(py::init<>())
    .def("write", &PrintWrapper::write);
  py::class_<ErrWrapper>(m, "ErrWrapper")
    .def(py::init<>())
    .def_static("instance", &ErrWrapper::instance, py::return_value_policy::reference)
    .def("write", &ErrWrapper::write)
    .def("startRecordingExceptionMessage", &ErrWrapper::startRecordingExceptionMessage)
    .def("stopRecordingExceptionMessage", &ErrWrapper::stopRecordingExceptionMessage)
    .def("getLastExceptionMessage", &ErrWrapper::getLastExceptionMessage);

  // expose a function to create a particular tree, only for debugging purpose, not
  // in mobase.
  mo2::python::add_make_tree_function(m);

}

bool PythonRunner::initPython()
{
  if (Py_IsInitialized())
    return true;
  try {

    // we initialize the interpreter "the old way" because scoped_interpreter does not
    // seem to work well with PyQt
    wchar_t argBuffer[MAX_PATH];
    const size_t cSize = strlen(argv0) + 1;
    mbstowcs(argBuffer, argv0, MAX_PATH);
    Py_SetProgramName(argBuffer);

    PyImport_AppendInittab("mobase", &PyInit_mobase);
    PyImport_AppendInittab("moprivate", &PyInit_moprivate);

    Py_OptimizeFlag = 2;
    Py_NoSiteFlag = 1;

    initPath();
    Py_InitializeEx(0);

    if (!Py_IsInitialized()) {
      return false;
    }

    py::module_ mainModule = py::module_::import("__main__");
    py::object mainNamespace = mainModule.attr("__dict__");
    mainNamespace["sys"] = py::module_::import("sys");
    mainNamespace["moprivate"] = py::module_::import("moprivate");
    py::module_::import("site");
    py::exec("sys.stdout = moprivate.PrintWrapper()\n"
             "sys.stderr = moprivate.ErrWrapper.instance()\n"
             "sys.excepthook = lambda x, y, z: sys.__excepthook__(x, y, z)\n",
                        mainNamespace);

    mainNamespace["mobase"] = py::module_::import("mobase");
    mo2::python::configure_python_logging(mainNamespace["mobase"]);

    return true;
  } catch (const py::error_already_set&) {
    // construct an error to extract the message
    pyexcept::PythonError err;
    MOBase::log::error("failed to init python: {}", err.what());
    return false;
  }
}

void PythonRunner::initPath()
{
  static QStringList paths = {
    QCoreApplication::applicationDirPath() + "/pythoncore.zip",
    QCoreApplication::applicationDirPath() + "/pythoncore",
    IOrganizer::getPluginDataPath()
  };

  Py_SetPath(paths.join(';').toStdWString().c_str());
}

void PythonRunner::ensureFolderInPath(QString folder) {
  py::module_ sys = py::module_::import("sys");
  py::list sysPath = sys.attr("path");

  // Converting to QStringList for Qt::CaseInsensitive and because .index()
  // raise an exception:
  const QStringList currentPath = sysPath.cast<QStringList>();
  if (!currentPath.contains(folder, Qt::CaseInsensitive)) {
    sysPath.insert(0, folder);
  }
}

template <class T>
void PythonRunner::appendIfInstance(py::object const& obj, QList<QObject*> &interfaces) {
  if (py::isinstance<T*>(obj)) {
    interfaces.append(obj.cast<T*>());
  }
}

QList<QObject*> PythonRunner::load(const QString& identifier)
{
  py::gil_scoped_acquire lock;

  // `pluginName` can either be a python file (single-file plugin or a folder (whole module).
  //
  // For whole module, we simply add the parent folder to path, then we load the module with a simple
  // py::import, and we retrieve the associated __dict__ from which we extract either createPlugin or
  // createPlugins.
  //
  // For single file, we need to use py::eval_file, and we will use the context (global variables)
  // from __main__ (already contains mobase, and other required module). Since the context is shared
  // between called of `instantiate`, we need to make sure to remove createPlugin(s) from previous call.
  try {

    // Dictionary that will contain createPlugin() or createPlugins().
    py::dict moduleDict;

    if (identifier.endsWith(".py")) {
      py::object mainModule = py::module_::import("__main__");
      py::dict moduleNamespace = mainModule.attr("__dict__");

      std::string temp = ToString(identifier);
      py::eval_file(temp.c_str(), moduleNamespace).is_none();
      moduleDict = moduleNamespace;
    }
    else {
      // Retrieve the module name:
      QStringList parts = identifier.split("/");
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
        MOBase::log::error("Plugin {}: createPlugins must return a list.", identifier);
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
      MOBase::log::error("Plugin {}: missing a createPlugin(s) function.", identifier);
    }

    // If we have no plugins, there was an issue, and we already logged the problem:
    if (plugins.empty()) {
      return QList<QObject*>();
    }

    QList<QObject*> allInterfaceList;

    for (py::object pluginObj : plugins) {

      // Add the plugin to keep it alive:
      m_PythonObjects[identifier].push_back(pluginObj);

      QList<QObject*> interfaceList;

      // appendIfInstance<IPluginGame>(pluginObj, interfaceList);
      // Must try the wrapper because it's only a plugin extension interface in C++, so doesn't extend QObject
      // appendIfInstance<IPluginDiagnoseWrapper>(pluginObj, interfaceList);
      // Must try the wrapper because it's only a plugin extension interface in C++, so doesn't extend QObject
      // appendIfInstance<IPluginFileMapperWrapper>(pluginObj, interfaceList);
      // appendIfInstance<IPluginInstallerCustom>(pluginObj, interfaceList);
      // appendIfInstance<IPluginInstallerSimple>(pluginObj, interfaceList);
      // appendIfInstance<IPluginModPage>(pluginObj, interfaceList);
      // appendIfInstance<IPluginPreview>(pluginObj, interfaceList);
      // appendIfInstance<IPluginTool>(pluginObj, interfaceList);

      if (interfaceList.isEmpty()) {
        // appendIfInstance<IPluginWrapper>(pluginObj, interfaceList);
      }

      if (interfaceList.isEmpty()) {
        MOBase::log::error("Plugin {}: no plugin interface implemented.", identifier);
      }

      // Append the plugins to the main list:
      allInterfaceList.append(interfaceList);
    }

    return allInterfaceList;
  }
  catch (const py::error_already_set&) {
    MOBase::log::error("Failed to import plugin from {}.", identifier);
    throw pyexcept::PythonError();
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
      // from this plugin (whose __path__ points under this module, including
      // the module of the plugin itself).
      py::object sys = py::module_::import("sys");
      py::dict modules = sys.attr("modules");
      py::list keys = modules.attr("keys")();
      for (std::size_t i = 0; i < py::len(keys); ++i) {
        py::object mod = modules[keys[i]];
        if (PyObject_HasAttrString(mod.ptr(), "__path__")) {
          QString mpath = mod.attr("__path__")[0].cast<QString>();

          if (!folder.relativeFilePath(mpath).startsWith("..")) {
            // If the path is under identifier, we need to unload it.
            log::debug("Unloading module {} from {} for {}.", keys[i].cast<std::string>(), mpath, identifier);

            PyDict_DelItem(modules.ptr(), keys[i].ptr());
          }
        }
      }
    }

    // Boost.Python does not handle cyclic garbace collection, so we need to release
    // everything hold by the objects before deleting the objects themselves (done when
    // erasing from m_PythonObjects).
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
