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
#include <iplugintool.h>
#include <iprofile.h>
#include <log.h>

#include "uibasewrappers.h"
#include "proxypluginwrappers.h"
#include "gamefeatureswrappers.h"
#include "sipApiAccess.h"

#include <Windows.h>
#include <utility.h>
#include <QDir>
#include <QFile>
#include <QCoreApplication>
#include <QWidget>

#include <variant>
#include <tuple>

#ifndef Q_MOC_RUN
#include <boost/python.hpp>
#include <boost/mp11.hpp>
#endif

#include "tuple_helper.h"
#include "variant_helper.h"
#include "converters.h"
#include "shared_ptr_converter.h"
#include "pylogger.h"

using namespace MOBase;

namespace bpy = boost::python;
namespace mp11 = boost::mp11;

/**
 * This macro should be used within a bpy::class_ declaration and will define two
 * methods: __getattr__ and Name, where Name will simply return the object as a QClass*
 * object, while __getattr__ will delegate to the underlying QClass object when required.
 *
 * This allow access to Qt interface for object exposed using boost::python (e.g., signals,
 * methods from QObject or QWidget, etc.).
 */
#define Q_DELEGATE(Class, QClass, Name) \
  .def(Name, +[](Class* w) -> QClass* { return w; }, bpy::return_value_policy<bpy::reference_existing_object>())          \
  .def("__getattr__", +[](Class* w, bpy::str str) -> bpy::object {  \
    return bpy::object{ (QClass*)w }.attr(str);                               \
  })

BOOST_PYTHON_MODULE(mobase)
{
  PyEval_InitThreads();

  bpy::import("PyQt5.QtCore");
  bpy::import("PyQt5.QtWidgets");

  utils::register_qstring_converter();
  utils::register_qvariant_converter();

  utils::register_qclass_converter<QObject>();
  utils::register_qclass_converter<QDateTime>();
  utils::register_qclass_converter<QDir>();
  utils::register_qclass_converter<QFileInfo>();
  utils::register_qclass_converter<QWidget>();
  utils::register_qclass_converter<QMainWindow>();
  utils::register_qclass_converter<QIcon>();
  utils::register_qclass_converter<QSize>();
  utils::register_qclass_converter<QUrl>();

  // QFlags:
  utils::register_qflags_converter<IPluginList::PluginStates>();
  utils::register_qflags_converter<IPluginGame::ProfileSettings>();
  utils::register_qflags_converter<IModList::ModStates>();

  // Pointers:
  bpy::register_ptr_to_python<std::shared_ptr<FileTreeEntry>>();
  bpy::register_ptr_to_python<std::shared_ptr<const FileTreeEntry>>();
  bpy::implicitly_convertible<std::shared_ptr<FileTreeEntry>, std::shared_ptr<const FileTreeEntry>>();
  bpy::register_ptr_to_python<std::shared_ptr<IFileTree>>();
  bpy::register_ptr_to_python<std::shared_ptr<const IFileTree>>();
  bpy::implicitly_convertible<std::shared_ptr<IFileTree>, std::shared_ptr<const IFileTree>>();

  utils::shared_ptr_from_python<std::shared_ptr<const ISaveGame>>();
  bpy::register_ptr_to_python<std::shared_ptr<const ISaveGame>>();

  utils::shared_ptr_from_python<std::shared_ptr<const IPluginRequirement>>();
  bpy::register_ptr_to_python<std::shared_ptr<const IPluginRequirement>>();

  // Containers:
  utils::register_sequence_container<std::vector<int>>();
  utils::register_sequence_container<std::vector<unsigned int>>();
  utils::register_sequence_container<QList<ExecutableInfo>>();
  utils::register_sequence_container<QList<ExecutableForcedLoadSetting>>();
  utils::register_sequence_container<QList<PluginSetting>>();
  utils::register_sequence_container<QList<ModRepositoryFileInfo>>();
  utils::register_sequence_container<QStringList>();
  utils::register_sequence_container<QList<QString>>();
  utils::register_sequence_container<QList<QFileInfo>>();
  utils::register_sequence_container<QList<QVariant>>(); // Required for QVariant since this is QVariantList.
  utils::register_sequence_container<std::vector<std::shared_ptr<const FileTreeEntry>>>();
  utils::register_sequence_container<std::vector<std::shared_ptr<const ISaveGame>>>();
  utils::register_sequence_container<std::vector<std::shared_ptr<const IPluginRequirement>>>();
  utils::register_sequence_container<std::vector<ModDataContent::Content>>();
  utils::register_sequence_container<std::vector<Mapping>>();

  utils::register_set_container<std::set<QString>>();

  utils::register_associative_container<QMap<QString, QVariant>>(); // Required for QVariant since this is QVariantMap.
  utils::register_associative_container<QMap<QString, QStringList>>();
  utils::register_associative_container<std::map<QString, IModList::ModStates>>();
  utils::register_associative_container<std::map<QString, QVariant>>();

  utils::register_associative_container<IFileTree::OverwritesType>();

  utils::register_optional<std::optional<IPluginRequirement::Problem>>();

  // Tuple:
  bpy::register_tuple<std::tuple<bool, DWORD>>(); // IOrganizer::waitForApplication
  bpy::register_tuple<std::tuple<bool, bool>>();  // IProfile::invalidationActive
  bpy::register_tuple<std::tuple<IPluginInstaller::EInstallResult, std::shared_ptr<IFileTree>, QString, int>>();

  // Variants:
  bpy::register_variant<std::variant<
    IPluginInstaller::EInstallResult,
    std::shared_ptr<IFileTree>,
    std::tuple<IPluginInstaller::EInstallResult, std::shared_ptr<IFileTree>, QString, int>>>();
  bpy::register_variant<std::variant<IFileTree::OverwritesType, std::size_t>>();
  bpy::register_variant<std::variant<QString, bool>>();

  // Functions:
  utils::register_functor_converter<void()>(); // converter for the onRefreshed-callback
  utils::register_functor_converter<void(const QString&)>();
  utils::register_functor_converter<void(int)>();
  utils::register_functor_converter<void(const QString&, unsigned int)>();
  utils::register_functor_converter<void(const QString&, int, int)>(); // converter for the onModMoved-callback and onPluginMoved callbacks
  utils::register_functor_converter<void(const std::map<QString, IModList::ModStates>&)>(); // converter for the onModStateChanged-callback (IModList)
  utils::register_functor_converter<void(const std::map<QString, IPluginList::PluginStates>&)>(); // converter for the onPluginStateChanged-callback (IPluginList)
  utils::register_functor_converter<void(const QString&, const QString&, const QVariant&, const QVariant&)>();
  utils::register_functor_converter<void(QMainWindow*)>();
  utils::register_functor_converter<void(IProfile*), bpy::pointer_wrapper<IProfile*>>();
  utils::register_functor_converter<void(IProfile*, const QString&, const QString&), bpy::pointer_wrapper<IProfile*>>();
  utils::register_functor_converter<void(IProfile*, IProfile*), bpy::pointer_wrapper<IProfile*>>();
  utils::register_functor_converter<bool(const QString&)>();
  utils::register_functor_converter<IFileTree::WalkReturn(const QString&, std::shared_ptr<const FileTreeEntry>)>();
  utils::register_functor_converter<bool(const IOrganizer::FileInfo&)>();
  utils::register_functor_converter<bool(std::shared_ptr<FileTreeEntry> const&)>();
  utils::register_functor_converter<std::variant<QString, bool>(QString const&)>();
  utils::register_functor_converter<void(IModInterface *), bpy::pointer_wrapper<IModInterface*>>();
  utils::register_functor_converter<bool(IOrganizer*), bpy::pointer_wrapper<IOrganizer*>>();
  utils::register_functor_converter<void(const IPlugin*), bpy::pointer_wrapper<const IPlugin*>>();

  // This one is kept for backward-compatibility while we deprecate onModStateChanged for singl mod.
  utils::register_functor_converter<void(const QString&, IModList::ModStates)>(); // converter for the onModStateChanged-callback (IModList).
  utils::register_functor_converter<void(const QString&, IPluginList::PluginStates)>(); // converter for the onPluginStateChanged-callback (IPluginList).

  //
  // Class declarations:
  //

  bpy::enum_<MOBase::VersionInfo::ReleaseType>("ReleaseType")
      .value("final", MOBase::VersionInfo::RELEASE_FINAL)
      .value("candidate", MOBase::VersionInfo::RELEASE_CANDIDATE)
      .value("beta", MOBase::VersionInfo::RELEASE_BETA)
      .value("alpha", MOBase::VersionInfo::RELEASE_ALPHA)
      .value("prealpha", MOBase::VersionInfo::RELEASE_PREALPHA)

      .value("FINAL", MOBase::VersionInfo::RELEASE_FINAL)
      .value("CANDIDATE", MOBase::VersionInfo::RELEASE_CANDIDATE)
      .value("BETA", MOBase::VersionInfo::RELEASE_BETA)
      .value("ALPHA", MOBase::VersionInfo::RELEASE_ALPHA)
      .value("PRE_ALPHA", MOBase::VersionInfo::RELEASE_PREALPHA)
      ;

  bpy::enum_<MOBase::VersionInfo::VersionScheme>("VersionScheme")
      .value("discover", MOBase::VersionInfo::SCHEME_DISCOVER)
      .value("regular", MOBase::VersionInfo::SCHEME_REGULAR)
      .value("decimalmark", MOBase::VersionInfo::SCHEME_DECIMALMARK)
      .value("numbersandletters", MOBase::VersionInfo::SCHEME_NUMBERSANDLETTERS)
      .value("date", MOBase::VersionInfo::SCHEME_DATE)
      .value("literal", MOBase::VersionInfo::SCHEME_LITERAL)

      .value("DISCOVER", MOBase::VersionInfo::SCHEME_DISCOVER)
      .value("REGULAR", MOBase::VersionInfo::SCHEME_REGULAR)
      .value("DECIMAL_MARK", MOBase::VersionInfo::SCHEME_DECIMALMARK)
      .value("NUMBERS_AND_LETTERS", MOBase::VersionInfo::SCHEME_NUMBERSANDLETTERS)
      .value("DATE", MOBase::VersionInfo::SCHEME_DATE)
      .value("LITERAL", MOBase::VersionInfo::SCHEME_LITERAL)
      ;

  bpy::class_<VersionInfo>("VersionInfo")
      .def(bpy::init<QString, VersionInfo::VersionScheme>(
        (bpy::arg("value"), bpy::arg("scheme") = VersionInfo::SCHEME_DISCOVER)))
      // Note: Order of the two init<> below is important because ReleaseType is a simple enum with an
      // implicit int conversion.
      .def(bpy::init<int, int, int, int, VersionInfo::ReleaseType>(
        (bpy::arg("major"), "minor", "subminor", "subsubminor", bpy::arg("release_type") = VersionInfo::RELEASE_FINAL)))
      .def(bpy::init<int, int, int, VersionInfo::ReleaseType>(
        (bpy::arg("major"), "minor", "subminor", bpy::arg("release_type") = VersionInfo::RELEASE_FINAL)))
      .def("clear", &VersionInfo::clear)
      .def("parse", &VersionInfo::parse,
        (bpy::arg("value"), bpy::arg("scheme") = VersionInfo::SCHEME_DISCOVER, bpy::arg("is_manual") = false))
      .def("canonicalString", &VersionInfo::canonicalString)
      .def("displayString", &VersionInfo::displayString, bpy::arg("forced_segments") = 2)
      .def("isValid", &VersionInfo::isValid)
      .def("scheme", &VersionInfo::scheme)
      .def("__str__", &VersionInfo::canonicalString)
      .def(bpy::self < bpy::self)
      .def(bpy::self > bpy::self)
      .def(bpy::self <= bpy::self)
      .def(bpy::self >= bpy::self)
      .def(bpy::self != bpy::self)
      .def(bpy::self == bpy::self)
      ;

  bpy::class_<PluginSetting>(
    "PluginSetting", bpy::init<const QString&, const QString&, const QVariant&>(
      (bpy::arg("key"), "description", "default_value")))
    .def_readwrite("key", &PluginSetting::key)
    .def_readwrite("description", &PluginSetting::description)
    .def_readwrite("default_value", &PluginSetting::defaultValue);

  bpy::class_<ExecutableInfo>("ExecutableInfo",
    bpy::init<const QString&, const QFileInfo&>((bpy::arg("title"), "binary")))
      .def("withArgument", &ExecutableInfo::withArgument, bpy::return_self<>(), bpy::arg("argument"))
      .def("withWorkingDirectory", &ExecutableInfo::withWorkingDirectory, bpy::return_self<>(), bpy::arg("directory"))
      .def("withSteamAppId", &ExecutableInfo::withSteamAppId, bpy::return_self<>(), bpy::arg("app_id"))
      .def("asCustom", &ExecutableInfo::asCustom, bpy::return_self<>())
      .def("isValid", &ExecutableInfo::isValid)
      .def("title", &ExecutableInfo::title)
      .def("binary", &ExecutableInfo::binary)
      .def("arguments", &ExecutableInfo::arguments)
      .def("workingDirectory", &ExecutableInfo::workingDirectory)
      .def("steamAppID", &ExecutableInfo::steamAppID)
      .def("isCustom", &ExecutableInfo::isCustom)
      ;

  bpy::class_<ExecutableForcedLoadSetting>("ExecutableForcedLoadSetting",
    bpy::init<const QString&, const QString&>((bpy::arg("process"), "library")))
      .def("withForced", &ExecutableForcedLoadSetting::withForced, bpy::return_self<>(), bpy::arg("forced"))
      .def("withEnabled", &ExecutableForcedLoadSetting::withEnabled, bpy::return_self<>(), bpy::arg("enabled"))
      .def("enabled", &ExecutableForcedLoadSetting::enabled)
      .def("forced", &ExecutableForcedLoadSetting::forced)
      .def("library", &ExecutableForcedLoadSetting::library)
      .def("process", &ExecutableForcedLoadSetting::process)
      ;

  bpy::class_<ISaveGameWrapper, bpy::bases<>, boost::noncopyable>("ISaveGame")
      .def("getFilepath", bpy::pure_virtual(&ISaveGame::getFilepath))
      .def("getCreationTime", bpy::pure_virtual(&ISaveGame::getCreationTime))
      .def("getName", bpy::pure_virtual(&ISaveGame::getName))
      .def("getSaveGroupIdentifier", bpy::pure_virtual(&ISaveGame::getSaveGroupIdentifier))
      .def("allFiles", bpy::pure_virtual(&ISaveGame::allFiles))
      ;

  // See Q_DELEGATE for more details.
  bpy::class_<ISaveGameInfoWidgetWrapper, bpy::bases<>, ISaveGameInfoWidgetWrapper*, boost::noncopyable>(
    "ISaveGameInfoWidget", bpy::init<bpy::optional<QWidget*>>(bpy::arg("parent")))
    .def("setSave", bpy::pure_virtual(&ISaveGameInfoWidget::setSave), bpy::arg("save"))

    Q_DELEGATE(ISaveGameInfoWidget, QWidget, "_widget")
    ;

  // Plugin requirements:
  auto iPluginRequirementClass = bpy::class_<
    IPluginRequirementWrapper, bpy::bases<>, boost::noncopyable>("IPluginRequirement");
  {
    bpy::scope scope = iPluginRequirementClass;

    bpy::class_<IPluginRequirement::Problem>("Problem",
      bpy::init<QString, QString>((bpy::arg("short_description"), bpy::arg("long_description") = "")))
      .def("shortDescription", &IPluginRequirement::Problem::shortDescription)
      .def("longDescription", &IPluginRequirement::Problem::longDescription);

    iPluginRequirementClass
      .def("check", bpy::pure_virtual(&IPluginRequirement::check))
      ;
  }

  bpy::class_<PluginRequirementFactory, boost::noncopyable>("PluginRequirementFactory")
    // pluginDependency
    .def("pluginDependency", +[](QStringList const& pluginNames) {
      return PluginRequirementFactory::pluginDependency(pluginNames);
    }, bpy::arg("plugins"))
    .def("pluginDependency", +[](QString const& pluginName) {
      return PluginRequirementFactory::pluginDependency(pluginName);
    }, bpy::arg("plugin"))
    .staticmethod("pluginDependency")
    // gameDependency
    .def("gameDependency", +[](QStringList const& gameNames) {
    return PluginRequirementFactory::gameDependency(gameNames);
      }, bpy::arg("games"))
    .def("gameDependency", +[](QString const& gameNames) {
        return PluginRequirementFactory::gameDependency(gameNames);
      }, bpy::arg("game"))
    .staticmethod("gameDependency")
    // diagnose
    .def("diagnose", &PluginRequirementFactory::diagnose, bpy::arg("diagnose"))
    .staticmethod("diagnose")
    // basic
    .def("basic", &PluginRequirementFactory::basic, (bpy::arg("checker"), "description"))
    .staticmethod("basic");

  bpy::class_<IOrganizer::FileInfo>("FileInfo", bpy::init<>())
      .def_readwrite("filePath", &IOrganizer::FileInfo::filePath)
      .def_readwrite("archive", &IOrganizer::FileInfo::archive)
      .def_readwrite("origins", &IOrganizer::FileInfo::origins)
    ;

  bpy::class_<IOrganizer, boost::noncopyable>("IOrganizer", bpy::no_init)
      .def("createNexusBridge", &IOrganizer::createNexusBridge, bpy::return_value_policy<bpy::reference_existing_object>())
      .def("profileName", &IOrganizer::profileName)
      .def("profilePath", &IOrganizer::profilePath)
      .def("downloadsPath", &IOrganizer::downloadsPath)
      .def("overwritePath", &IOrganizer::overwritePath)
      .def("basePath", &IOrganizer::basePath)
      .def("modsPath", &IOrganizer::modsPath)
      .def("appVersion", &IOrganizer::appVersion)
      .def("createMod", &IOrganizer::createMod, bpy::return_value_policy<bpy::reference_existing_object>(), bpy::arg("name"))
      .def("getGame", &IOrganizer::getGame, bpy::return_value_policy<bpy::reference_existing_object>(), bpy::arg("name"))
      .def("modDataChanged", &IOrganizer::modDataChanged, bpy::arg("mod"))
      .def("isPluginEnabled", +[](IOrganizer* o, IPlugin* plugin) { return o->isPluginEnabled(plugin); }, bpy::arg("plugin"))
      .def("isPluginEnabled", +[](IOrganizer* o, QString const& plugin) { return o->isPluginEnabled(plugin); }, bpy::arg("plugin"))
      .def("pluginSetting", &IOrganizer::pluginSetting, (bpy::arg("plugin_name"), "key"))
      .def("setPluginSetting", &IOrganizer::setPluginSetting, (bpy::arg("plugin_name"), "key", "value"))
      .def("persistent", &IOrganizer::persistent, (bpy::arg("plugin_name"), "key", bpy::arg("default") = QVariant()))
      .def("setPersistent", &IOrganizer::setPersistent, (bpy::arg("plugin_name"), "key", "value", bpy::arg("sync") = true))
      .def("pluginDataPath", &IOrganizer::pluginDataPath)
      .def("installMod", &IOrganizer::installMod, bpy::return_value_policy<bpy::reference_existing_object>(), (bpy::arg("filename"), bpy::arg("name_suggestion") = ""))
      .def("resolvePath", &IOrganizer::resolvePath, bpy::arg("filename"))
      .def("listDirectories", &IOrganizer::listDirectories, bpy::arg("directory"))

      // Provide multiple overloads of findFiles:
      .def("findFiles", +[](const IOrganizer* o, QString const& p, std::function<bool(QString const&)> f) { return o->findFiles(p, f); },
        (bpy::arg("path"), "filter"))

      // In C++, it is possible to create a QStringList implicitly from a single QString. This is not possible with the current
      // converters in python (and I do not think it is a good idea to have it everywhere), but here it is nice to be able to
      // pass a single string, so we add an extra overload.
      // Important: the order matters, because a Python string can be converted to a QStringList since it is a sequence of
      // single-character strings:
      .def("findFiles", +[](const IOrganizer* o, QString const& p, const QStringList& gf) { return o->findFiles(p, gf); },
        (bpy::arg("path"), "patterns"))
      .def("findFiles", +[](const IOrganizer* o, QString const& p, const QString& f) { return o->findFiles(p, QStringList{ f }); },
        (bpy::arg("path"), "pattern"))

      .def("getFileOrigins", &IOrganizer::getFileOrigins, bpy::arg("filename"))
      .def("findFileInfos", &IOrganizer::findFileInfos, (bpy::arg("path"), "filter"))
      .def("downloadManager", &IOrganizer::downloadManager, bpy::return_value_policy<bpy::reference_existing_object>())
      .def("pluginList", &IOrganizer::pluginList, bpy::return_value_policy<bpy::reference_existing_object>())
      .def("modList", &IOrganizer::modList, bpy::return_value_policy<bpy::reference_existing_object>())
      .def("profile", &IOrganizer::profile, bpy::return_value_policy<bpy::reference_existing_object>())

      // Custom implementation for startApplication and waitForApplication because 1) HANDLE (= void*) is not properly
      // converted from/to python, and 2) we need to convert the by-ptr argument to a return-tuple for waitForApplication:
      .def("startApplication",
        +[](IOrganizer* o, const QString& executable, const QStringList& args, const QString& cwd, const QString& profile,
          const QString& forcedCustomOverwrite, bool ignoreCustomOverwrite) {
            return (std::uintptr_t) o->startApplication(executable, args, cwd, profile, forcedCustomOverwrite, ignoreCustomOverwrite);
        }, (bpy::arg("executable"), (bpy::arg("args") = QStringList()), (bpy::arg("cwd") = ""), (bpy::arg("profile") = ""),
            (bpy::arg("forcedCustomOverwrite") = ""), (bpy::arg("ignoreCustomOverwrite") = false)), bpy::return_value_policy<bpy::return_by_value>())
      .def("waitForApplication", +[](IOrganizer *o, std::uintptr_t handle) {
          DWORD returnCode;
          bool result = o->waitForApplication((HANDLE)handle, &returnCode);
          return std::make_tuple(result, returnCode);
        }, bpy::arg("handle"))
      .def("refresh", &IOrganizer::refresh, (bpy::arg("save_changes") = true))
      .def("managedGame", &IOrganizer::managedGame, bpy::return_value_policy<bpy::reference_existing_object>())

      .def("onAboutToRun", &IOrganizer::onAboutToRun, bpy::arg("callback"))
      .def("onFinishedRun", &IOrganizer::onFinishedRun, bpy::arg("callback"))
      .def("onUserInterfaceInitialized", &IOrganizer::onUserInterfaceInitialized, bpy::arg("callback"))
      .def("onProfileCreated", &IOrganizer::onProfileCreated, bpy::arg("callback"))
      .def("onProfileRenamed", &IOrganizer::onProfileRenamed, bpy::arg("callback"))
      .def("onProfileRemoved", &IOrganizer::onProfileRemoved, bpy::arg("callback"))
      .def("onProfileChanged", &IOrganizer::onProfileChanged, bpy::arg("callback"))

      .def("onPluginSettingChanged", &IOrganizer::onPluginSettingChanged, bpy::arg("callback"))
      .def("onPluginEnabled", +[](IOrganizer* o, std::function<void(const IPlugin*)> const& func) {
          o->onPluginEnabled(func);
        }, bpy::arg("callback"))
      .def("onPluginEnabled", +[](IOrganizer* o, QString const& name, std::function<void()> const& func) {
          o->onPluginEnabled(name, func);
        }, (bpy::arg("name"), bpy::arg("callback")))
      .def("onPluginDisabled", +[](IOrganizer* o, std::function<void(const IPlugin*)> const& func) {
          o->onPluginDisabled(func);
        }, bpy::arg("callback"))
      .def("onPluginDisabled", +[](IOrganizer* o, QString const& name, std::function<void()> const& func) {
          o->onPluginDisabled(name, func);
        }, (bpy::arg("name"), bpy::arg("callback")))

      // DEPRECATED:
      .def("getMod", +[](IOrganizer* o, QString const& name) {
          utils::show_deprecation_warning("getMod",
            "IOrganizer::getMod(str) is deprecated, use IModList::getMod(str) instead.");
          return o->modList()->getMod(name);
      }, bpy::return_value_policy<bpy::reference_existing_object>(), bpy::arg("name"))
      .def("removeMod", +[](IOrganizer* o, IModInterface *mod) {
          utils::show_deprecation_warning("removeMod",
            "IOrganizer::removeMod(IModInterface) is deprecated, use IModList::removeMod(IModInterface) instead.");
          return o->modList()->removeMod(mod);
      }, bpy::arg("mod"))
      .def("modsSortedByProfilePriority", +[](IOrganizer* o) {
          utils::show_deprecation_warning("modsSortedByProfilePriority",
            "IOrganizer::modsSortedByProfilePriority() is deprecated, use IModList::allModsByProfilePriority() instead.");
          return o->modList()->allModsByProfilePriority();
      })
      .def("refreshModList", +[](IOrganizer* o, bool s) {
        utils::show_deprecation_warning("refreshModList",
          "IOrganizer::refreshModList(bool) is deprecated, use IOrganizer::refresh(bool) instead.");
        o->refresh(s);
      }, (bpy::arg("save_changes") = true))
      .def("onModInstalled", +[](IOrganizer* organizer, const std::function<void(QString const&)>& func) {
        utils::show_deprecation_warning("onModInstalled",
          "IOrganizer::onModInstalled(Callable[[str], None]) is deprecated, "
          "use IModList::onModInstalled(Callable[[IModInterface], None]) instead.");
        return organizer->modList()->onModInstalled([func](MOBase::IModInterface* m) { func(m->name()); });;
      }, bpy::arg("callback"))

      .def("getPluginDataPath", &IOrganizer::getPluginDataPath)
      .staticmethod("getPluginDataPath")

      ;

  // FileTreeEntry Scope:
  auto fileTreeEntryClass = bpy::class_<FileTreeEntry, boost::noncopyable>("FileTreeEntry", bpy::no_init);
  {

    bpy::scope scope = fileTreeEntryClass;

    bpy::enum_<FileTreeEntry::FileTypes>("FileTypes")
    .value("FILE_OR_DIRECTORY", FileTreeEntry::FILE_OR_DIRECTORY)
    .value("FILE", FileTreeEntry::FILE)
    .value("DIRECTORY", FileTreeEntry::DIRECTORY)
    .export_values()
    ;

    fileTreeEntryClass

      .def("isFile", &FileTreeEntry::isFile)
      .def("isDir", &FileTreeEntry::isDir)
      // Forcing the conversion to FileTypeS to avoid having to expose FileType in python:
      .def("fileType", +[](FileTreeEntry* p) { return FileTreeEntry::FileTypes{ p->fileType() }; })
      // This should probably not be exposed in python since we provide automatic downcast:
      // .def("getTree", static_cast<std::shared_ptr<IFileTree>(FileTreeEntry::*)()>(&FileTreeEntry::astree))
      .def("name", &FileTreeEntry::name)
      .def("suffix", &FileTreeEntry::suffix)
      .def("hasSuffix", +[](FileTreeEntry* entry, QStringList suffixes) { return entry->hasSuffix(suffixes); }, bpy::arg("suffixes"))
      .def("hasSuffix", +[](FileTreeEntry* entry, QString suffix) { return entry->hasSuffix(suffix); }, bpy::arg("suffix"))
      .def("parent", static_cast<std::shared_ptr<IFileTree>(FileTreeEntry::*)()>(&FileTreeEntry::parent), "[optional]")
      .def("path", &FileTreeEntry::path, bpy::arg("sep") = "\\")
      .def("pathFrom", &FileTreeEntry::pathFrom, (bpy::arg("tree"), bpy::arg("sep") = "\\"))

      // Mutable operation:
      .def("detach", &FileTreeEntry::detach)
      .def("moveTo", &FileTreeEntry::moveTo, bpy::arg("tree"))

      // Special methods:
      .def("__eq__", +[](const FileTreeEntry* entry, QString other) {
        return entry->compare(other) == 0;
      })
      .def("__eq__", +[](const FileTreeEntry* entry, std::shared_ptr<FileTreeEntry> other) {
        return entry == other.get();
      })

      // Special methods for debug:
      .def("__repr__", +[](const FileTreeEntry* entry) { return "FileTreeEntry(\"" + entry->name() + "\")"; })
      ;
  }

  // IFileTree scope:
  auto iFileTreeClass = bpy::class_<IFileTree, bpy::bases<FileTreeEntry>, boost::noncopyable>("IFileTree", bpy::no_init);
  {

    bpy::scope scope = iFileTreeClass;

    bpy::enum_<IFileTree::InsertPolicy>("InsertPolicy")
      .value("FAIL_IF_EXISTS", IFileTree::InsertPolicy::FAIL_IF_EXISTS)
      .value("REPLACE", IFileTree::InsertPolicy::REPLACE)
      .value("MERGE", IFileTree::InsertPolicy::MERGE)
      .export_values()
      ;

    bpy::enum_<IFileTree::WalkReturn>("WalkReturn")
      .value("CONTINUE", IFileTree::WalkReturn::CONTINUE)
      .value("STOP", IFileTree::WalkReturn::STOP)
      .value("SKIP", IFileTree::WalkReturn::SKIP)
      .export_values()
      ;

    iFileTreeClass

      // Non-mutable operations:
      .def("exists", static_cast<bool(IFileTree::*)(QString, IFileTree::FileTypes) const>(&IFileTree::exists),
        (bpy::arg("path"), bpy::arg("type") = IFileTree::FILE_OR_DIRECTORY))
      .def("find", static_cast<std::shared_ptr<FileTreeEntry>(IFileTree::*)(QString, IFileTree::FileTypes)>(&IFileTree::find),
        bpy::return_value_policy<utils::downcast_return<FileTreeEntry, IFileTree>>(), (bpy::arg("path"), bpy::arg("type") = IFileTree::FILE_OR_DIRECTORY), "[optional]")
      .def("pathTo", &IFileTree::pathTo, (bpy::arg("entry"), bpy::arg("sep") = "\\"))

      // Note: walk() would probably be better as a generator in python, but it is likely impossible to construct
      // from the C++ walk() method.
      .def("walk", &IFileTree::walk, (bpy::arg("callback"), bpy::arg("sep") = "\\"))

      // Kind-of-static operations:
      .def("createOrphanTree", &IFileTree::createOrphanTree, bpy::arg("name") = "")

      // addFile() and addDirectory throws exception instead of returning null pointer in order
      // to have better traces.
      .def("addFile", +[](IFileTree* w, QString path, bool replaceIfExists) {
          auto result = w->addFile(path, replaceIfExists);
          if (result == nullptr) {
            throw std::logic_error("addFile failed");
          }
          return result;
        }, (bpy::arg("path"), bpy::arg("replace_if_exists") = false))
      .def("addDirectory", +[](IFileTree* w, QString path) {
          auto result = w->addDirectory(path);
          if (result == nullptr) {
            throw std::logic_error("addDirectory failed");
          }
          return result;
        }, bpy::arg("path"))

      // Merge needs custom return types depending if the user wants overrides or not. A failure is translated
      // into an exception for easier tracing and handling.
      .def("merge", +[](IFileTree* p, std::shared_ptr<IFileTree> other, bool returnOverwrites) -> std::variant<IFileTree::OverwritesType, std::size_t> {
            IFileTree::OverwritesType overwrites;
            auto result = p->merge(other, returnOverwrites ? &overwrites : nullptr);
            if (result == IFileTree::MERGE_FAILED) {
              throw std::logic_error("merge failed");
            }
            if (returnOverwrites) {
              return { overwrites };
            }
            return { result };
        }, (bpy::arg("other"), bpy::arg("overwrites") = false))

      // Insert and erase returns an iterator, which makes no sense in python, so we convert it to bool. Erase is also
      // renamed "remove" since "erase" is very C++.
      .def("insert", +[](IFileTree* p, std::shared_ptr<FileTreeEntry> entry, IFileTree::InsertPolicy insertPolicy) {
          return p->insert(entry, insertPolicy) == p->end();
        }, (bpy::arg("entry"), bpy::arg("policy") = IFileTree::InsertPolicy::FAIL_IF_EXISTS))

      .def("remove", +[](IFileTree* p, QString name) { return p->erase(name).first != p->end(); }, bpy::arg("name"))
      .def("remove", +[](IFileTree* p, std::shared_ptr<FileTreeEntry> entry) { return p->erase(entry) != p->end(); }, bpy::arg("entry"))

      .def("move", &IFileTree::move, (bpy::arg("entry"), "path", bpy::arg("policy") = IFileTree::InsertPolicy::FAIL_IF_EXISTS))
      .def("copy", +[](IFileTree* w, std::shared_ptr<FileTreeEntry> entry, QString path, IFileTree::InsertPolicy insertPolicy) {
        auto result = w->copy(entry, path, insertPolicy);
        if (result == nullptr) {
          throw std::logic_error("copy failed");
        }
        return result;
      }, (bpy::arg("entry"), bpy::arg("path") = "", bpy::arg("insert_policy") = IFileTree::InsertPolicy::FAIL_IF_EXISTS))

      .def("clear", &IFileTree::clear)
      .def("removeAll", &IFileTree::removeAll, bpy::arg("names"))
      .def("removeIf", &IFileTree::removeIf, bpy::arg("filter"))

      // Special methods:
      .def("__getitem__", static_cast<std::shared_ptr<FileTreeEntry>(IFileTree::*)(std::size_t)>(&IFileTree::at),
        bpy::return_value_policy<utils::downcast_return<FileTreeEntry, IFileTree>>())
      .def("__iter__", bpy::range<bpy::return_value_policy<utils::downcast_return<FileTreeEntry, IFileTree>>>(
        static_cast<IFileTree::iterator(IFileTree::*)()>(&IFileTree::begin),
        static_cast<IFileTree::iterator(IFileTree::*)()>(&IFileTree::end)))
      .def("__len__", &IFileTree::size)
      .def("__bool__", +[](const IFileTree* tree) { return !tree->empty(); })
      .def("__repr__", +[](const IFileTree* entry) { return "IFileTree(\"" + entry->name() + "\")"; })
      ;
  }


  bpy::class_<IProfile, boost::noncopyable>("IProfile", bpy::no_init)
      .def("name", &IProfile::name)
      .def("absolutePath", &IProfile::absolutePath)
      .def("localSavesEnabled", &IProfile::localSavesEnabled)
      .def("localSettingsEnabled", &IProfile::localSettingsEnabled)
      .def("invalidationActive", +[](const IProfile* p) {
        bool supported;
        bool active = p->invalidationActive(&supported);
        return std::make_tuple(active, supported);
      })
      .def("absoluteIniFilePath", &IProfile::absoluteIniFilePath, bpy::arg("inifile"))
      ;

  bpy::class_<IModRepositoryBridge, boost::noncopyable>("IModRepositoryBridge", bpy::no_init)
      .def("requestDescription", &IModRepositoryBridge::requestDescription, (bpy::arg("game_name"), "mod_id", "user_data"))
      .def("requestFiles", &IModRepositoryBridge::requestFiles, (bpy::arg("game_name"), "mod_id", "user_data"))
      .def("requestFileInfo", &IModRepositoryBridge::requestFileInfo, (bpy::arg("game_name"), "mod_id", "file_id", "user_data"))
      .def("requestDownloadURL", &IModRepositoryBridge::requestDownloadURL, (bpy::arg("game_name"), "mod_id", "file_id", "user_data"))
      .def("requestToggleEndorsement", &IModRepositoryBridge::requestToggleEndorsement, (bpy::arg("game_name"), "mod_id", "mod_version", "endorse", "user_data"))

      Q_DELEGATE(IModRepositoryBridge, QObject, "_object")
      ;

  bpy::class_<ModRepositoryFileInfo>("ModRepositoryFileInfo", bpy::no_init)
      .def(bpy::init<const ModRepositoryFileInfo &>(bpy::arg("other")))
      .def(bpy::init<bpy::optional<QString, int, int>>((bpy::arg("game_name"), "mod_id", "file_id")))
      .def("__str__", &ModRepositoryFileInfo::toString)
      .def("createFromJson", &ModRepositoryFileInfo::createFromJson, bpy::arg("data")).staticmethod("createFromJson")
      .def_readwrite("name", &ModRepositoryFileInfo::name)
      .def_readwrite("uri", &ModRepositoryFileInfo::uri)
      .def_readwrite("description", &ModRepositoryFileInfo::description)
      .def_readwrite("version", &ModRepositoryFileInfo::version)
      .def_readwrite("newestVersion", &ModRepositoryFileInfo::newestVersion)
      .def_readwrite("categoryID", &ModRepositoryFileInfo::categoryID)
      .def_readwrite("modName", &ModRepositoryFileInfo::modName)
      .def_readwrite("gameName", &ModRepositoryFileInfo::gameName)
      .def_readwrite("modID", &ModRepositoryFileInfo::modID)
      .def_readwrite("fileID", &ModRepositoryFileInfo::fileID)
      .def_readwrite("fileSize", &ModRepositoryFileInfo::fileSize)
      .def_readwrite("fileName", &ModRepositoryFileInfo::fileName)
      .def_readwrite("fileCategory", &ModRepositoryFileInfo::fileCategory)
      .def_readwrite("fileTime", &ModRepositoryFileInfo::fileTime)
      .def_readwrite("repository", &ModRepositoryFileInfo::repository)
      .def_readwrite("userData", &ModRepositoryFileInfo::userData)
      ;

  bpy::class_<IDownloadManager, boost::noncopyable>("IDownloadManager", bpy::no_init)
      .def("startDownloadURLs", &IDownloadManager::startDownloadURLs, bpy::arg("urls"))
      .def("startDownloadNexusFile", &IDownloadManager::startDownloadNexusFile, (bpy::arg("mod_id"), "file_id"))
      .def("downloadPath", &IDownloadManager::downloadPath, bpy::arg("id"))
      .def("onDownloadComplete", &IDownloadManager::onDownloadComplete, bpy::arg("callback"))
      .def("onDownloadPaused", &IDownloadManager::onDownloadPaused, bpy::arg("callback"))
      .def("onDownloadFailed", &IDownloadManager::onDownloadFailed, bpy::arg("callback"))
      .def("onDownloadRemoved", &IDownloadManager::onDownloadRemoved, bpy::arg("callback"))
      ;

  bpy::class_<IInstallationManager, boost::noncopyable>("IInstallationManager", bpy::no_init)
    .def("getSupportedExtensions", &IInstallationManager::getSupportedExtensions)
    .def("extractFile", &IInstallationManager::extractFile, (bpy::arg("entry"), bpy::arg("silent") = false))
    .def("extractFiles", &IInstallationManager::extractFiles, (bpy::arg("entries"), bpy::arg("silent") = false))
    .def("createFile", &IInstallationManager::createFile, bpy::arg("entry"))
    .def("installArchive", &IInstallationManager::installArchive, (bpy::arg("mod_name"), "archive", "mod_id"))
    ;

  bpy::enum_<EndorsedState>("EndorsedState")
    .value("ENDORSED_FALSE", EndorsedState::ENDORSED_FALSE)
    .value("ENDORSED_TRUE", EndorsedState::ENDORSED_TRUE)
    .value("ENDORSED_UNKNOWN", EndorsedState::ENDORSED_UNKNOWN)
    .value("ENDORSED_NEVER", EndorsedState::ENDORSED_NEVER)
    ;

  bpy::enum_<TrackedState>("TrackedState")
    .value("TRACKED_FALSE", TrackedState::TRACKED_FALSE)
    .value("TRACKED_TRUE", TrackedState::TRACKED_TRUE)
    .value("TRACKED_UNKNOWN", TrackedState::TRACKED_UNKNOWN)
    ;

  bpy::class_<IModInterface, boost::noncopyable>("IModInterface", bpy::no_init)
      .def("name", &IModInterface::name)
      .def("absolutePath", &IModInterface::absolutePath)

      .def("comments", &IModInterface::comments)
      .def("notes", &IModInterface::notes)
      .def("gameName", &IModInterface::gameName)
      .def("repository", &IModInterface::repository)
      .def("nexusId", &IModInterface::nexusId)
      .def("version", &IModInterface::version)
      .def("newestVersion", &IModInterface::newestVersion)
      .def("ignoredVersion", &IModInterface::ignoredVersion)
      .def("installationFile", &IModInterface::installationFile)
      .def("converted", &IModInterface::converted)
      .def("validated", &IModInterface::validated)
      .def("color", &IModInterface::color)
      .def("url", &IModInterface::url)
      .def("primaryCategory", &IModInterface::primaryCategory)
      .def("categories", &IModInterface::categories)
      .def("trackedState", &IModInterface::trackedState)
      .def("endorsedState", &IModInterface::endorsedState)
      .def("fileTree", &IModInterface::fileTree)

      .def("setVersion", &IModInterface::setVersion, bpy::arg("version"))
      .def("setNewestVersion", &IModInterface::setNewestVersion, bpy::arg("version"))
      .def("setIsEndorsed", &IModInterface::setIsEndorsed, bpy::arg("endorsed"))
      .def("setNexusID", &IModInterface::setNexusID, bpy::arg("nexus_id"))
      .def("addNexusCategory", &IModInterface::addNexusCategory, bpy::arg("category_id"))
      .def("addCategory", &IModInterface::addCategory, bpy::arg("name"))
      .def("removeCategory", &IModInterface::removeCategory, bpy::arg("name"))
      .def("setGameName", &IModInterface::setGameName, bpy::arg("name"))
      .def("setName", &IModInterface::setName, bpy::arg("name"))
      .def("setUrl", &IModInterface::setUrl, bpy::arg("url"))
      .def("remove", &IModInterface::remove)
      .def("pluginSetting", &IModInterface::pluginSetting, (bpy::arg("plugin_name"), "key", bpy::arg("default") = QVariant()))
      .def("pluginSettings", &IModInterface::pluginSettings, bpy::arg("plugin_name"))
      .def("setPluginSetting", &IModInterface::setPluginSetting, (bpy::arg("plugin_name"), "key", bpy::arg("value")))
      .def("clearPluginSettings", &IModInterface::clearPluginSettings, bpy::arg("plugin_name"))

      ;

  bpy::enum_<MOBase::EGuessQuality>("GuessQuality")
      .value("INVALID", MOBase::GUESS_INVALID)
      .value("FALLBACK", MOBase::GUESS_FALLBACK)
      .value("GOOD", MOBase::GUESS_GOOD)
      .value("META", MOBase::GUESS_META)
      .value("PRESET", MOBase::GUESS_PRESET)
      .value("USER", MOBase::GUESS_USER)
      ;

  bpy::class_<MOBase::GuessedValue<QString>, boost::noncopyable>("GuessedString")
      .def(bpy::init<>())
      .def(bpy::init<QString const&, EGuessQuality>((bpy::arg("value"), bpy::arg("quality") = EGuessQuality::GUESS_USER)))
      .def("update",
           static_cast<GuessedValue<QString>& (GuessedValue<QString>::*)(const QString&)>(&GuessedValue<QString>::update),
           bpy::return_self<>(), bpy::arg("value"))
      .def("update",
           static_cast<GuessedValue<QString>& (GuessedValue<QString>::*)(const QString&, EGuessQuality)>(&GuessedValue<QString>::update),
           bpy::return_self<>(), (bpy::arg("value"), "quality"))

      // Methods to simulate the assignment operator:
      .def("reset", +[](GuessedValue<QString>* gv) {
        *gv = GuessedValue<QString>(); }, bpy::return_self<>())
      .def("reset", +[](GuessedValue<QString>* gv, const QString& value, EGuessQuality eq) {
        *gv = GuessedValue<QString>(value, eq); }, bpy::return_self<>(), (bpy::arg("value"), "quality"))
      .def("reset", +[](GuessedValue<QString>* gv, const GuessedValue<QString>& other) {
        *gv = other; }, bpy::return_self<>(), bpy::arg("other"))

      // Use an intermediate lambda to avoid having to register the std::function conversion:
      .def("setFilter", +[](GuessedValue<QString>* gv, std::function<std::variant<QString, bool>(QString const&)> fn) {
        gv->setFilter([fn](QString& s) {
          auto ret = fn(s);
          return std::visit([&s](auto v) {
            if constexpr (std::is_same_v<decltype(v), QString>) {
              s = v;
              return true;
            }
            else if constexpr (std::is_same_v<decltype(v), bool>) {
              return v;
            }
          }, ret);
        });
      }, bpy::arg("filter"))

      // This makes a copy in python but it more practical than exposing an iterator:
      .def("variants", &GuessedValue<QString>::variants, bpy::return_value_policy<bpy::copy_const_reference>())
      .def("__str__", &MOBase::GuessedValue<QString>::operator const QString&, bpy::return_value_policy<bpy::copy_const_reference>())
      ;

  bpy::enum_<IPluginList::PluginState>("PluginState")
      .value("missing", IPluginList::STATE_MISSING)
      .value("inactive", IPluginList::STATE_INACTIVE)
      .value("active", IPluginList::STATE_ACTIVE)

      .value("MISSING", IPluginList::STATE_MISSING)
      .value("INACTIVE", IPluginList::STATE_INACTIVE)
      .value("ACTIVE", IPluginList::STATE_ACTIVE)
      ;

  bpy::class_<IPluginList, boost::noncopyable>("IPluginList", bpy::no_init)
      .def("state", &MOBase::IPluginList::state, bpy::arg("name"))
      .def("priority", &MOBase::IPluginList::priority, bpy::arg("name"))
      .def("setPriority", &MOBase::IPluginList::setPriority, (bpy::arg("name"), "priority"))
      .def("loadOrder", &MOBase::IPluginList::loadOrder, bpy::arg("name"))
      .def("isMaster", &MOBase::IPluginList::isMaster, bpy::arg("name"))
      .def("masters", &MOBase::IPluginList::masters, bpy::arg("name"))
      .def("origin", &MOBase::IPluginList::origin, bpy::arg("name"))
      .def("onRefreshed", &MOBase::IPluginList::onRefreshed, bpy::arg("callback"))
      .def("onPluginMoved", &MOBase::IPluginList::onPluginMoved, bpy::arg("callback"))

      // Kept but deprecated for backward compatibility:
      .def("onPluginStateChanged", +[](IPluginList* modList, const std::function<void(const QString&, IPluginList::PluginStates)>& fn) {
        utils::show_deprecation_warning("onPluginStateChanged",
          "onPluginStateChanged(Callable[[str, IPluginList.PluginStates], None]) is deprecated, "
          "use onPluginStateChanged(Callable[[Dict[str, IPluginList.PluginStates], None]) instead.");
        return modList->onPluginStateChanged([fn](auto const& map) {
          for (const auto& entry : map) {
            fn(entry.first, entry.second);
          }
          });
          }, bpy::arg("callback"))
      .def("onPluginStateChanged", &MOBase::IPluginList::onPluginStateChanged, bpy::arg("callback"))
      .def("pluginNames", &MOBase::IPluginList::pluginNames)
      .def("setState", &MOBase::IPluginList::setState, (bpy::arg("name"), "state"))
      .def("setLoadOrder", &MOBase::IPluginList::setLoadOrder, bpy::arg("loadorder"))
      ;

  bpy::enum_<IModList::ModState>("ModState")
      .value("exists", IModList::STATE_EXISTS)
      .value("active", IModList::STATE_ACTIVE)
      .value("essential", IModList::STATE_ESSENTIAL)
      .value("empty", IModList::STATE_EMPTY)
      .value("endorsed", IModList::STATE_ENDORSED)
      .value("valid", IModList::STATE_VALID)
      .value("alternate", IModList::STATE_ALTERNATE)

      .value("EXISTS", IModList::STATE_EXISTS)
      .value("ACTIVE", IModList::STATE_ACTIVE)
      .value("ESSENTIAL", IModList::STATE_ESSENTIAL)
      .value("EMPTY", IModList::STATE_EMPTY)
      .value("ENDORSED", IModList::STATE_ENDORSED)
      .value("VALID", IModList::STATE_VALID)
      .value("ALTERNATE", IModList::STATE_ALTERNATE)
      ;

  bpy::class_<IModList, boost::noncopyable>("IModList", bpy::no_init)
      .def("displayName", &MOBase::IModList::displayName, bpy::arg("name"))
      .def("allMods", &MOBase::IModList::allMods)
      .def("allModsByProfilePriority", &MOBase::IModList::allModsByProfilePriority, bpy::arg("profile") = bpy::ptr((IProfile*)nullptr))

      .def("getMod", &MOBase::IModList::getMod, bpy::return_value_policy<bpy::reference_existing_object>(), bpy::arg("name"))
      .def("removeMod", &MOBase::IModList::removeMod, bpy::arg("mod"))

      .def("state", &MOBase::IModList::state, bpy::arg("name"))
      .def("setActive",
        static_cast<int(IModList::*)(QStringList const&, bool)>(&MOBase::IModList::setActive), (bpy::arg("names"), "active"))
      .def("setActive",
        static_cast<bool(IModList::*)(QString const&, bool)>(&MOBase::IModList::setActive), (bpy::arg("name"), "active"))
      .def("priority", &MOBase::IModList::priority, bpy::arg("name"))
      .def("setPriority", &MOBase::IModList::setPriority, (bpy::arg("name"), "priority"))

      // Kept but deprecated for backward compatibility:
      .def("onModStateChanged", +[](IModList* modList, const std::function<void(const QString&, IModList::ModStates)>& fn) {
        utils::show_deprecation_warning("onModStateChanged",
          "onModStateChanged(Callable[[str, IModList.ModStates], None]) is deprecated, "
          "use onModStateChanged(Callable[[Dict[str, IModList.ModStates], None]) instead.");
        return modList->onModStateChanged([fn](auto const& map) {
          for (const auto& entry : map) {
            fn(entry.first, entry.second);
          }
          });
        }, bpy::arg("callback"))

      .def("onModInstalled", &MOBase::IModList::onModInstalled, bpy::arg("callback"))
      .def("onModRemoved", &MOBase::IModList::onModRemoved, bpy::arg("callback"))
      .def("onModStateChanged", &MOBase::IModList::onModStateChanged, bpy::arg("callback"))
      .def("onModMoved", &MOBase::IModList::onModMoved, bpy::arg("callback"))
      ;

  // Note: localizedName(), master() and requirements have to go in all the plugin wrappers declaration,
  // since the default functions are specific to each wrapper, otherwise in turns into an
  // infinite recursion mess.
  bpy::class_<IPluginWrapper, boost::noncopyable>("IPlugin")
    .def("init", bpy::pure_virtual(&MOBase::IPlugin::init), bpy::arg("organizer"))
    .def("name", bpy::pure_virtual(&MOBase::IPlugin::name))
    .def("localizedName", &MOBase::IPlugin::localizedName, &IPluginWrapper::localizedName_Default)
    .def("master", &MOBase::IPlugin::master, &IPluginToolWrapper::master_Default)
    .def("author", bpy::pure_virtual(&MOBase::IPlugin::author))
    .def("description", bpy::pure_virtual(&MOBase::IPlugin::description))
    .def("version", bpy::pure_virtual(&MOBase::IPlugin::version))
    .def("requirements", &MOBase::IPlugin::requirements, &IPluginWrapper::requirements_Default)
    .def("settings", bpy::pure_virtual(&MOBase::IPlugin::settings))
    ;

  bpy::class_<IPluginDiagnoseWrapper, bpy::bases<IPlugin>, boost::noncopyable>("IPluginDiagnose")
      .def("localizedName", &MOBase::IPlugin::localizedName, &IPluginDiagnoseWrapper::localizedName_Default)
      .def("master", &MOBase::IPlugin::master, &IPluginToolWrapper::master_Default)
      .def("requirements", &MOBase::IPlugin::requirements, &IPluginDiagnoseWrapper::requirements_Default)

      .def("activeProblems", bpy::pure_virtual(&MOBase::IPluginDiagnose::activeProblems))
      .def("shortDescription", bpy::pure_virtual(&MOBase::IPluginDiagnose::shortDescription), bpy::arg("key"))
      .def("fullDescription", bpy::pure_virtual(&MOBase::IPluginDiagnose::fullDescription), bpy::arg("key"))
      .def("hasGuidedFix", bpy::pure_virtual(&MOBase::IPluginDiagnose::hasGuidedFix), bpy::arg("key"))
      .def("startGuidedFix", bpy::pure_virtual(&MOBase::IPluginDiagnose::startGuidedFix), bpy::arg("key"))
      .def("_invalidate", &IPluginDiagnoseWrapper::invalidate)
      ;

  bpy::class_<Mapping>("Mapping", bpy::init<>())
      .def("__init__", bpy::make_constructor(+[](QString src, QString dst, bool dir, bool crt) -> Mapping* {
        return new Mapping{ src, dst, dir, crt };
        }, bpy::default_call_policies(),
          (bpy::arg("source"), bpy::arg("destination"), bpy::arg("is_directory"), bpy::arg("create_target") = false)))
      .def_readwrite("source", &Mapping::source)
      .def_readwrite("destination", &Mapping::destination)
      .def_readwrite("isDirectory", &Mapping::isDirectory)
      .def_readwrite("createTarget", &Mapping::createTarget)
      .def("__str__", +[](Mapping * m) {
        return fmt::format(L"Mapping({}, {}, {}, {})", m->source.toStdWString(), m->destination.toStdWString(), m->isDirectory, m->createTarget);
      })
      ;

  bpy::class_<IPluginFileMapperWrapper, bpy::bases<IPlugin>, boost::noncopyable>("IPluginFileMapper")
      .def("localizedName", &MOBase::IPlugin::localizedName, &IPluginFileMapperWrapper::localizedName_Default)
      .def("master", &MOBase::IPlugin::master, &IPluginToolWrapper::master_Default)
      .def("requirements", &MOBase::IPlugin::requirements, &IPluginFileMapperWrapper::requirements_Default)
      .def("mappings", bpy::pure_virtual(&MOBase::IPluginFileMapper::mappings))
      ;

  bpy::enum_<MOBase::IPluginGame::LoadOrderMechanism>("LoadOrderMechanism")
      .value("FileTime", MOBase::IPluginGame::LoadOrderMechanism::FileTime)
      .value("PluginsTxt", MOBase::IPluginGame::LoadOrderMechanism::PluginsTxt)

      .value("FILE_TIME", MOBase::IPluginGame::LoadOrderMechanism::FileTime)
      .value("PLUGINS_TXT", MOBase::IPluginGame::LoadOrderMechanism::PluginsTxt)
      ;

  bpy::enum_<MOBase::IPluginGame::SortMechanism>("SortMechanism")
      .value("NONE", MOBase::IPluginGame::SortMechanism::NONE)
      .value("MLOX", MOBase::IPluginGame::SortMechanism::MLOX)
      .value("BOSS", MOBase::IPluginGame::SortMechanism::BOSS)
      .value("LOOT", MOBase::IPluginGame::SortMechanism::LOOT)
      ;

  // This doesn't actually do the conversion, but might be convenient for accessing the names for enum bits
  bpy::enum_<MOBase::IPluginGame::ProfileSetting>("ProfileSetting")
      .value("mods", MOBase::IPluginGame::MODS)
      .value("configuration", MOBase::IPluginGame::CONFIGURATION)
      .value("savegames", MOBase::IPluginGame::SAVEGAMES)
      .value("preferDefaults", MOBase::IPluginGame::PREFER_DEFAULTS)

      .value("MODS", MOBase::IPluginGame::MODS)
      .value("CONFIGURATION", MOBase::IPluginGame::CONFIGURATION)
      .value("SAVEGAMES", MOBase::IPluginGame::SAVEGAMES)
      .value("PREFER_DEFAULTS", MOBase::IPluginGame::PREFER_DEFAULTS)
      ;

  bpy::class_<IPluginGameWrapper, bpy::bases<IPlugin>, boost::noncopyable>("IPluginGame")
      .def("localizedName", &MOBase::IPlugin::localizedName, &IPluginGameWrapper::localizedName_Default)
      .def("master", &MOBase::IPlugin::master, &IPluginToolWrapper::master_Default)

      .def("detectGame", bpy::pure_virtual(&MOBase::IPluginGame::detectGame))
      .def("gameName", bpy::pure_virtual(&MOBase::IPluginGame::gameName))
      .def("initializeProfile", bpy::pure_virtual(&MOBase::IPluginGame::initializeProfile), (bpy::arg("directory"), "settings"))
      .def("listSaves", bpy::pure_virtual(&MOBase::IPluginGame::listSaves), bpy::arg("folder"))
      .def("isInstalled", bpy::pure_virtual(&MOBase::IPluginGame::isInstalled))
      .def("gameIcon", bpy::pure_virtual(&MOBase::IPluginGame::gameIcon))
      .def("gameDirectory", bpy::pure_virtual(&MOBase::IPluginGame::gameDirectory))
      .def("dataDirectory", bpy::pure_virtual(&MOBase::IPluginGame::dataDirectory))
      .def("setGamePath", bpy::pure_virtual(&MOBase::IPluginGame::setGamePath), bpy::arg("path"))
      .def("documentsDirectory", bpy::pure_virtual(&MOBase::IPluginGame::documentsDirectory))
      .def("savesDirectory", bpy::pure_virtual(&MOBase::IPluginGame::savesDirectory))
      .def("executables", bpy::pure_virtual(&MOBase::IPluginGame::executables))
      .def("executableForcedLoads", bpy::pure_virtual(&MOBase::IPluginGame::executableForcedLoads))
      .def("steamAPPId", bpy::pure_virtual(&MOBase::IPluginGame::steamAPPId))
      .def("primaryPlugins", bpy::pure_virtual(&MOBase::IPluginGame::primaryPlugins))
      .def("gameVariants", bpy::pure_virtual(&MOBase::IPluginGame::gameVariants))
      .def("setGameVariant", bpy::pure_virtual(&MOBase::IPluginGame::setGameVariant), bpy::arg("variant"))
      .def("binaryName", bpy::pure_virtual(&MOBase::IPluginGame::binaryName))
      .def("gameShortName", bpy::pure_virtual(&MOBase::IPluginGame::gameShortName))
      .def("primarySources", bpy::pure_virtual(&MOBase::IPluginGame::primarySources))
      .def("validShortNames", bpy::pure_virtual(&MOBase::IPluginGame::validShortNames))
      .def("gameNexusName", bpy::pure_virtual(&MOBase::IPluginGame::gameNexusName))
      .def("iniFiles", bpy::pure_virtual(&MOBase::IPluginGame::iniFiles))
      .def("DLCPlugins", bpy::pure_virtual(&MOBase::IPluginGame::DLCPlugins))
      .def("CCPlugins", bpy::pure_virtual(&MOBase::IPluginGame::CCPlugins))
      .def("loadOrderMechanism", bpy::pure_virtual(&MOBase::IPluginGame::loadOrderMechanism))
      .def("sortMechanism", bpy::pure_virtual(&MOBase::IPluginGame::sortMechanism))
      .def("nexusModOrganizerID", bpy::pure_virtual(&MOBase::IPluginGame::nexusModOrganizerID))
      .def("nexusGameID", bpy::pure_virtual(&MOBase::IPluginGame::nexusGameID))
      .def("looksValid", bpy::pure_virtual(&MOBase::IPluginGame::looksValid), bpy::arg("directory"))
      .def("gameVersion", bpy::pure_virtual(&MOBase::IPluginGame::gameVersion))
      .def("getLauncherName", bpy::pure_virtual(&MOBase::IPluginGame::getLauncherName))

      .def("featureList", +[](MOBase::IPluginGame* p) {
        // Constructing a dict from class name to actual object:
        bpy::dict dict;
        mp11::mp_for_each<
          // Must user pointers because mp_for_each construct object:
          mp11::mp_transform<std::add_pointer_t, MpGameFeaturesList>
        >([&](auto* pt) {
          using T = std::remove_pointer_t<decltype(pt)>;
          typename bpy::reference_existing_object::apply<T*>::type converter;

          // Retrieve the python class object:
          const bpy::converter::registration* registration = bpy::converter::registry::query(bpy::type_id<T>());
          bpy::object key = bpy::object(bpy::handle<>(bpy::borrowed(registration->get_class_object())));

          // Set the object:
          dict[key] = bpy::handle<>(converter(p->feature<T>()));
        });
        return dict;
      })

      .def("feature", +[](MOBase::IPluginGame* p, bpy::object clsObj) {
        bpy::object feature;
        mp11::mp_for_each<
          // Must user pointers because mp_for_each construct object:
          mp11::mp_transform<std::add_pointer_t, MpGameFeaturesList>
        >([&](auto* pt) {
          using T = std::remove_pointer_t<decltype(pt)>;
          typename bpy::reference_existing_object::apply<T*>::type converter;

          // Retrieve the python class object:
          const bpy::converter::registration* registration = bpy::converter::registry::query(bpy::type_id<T>());

          if (clsObj.ptr() == (PyObject*) registration->get_class_object()) {
            feature = bpy::object(bpy::handle<>(converter(p->feature<T>())));
          }
          });
        return feature;
      }, bpy::arg("feature_type"))
      ;

  bpy::enum_<MOBase::IPluginInstaller::EInstallResult>("InstallResult")
      .value("SUCCESS", MOBase::IPluginInstaller::RESULT_SUCCESS)
      .value("FAILED", MOBase::IPluginInstaller::RESULT_FAILED)
      .value("CANCELED", MOBase::IPluginInstaller::RESULT_CANCELED)
      .value("MANUAL_REQUESTED", MOBase::IPluginInstaller::RESULT_MANUALREQUESTED)
      .value("NOT_ATTEMPTED", MOBase::IPluginInstaller::RESULT_NOTATTEMPTED)
      ;

  bpy::class_<IPluginInstaller, bpy::bases<IPlugin>, boost::noncopyable>("IPluginInstaller", bpy::no_init)
    .def("isArchiveSupported", &IPluginInstaller::isArchiveSupported, bpy::arg("tree"))
    .def("priority", &IPluginInstaller::priority)
    .def("onInstallationStart", &IPluginInstaller::onInstallationStart, (bpy::arg("archive"), bpy::arg("reinstallation"), bpy::arg("current_mod")))
    .def("onInstallationEnd", &IPluginInstaller::onInstallationEnd, (bpy::arg("result"), bpy::arg("new_mod")))
    .def("isManualInstaller", &IPluginInstaller::isManualInstaller)
    .def("setParentWidget", &IPluginInstaller::setParentWidget, bpy::arg("parent"))
    .def("setInstallationManager", &IPluginInstaller::setInstallationManager, bpy::arg("manager"))
    ;

  bpy::class_<IPluginInstallerSimpleWrapper, bpy::bases<IPluginInstaller>, boost::noncopyable>("IPluginInstallerSimple")
    .def("onInstallationStart", &IPluginInstaller::onInstallationStart, (bpy::arg("archive"), bpy::arg("reinstallation"), bpy::arg("current_mod")))
    .def("onInstallationEnd", &IPluginInstaller::onInstallationEnd, (bpy::arg("result"), bpy::arg("new_mod")))
    .def("localizedName", &MOBase::IPlugin::localizedName, &IPluginInstallerSimpleWrapper::localizedName_Default)
    .def("master", &MOBase::IPlugin::master, &IPluginToolWrapper::master_Default)
    .def("requirements", &MOBase::IPlugin::requirements, &IPluginInstallerSimpleWrapper::requirements_Default)

    // Note: Keeping the variant here even if we always return a tuple to be consistent with the wrapper and
    // have proper stubs generation.
    .def("install", +[](IPluginInstallerSimple* p, GuessedValue<QString>& modName, std::shared_ptr<IFileTree>& tree, QString& version, int& nexusID)
      -> std::variant<IPluginInstaller::EInstallResult, std::shared_ptr<IFileTree>, std::tuple<IPluginInstaller::EInstallResult, std::shared_ptr<IFileTree>, QString, int>> {
        auto result = p->install(modName, tree, version, nexusID);
        return std::make_tuple(result, tree, version, nexusID);
      }, (bpy::arg("name"), "tree", "version", "nexus_id"))
    .def("_parentWidget", &IPluginInstallerSimpleWrapper::parentWidget, bpy::return_value_policy<bpy::return_by_value>())
    .def("_manager", &IPluginInstallerSimpleWrapper::manager, bpy::return_value_policy<bpy::reference_existing_object>())
    ;

  bpy::class_<IPluginInstallerCustomWrapper, bpy::bases<IPluginInstaller>, boost::noncopyable>("IPluginInstallerCustom")
    .def("onInstallationStart", &IPluginInstaller::onInstallationStart, (bpy::arg("archive"), bpy::arg("reinstallation"), bpy::arg("current_mod")))
    .def("onInstallationEnd", &IPluginInstaller::onInstallationEnd, (bpy::arg("result"), bpy::arg("new_mod")))
    .def("localizedName", &MOBase::IPlugin::localizedName, &IPluginInstallerCustomWrapper::localizedName_Default)
    .def("master", &MOBase::IPlugin::master, &IPluginToolWrapper::master_Default)
    .def("requirements", &MOBase::IPlugin::requirements, &IPluginInstallerCustomWrapper::requirements_Default)

    // Needs to add both otherwize boost does not understand:
    .def("isArchiveSupported", &IPluginInstaller::isArchiveSupported, bpy::arg("tree"))
    .def("isArchiveSupported", &IPluginInstallerCustom::isArchiveSupported, bpy::arg("archive_name"))
    .def("supportedExtensions", &IPluginInstallerCustom::supportedExtensions)
    .def("install", &IPluginInstallerCustom::install, (bpy::arg("mod_name"), "game_name", "archive_name", "version", "nexus_id"))
    .def("_parentWidget", &IPluginInstallerSimpleWrapper::parentWidget, bpy::return_value_policy<bpy::return_by_value>())
    .def("_manager", &IPluginInstallerCustomWrapper::manager, bpy::return_value_policy<bpy::reference_existing_object>())
    ;

  bpy::class_<IPluginModPageWrapper, bpy::bases<IPlugin>, boost::noncopyable>("IPluginModPage")
    .def("localizedName", &MOBase::IPlugin::localizedName, &IPluginModPageWrapper::localizedName_Default)
    .def("master", &MOBase::IPlugin::master, &IPluginToolWrapper::master_Default)
    .def("requirements", &MOBase::IPlugin::requirements, &IPluginModPageWrapper::requirements_Default)

    .def("displayName", bpy::pure_virtual(&IPluginModPage::displayName))
    .def("icon", bpy::pure_virtual(&IPluginModPage::icon))
    .def("pageURL", bpy::pure_virtual(&IPluginModPage::pageURL))
    .def("useIntegratedBrowser", bpy::pure_virtual(&IPluginModPage::useIntegratedBrowser))
    .def("handlesDownload", bpy::pure_virtual(&IPluginModPage::handlesDownload), (bpy::arg("page_url"), "download_url", "fileinfo"))
    .def("setParentWidget", &IPluginModPage::setParentWidget, &IPluginModPageWrapper::setParentWidget_Default, bpy::arg("parent"))
    .def("_parentWidget", &IPluginModPageWrapper::parentWidget, bpy::return_value_policy<bpy::return_by_value>())
    ;

  bpy::class_<IPluginPreviewWrapper, bpy::bases<IPlugin>, boost::noncopyable>("IPluginPreview")
    .def("localizedName", &MOBase::IPlugin::localizedName, &IPluginPreviewWrapper::localizedName_Default)
    .def("master", &MOBase::IPlugin::master, &IPluginToolWrapper::master_Default)
    .def("requirements", &MOBase::IPlugin::requirements, &IPluginPreviewWrapper::requirements_Default)

    .def("supportedExtensions", bpy::pure_virtual(&IPluginPreview::supportedExtensions))
    .def("genFilePreview", bpy::pure_virtual(&IPluginPreview::genFilePreview), bpy::return_value_policy<bpy::return_by_value>(),
      (bpy::arg("filename"), "max_size"))
    ;

  bpy::class_<IPluginToolWrapper, bpy::bases<IPlugin>, boost::noncopyable>("IPluginTool")
    .def("localizedName", &MOBase::IPlugin::localizedName, &IPluginToolWrapper::localizedName_Default)
    .def("master", &MOBase::IPlugin::master, &IPluginToolWrapper::master_Default)
    .def("requirements", &MOBase::IPlugin::requirements, &IPluginToolWrapper::requirements_Default)

    .def("displayName", bpy::pure_virtual(&IPluginTool::displayName))
    .def("tooltip", bpy::pure_virtual(&IPluginTool::tooltip))
    .def("icon", bpy::pure_virtual(&IPluginTool::icon))
    .def("display", bpy::pure_virtual(&IPluginTool::display))
    .def("setParentWidget", &IPluginTool::setParentWidget, &IPluginToolWrapper::setParentWidget_Default, bpy::arg("parent"))
    .def("_parentWidget", &IPluginToolWrapper::parentWidget, bpy::return_value_policy<bpy::return_by_value>())
    ;

  registerGameFeaturesPythonConverters();

  bpy::def("getFileVersion", &MOBase::getFileVersion, bpy::arg("filepath"));
  bpy::def("getProductVersion", &MOBase::getProductVersion, bpy::arg("executable"));
  bpy::def("getIconForExecutable", &MOBase::iconForExecutable, bpy::arg("executable"));

  // Expose MoVariant: MoVariant is a fake object whose only purpose is to be used as a type-hint
  // on the python side (e.g., def foo(x: mobase.MoVariant)). The real MoVariant is defined in the
  // generated stubs, since it's only relevant when doing type-checking, but this needs to be defined,
  // otherwise MoVariant is not found when actually running plugins through MO2, making them crash.
  bpy::scope().attr("MoVariant") = bpy::object();
}

/**
 *
 */
class PythonRunner : public IPythonRunner
{

public:
  PythonRunner();
  ~PythonRunner();

  bool initPython(const QString& pythonDir);
  QList<QObject*> instantiate(const QString& pluginName);
  bool isPythonInstalled() const;
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
  void appendIfInstance(bpy::object const& obj, QList<QObject*>& interfaces);

private:

  // List of python objects representing plugins to keep all the bpy::object "alive"
  // during the execution.
  std::vector<bpy::object> m_PythonObjects;
  wchar_t* m_PythonHome;
};

IPythonRunner* CreatePythonRunner(const QString& pythonDir)
{
  PythonRunner* result = new PythonRunner;
  if (result->initPython(pythonDir)) {
    return result;
  }
  else {
    delete result;
    return nullptr;
  }
}

PythonRunner::PythonRunner()
{
  m_PythonHome = new wchar_t[MAX_PATH + 1];
}

PythonRunner::~PythonRunner() {
  // We need the GIL lock when destroying Python objects.
  GILock lock;
  m_PythonObjects.clear();
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

BOOST_PYTHON_MODULE(moprivate)
{
  bpy::class_<PrintWrapper, boost::noncopyable>("PrintWrapper", bpy::init<>())
    .def("write", &PrintWrapper::write);
  bpy::class_<ErrWrapper, boost::noncopyable>("ErrWrapper", bpy::init<>())
    .def("instance", &ErrWrapper::instance, bpy::return_value_policy<bpy::reference_existing_object>()).staticmethod("instance")
    .def("write", &ErrWrapper::write)
    .def("startRecordingExceptionMessage", &ErrWrapper::startRecordingExceptionMessage)
    .def("stopRecordingExceptionMessage", &ErrWrapper::stopRecordingExceptionMessage)
    .def("getLastExceptionMessage", &ErrWrapper::getLastExceptionMessage);

  utils::register_functor_converter<bool(QString, bool)>();

  // Expose a function to create a particular tree, only for debugging purpose, not in mobase.
  bpy::def("makeTree", +[](std::function<bool(QString, bool)> callback) -> std::shared_ptr<IFileTree> {
    struct FileTree : IFileTree {

      using callback_t = std::function<bool(QString, bool)>;

      FileTree(std::shared_ptr<const IFileTree> parent, QString name, callback_t callback) :
        FileTreeEntry(parent, name), IFileTree(), m_Callback(callback){ }

      std::shared_ptr<FileTreeEntry> addFile(QString name, bool) override {
        if (m_Callback && !m_Callback(name, false)) {
          throw UnsupportedOperationException("File rejected by callback.");
        }
        return IFileTree::addFile(name);
      }

      std::shared_ptr<IFileTree> addDirectory(QString name) override {
        if (m_Callback && !m_Callback(name, true)) {
          throw UnsupportedOperationException("Directory rejected by callback.");
        }
        return IFileTree::addDirectory(name);
      }

    protected:

      std::shared_ptr<IFileTree> makeDirectory(std::shared_ptr<const IFileTree> parent, QString name) const override {
        return std::make_shared<FileTree>(parent, name, m_Callback);
      }

      bool doPopulate(std::shared_ptr<const IFileTree> parent, std::vector<std::shared_ptr<FileTreeEntry>>& entries) const override { return true; }
      std::shared_ptr<IFileTree> doClone() const override { return std::make_shared<FileTree>(nullptr, name(), m_Callback); }

    private:
      callback_t m_Callback;
    };
    return std::make_shared<FileTree>(nullptr, "", callback);
  }, bpy::arg("callback") = bpy::object{});
}

bool PythonRunner::initPython(const QString &pythonPath)
{
  if (Py_IsInitialized())
    return true;
  try {
    if (!pythonPath.isEmpty() && !QFile::exists(pythonPath + "/python.exe")) {
      return false;
    }
    pythonPath.toWCharArray(m_PythonHome);
    if (!pythonPath.isEmpty()) {
      Py_SetPythonHome(m_PythonHome);
    }

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
      if (PyGILState_Check())
        PyEval_SaveThread();
      return false;
    }

    PySys_SetArgv(0, (wchar_t**)&argBuffer);

    auto os = bpy::import("os");
    auto mainModule = bpy::import("__main__");
    auto mainNamespace = mainModule.attr("__dict__");
    mainNamespace["os"] = os;
    mainNamespace["sys"] = bpy::import("sys");
    mainNamespace["moprivate"] = bpy::import("moprivate");
    mainNamespace["mobase"] = bpy::import("mobase");
    bpy::import("site");
    bpy::exec("sys.stdout = moprivate.PrintWrapper()\n"
              "sys.stderr = moprivate.ErrWrapper.instance()\n"
              "sys.excepthook = lambda x, y, z: sys.__excepthook__(x, y, z)\n",
                        mainNamespace);

    // add dlls directory
    const auto dlls = QCoreApplication::applicationDirPath().toStdString() + "/dlls";
    os.attr("add_dll_directory")(dlls);

    configure_python_logging(mainNamespace["mobase"]);

    PyEval_SaveThread();
    return true;
  } catch (const bpy::error_already_set&) {
    qDebug("failed to init python");
    PyErr_Print();
    if (PyErr_Occurred()) {
      PyErr_Print();
    } else {
      qCritical("An unexpected C++ exception was thrown in python code");
    }
    if (PyGILState_Check())
      PyEval_SaveThread();
    return false;
  }
}


bool handled_exec_file(bpy::str filename, bpy::object globals = bpy::object(), bpy::object locals = bpy::object())
{
  return bpy::handle_exception(std::bind<bpy::object(&)(bpy::str, bpy::object, bpy::object)>(bpy::exec_file, filename, globals, locals));
}


void PythonRunner::initPath()
{
  static QStringList paths = {
    QCoreApplication::applicationDirPath() + "/dlls/pythoncore.zip",
    QCoreApplication::applicationDirPath() + "/dlls/pythoncore",
    IOrganizer::getPluginDataPath()
  };

  Py_SetPath(paths.join(';').toStdWString().c_str());
}

void PythonRunner::ensureFolderInPath(QString folder) {
  bpy::object sys = bpy::import("sys");
  bpy::list sysPath = bpy::extract<bpy::list>(sys.attr("path"));

  // Converting to QStringList for Qt::CaseInsensitive and because .index()
  // raise an exception:
  QStringList currentPath = bpy::extract<QStringList>(sysPath);
  if (!currentPath.contains(folder, Qt::CaseInsensitive)) {
    sysPath.insert(0, folder);
  }
}



template <class T>
void PythonRunner::appendIfInstance(bpy::object const& obj, QList<QObject*> &interfaces) {
  bpy::extract<T*> extr{ obj };
  if (extr.check()) {
    interfaces.append(extr);
  }
}

QList<QObject*> PythonRunner::instantiate(const QString &pluginName)
{
  GILock lock;

  // `pluginName` can either be a python file (single-file plugin or a folder (whole module).
  //
  // For whole module, we simply add the parent folder to path, then we load the module with a simple
  // bpy::import, and we retrieve the associated __dict__ from which we extract either createPlugin or
  // createPlugins.
  //
  // For single file, we need to use bpy::exec_file, and we will use the context (global variables)
  // from __main__ (already contains mobase, and other required module). Since the context is shared
  // between called of `instantiate`, we need to make sure to remove createPlugin(s) from previous call.
  try {

    // Dictionary that will contain createPlugin() or createPlugins().
    bpy::dict moduleDict;

    if (pluginName.endsWith(".py")) {
      bpy::object mainModule = bpy::import("__main__");
      bpy::dict moduleNamespace = bpy::extract<bpy::dict>(mainModule.attr("__dict__"))();

      std::string temp = ToString(pluginName);
      if (!handled_exec_file(temp.c_str(), moduleNamespace)) {
        moduleDict = moduleNamespace;
      }
    }
    else {
      // Retrieve the module name:
      QStringList parts = pluginName.split("/");
      std::string moduleName = ToString(parts.takeLast());
      ensureFolderInPath(parts.join("/"));
      moduleDict = bpy::dict(bpy::import(moduleName.c_str()).attr("__dict__"));
    }

    if (bpy::len(moduleDict) == 0) {
      MOBase::log::error("Failed to import plugin from {}.", pluginName);
      throw pyexcept::PythonError();
    }

    // Create the plugins:
    std::vector<bpy::object> plugins;

    if (moduleDict.has_key("createPlugin")) {
      plugins.push_back(moduleDict["createPlugin"]());

      // Clear for future call
      bpy::delitem(moduleDict, bpy::str("createPlugin"));
    }
    else if (moduleDict.has_key("createPlugins")) {
      bpy::object pyPlugins = moduleDict["createPlugins"]();
      if (!PySequence_Check(pyPlugins.ptr())) {
        MOBase::log::error("Plugin {}: createPlugins must return a list.", pluginName);
      }
      else {
        bpy::list pyList(pyPlugins);
        int nPlugins = bpy::len(pyList);
        for (int i = 0; i < nPlugins; ++i) {
          plugins.push_back(pyList[i]);
        }
      }

      // Clear for future call
      bpy::delitem(moduleDict, bpy::str("createPlugins"));
    }
    else {
      MOBase::log::error("Plugin {}: missing a createPlugin(s) function.", pluginName);
    }

    // If we have no plugins, there was an issue, and we already logged the problem:
    if (plugins.empty()) {
      return QList<QObject*>();
    }

    QList<QObject*> allInterfaceList;

    for (bpy::object pluginObj : plugins) {

      // Add the plugin to keep it alive:
      m_PythonObjects.push_back(pluginObj);

      QList<QObject*> interfaceList;

      appendIfInstance<IPluginGame>(pluginObj, interfaceList);
      // Must try the wrapper because it's only a plugin extension interface in C++, so doesn't extend QObject
      appendIfInstance<IPluginDiagnoseWrapper>(pluginObj, interfaceList);
      // Must try the wrapper because it's only a plugin extension interface in C++, so doesn't extend QObject
      appendIfInstance<IPluginFileMapperWrapper>(pluginObj, interfaceList);
      appendIfInstance<IPluginInstallerCustom>(pluginObj, interfaceList);
      appendIfInstance<IPluginInstallerSimple>(pluginObj, interfaceList);
      appendIfInstance<IPluginModPage>(pluginObj, interfaceList);
      appendIfInstance<IPluginPreview>(pluginObj, interfaceList);
      appendIfInstance<IPluginTool>(pluginObj, interfaceList);

      if (interfaceList.isEmpty()) {
        appendIfInstance<IPluginWrapper>(pluginObj, interfaceList);
      }

      if (interfaceList.isEmpty()) {
        MOBase::log::error("Plugin {}: no plugin interface implemented.", pluginName);
      }

      // Append the plugins to the main list:
      allInterfaceList.append(interfaceList);
    }

    return allInterfaceList;
  }
  catch (const bpy::error_already_set&) {
    MOBase::log::error("Failed to import plugin from {}.", pluginName);
    throw pyexcept::PythonError();
  }
}

bool PythonRunner::isPythonInstalled() const
{
  return Py_IsInitialized() != 0;
}


bool PythonRunner::isPythonVersionSupported() const
{
  const char *version = Py_GetVersion();
  return strstr(version, "3.8") == version;
}

