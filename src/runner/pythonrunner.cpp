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
  bpy::register_ptr_to_python<std::shared_ptr<IFileTree>>();
  bpy::register_ptr_to_python<std::shared_ptr<const IFileTree>>();

  // Containers:
  utils::register_sequence_container<std::vector<int>>();
  utils::register_sequence_container<QList<ExecutableInfo>>();
  utils::register_sequence_container<QList<PluginSetting>>();
  utils::register_sequence_container<QList<ModRepositoryFileInfo>>();
  utils::register_sequence_container<QStringList>();
  utils::register_sequence_container<QList<QString>>();
  utils::register_sequence_container<QList<QFileInfo>>();
  utils::register_sequence_container<QList<QVariant>>(); // Required for QVariant since this is QVariantList.
  utils::register_sequence_container<std::vector<std::shared_ptr<const MOBase::FileTreeEntry>>>();
  utils::register_sequence_container<std::vector<ModDataContent::Content>>();

  utils::register_sequence_container<std::vector<unsigned int>>();
  utils::register_sequence_container<std::vector<Mapping>>();

  utils::register_set_container<std::set<QString>>();
  
  utils::register_associative_container<QMap<QString, QVariant>>(); // Required for QVariant since this is QVariantMap.
  utils::register_associative_container<QMap<QString, QStringList>>();
  
  utils::register_associative_container<IFileTree::OverwritesType>();

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
  utils::register_functor_converter<void(const QString&, unsigned int)>();
  utils::register_functor_converter<void(const QString&, IModList::ModStates)>(); // converter for the onModStateChanged-callback
  utils::register_functor_converter<bool(const IOrganizer::FileInfo&)>();
  utils::register_functor_converter<bool(const QString&)>();
  utils::register_functor_converter<bool(std::shared_ptr<FileTreeEntry> const&)>();
  utils::register_functor_converter<bool(QString const&, std::shared_ptr<const FileTreeEntry>)>();
  utils::register_functor_converter<std::variant<QString, bool>(QString const&)>();

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
      .def(bpy::init<QString>())
      .def(bpy::init<QString, VersionInfo::VersionScheme>())
      .def(bpy::init<int, int, int>())
      .def(bpy::init<int, int, int, VersionInfo::ReleaseType>())
      .def(bpy::init<int, int, int, int>())
      .def(bpy::init<int, int, int, int, VersionInfo::ReleaseType>())
      .def("clear", &VersionInfo::clear)
      .def("parse", &VersionInfo::parse)
      .def("canonicalString", &VersionInfo::canonicalString)
      .def("displayString", &VersionInfo::displayString)
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

  bpy::class_<PluginSetting>("PluginSetting", bpy::init<const QString&, const QString&, const QVariant&>());

  bpy::class_<ExecutableInfo>("ExecutableInfo", bpy::init<const QString&, const QFileInfo&>())
      .def("withArgument", &ExecutableInfo::withArgument, bpy::return_value_policy<bpy::reference_existing_object>())
      .def("withWorkingDirectory", &ExecutableInfo::withWorkingDirectory, bpy::return_value_policy<bpy::reference_existing_object>())
      .def("withSteamAppId", &ExecutableInfo::withSteamAppId, bpy::return_value_policy<bpy::reference_existing_object>())
      .def("asCustom", &ExecutableInfo::asCustom, bpy::return_value_policy<bpy::reference_existing_object>())
      .def("isValid", &ExecutableInfo::isValid)
      .def("title", &ExecutableInfo::title)
      .def("binary", &ExecutableInfo::binary)
      .def("arguments", &ExecutableInfo::arguments)
      .def("workingDirectory", &ExecutableInfo::workingDirectory)
      .def("steamAppID", &ExecutableInfo::steamAppID)
      .def("isCustom", &ExecutableInfo::isCustom)
      ;

  bpy::class_<ISaveGameWrapper, bpy::bases<>, ISaveGameWrapper*, boost::noncopyable>("ISaveGame")
      .def("getFilename", bpy::pure_virtual(&ISaveGame::getFilename))
      .def("getCreationTime", bpy::pure_virtual(&ISaveGame::getCreationTime))
      .def("getSaveGroupIdentifier", bpy::pure_virtual(&ISaveGame::getSaveGroupIdentifier))
      .def("allFiles", bpy::pure_virtual(&ISaveGame::allFiles))
      .def("hasScriptExtenderFile", bpy::pure_virtual(&ISaveGame::hasScriptExtenderFile))
      ;

  // See Q_DELEGATE for more details.
  bpy::class_<ISaveGameInfoWidgetWrapper, bpy::bases<>, ISaveGameInfoWidgetWrapper*, boost::noncopyable>("ISaveGameInfoWidget", bpy::init<bpy::optional<QWidget*>>())
    .def("setSave", bpy::pure_virtual(&ISaveGameInfoWidget::setSave))

    Q_DELEGATE(ISaveGameInfoWidget, QWidget, "_widget")
    ;

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
      .def("getMod", &IOrganizer::getMod, bpy::return_value_policy<bpy::reference_existing_object>())
      .def("createMod", &IOrganizer::createMod, bpy::return_value_policy<bpy::reference_existing_object>())
      .def("getGame", &IOrganizer::getGame, bpy::return_value_policy<bpy::reference_existing_object>())
      .def("removeMod", &IOrganizer::removeMod)
      .def("modDataChanged", &IOrganizer::modDataChanged)
      .def("pluginSetting", &IOrganizer::pluginSetting)
      .def("setPluginSetting", &IOrganizer::setPluginSetting)
      .def("persistent", &IOrganizer::persistent, bpy::arg("persistent") = QVariant())
      .def("setPersistent", &IOrganizer::setPersistent, bpy::arg("sync") = true)
      .def("pluginDataPath", &IOrganizer::pluginDataPath)
      .def("installMod", &IOrganizer::installMod, (bpy::arg("name_suggestion") = ""), bpy::return_value_policy<bpy::reference_existing_object>())
      .def("resolvePath", &IOrganizer::resolvePath)
      .def("listDirectories", &IOrganizer::listDirectories)

      // Provide multiple overloads of findFiles:
      .def("findFiles", +[](const IOrganizer* o, QString const& p, std::function<bool(QString const&)> f) { return o->findFiles(p, f); })

      // In C++, it is possible to create a QStringList implicitly from a single QString. This is not possible with the current
      // converters in python (and I do not think it is a good idea to have it everywhere), but here it is nice to be able to 
      // pass a single string, so we add an extra overload.
      // Important: the order matters, because a Python string can be converted to a QStringList since it iss a sequence of 
      // single-character strings:
      .def("findFiles", +[](const IOrganizer* o, QString const& p, const QStringList& gf) { return o->findFiles(p, gf); })
      .def("findFiles", +[](const IOrganizer* o, QString const& p, const QString& f) { return o->findFiles(p, QStringList{ f }); })

      .def("getFileOrigins", &IOrganizer::getFileOrigins)
      .def("findFileInfos", &IOrganizer::findFileInfos)
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
        }, ((bpy::arg("args") = QStringList()), (bpy::arg("cwd") = ""), (bpy::arg("profile") = ""), 
            (bpy::arg("forcedCustomOverwrite") = ""), (bpy::arg("ignoreCustomOverwrite") = false)), bpy::return_value_policy<bpy::return_by_value>())
      .def("waitForApplication", +[](IOrganizer *o, std::uintptr_t handle) {
          DWORD returnCode;
          bool result = o->waitForApplication((HANDLE)handle, &returnCode);
          return std::make_tuple(result, returnCode);
        })

      .def("onModInstalled", &IOrganizer::onModInstalled)
      .def("onAboutToRun", &IOrganizer::onAboutToRun)
      .def("onFinishedRun", &IOrganizer::onFinishedRun)
      .def("refreshModList", &IOrganizer::refreshModList, (bpy::arg("save_changes")=true))
      .def("managedGame", &IOrganizer::managedGame, bpy::return_value_policy<bpy::reference_existing_object>())
      .def("modsSortedByProfilePriority", &IOrganizer::modsSortedByProfilePriority)

      Q_DELEGATE(IOrganizer, QObject, "_object")
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
      .def("parent", static_cast<std::shared_ptr<IFileTree>(FileTreeEntry::*)()>(&FileTreeEntry::parent), "[optional]")
      .def("path", &FileTreeEntry::path, bpy::arg("sep") = "\\")
      .def("pathFrom", &FileTreeEntry::pathFrom, bpy::arg("sep") = "\\")

      // Mutable operation:
      .def("detach", &FileTreeEntry::detach)
      .def("moveTo", &FileTreeEntry::moveTo)

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
      .def("exists", static_cast<bool(IFileTree::*)(QString, IFileTree::FileTypes) const>(&IFileTree::exists), (bpy::arg("type") = IFileTree::FILE_OR_DIRECTORY))
      .def("find", static_cast<std::shared_ptr<FileTreeEntry>(IFileTree::*)(QString, IFileTree::FileTypes)>(&IFileTree::find), 
        bpy::arg("type") = IFileTree::FILE_OR_DIRECTORY, bpy::return_value_policy<utils::downcast_return<FileTreeEntry, IFileTree>>(), "[optional]")
      .def("pathTo", &IFileTree::pathTo, bpy::arg("sep") = "\\")

      // Note: walk() would probably be better as a generator in python, but it is likely impossible to construct
      // from the C++ walk() method.
      .def("walk", &IFileTree::walk, bpy::arg("sep") = "\\")

      // Kind-of-static operations:
      .def("createOrphanTree", &IFileTree::createOrphanTree, bpy::arg("name") = "")

      // addFile() and addDirectory throws exception instead of returning null pointer in order
      // to have better traces.
      .def("addFile", +[](IFileTree* w, QString name, bool replaceIfExists) {
          auto result = w->addFile(name, replaceIfExists);
          if (result == nullptr) {
            throw std::logic_error("addFile failed");
          }
          return result;
        }, bpy::arg("replace_if_exists") = false)
      .def("addDirectory", +[](IFileTree* w, QString name) {
          auto result = w->addDirectory(name);
          if (result == nullptr) {
            throw std::logic_error("addDirectory failed");
          }
          return result;
        })

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
        }, bpy::arg("overwrites") = false)

      // Insert and erase returns an iterator, which makes no sense in python, so we convert it to bool. Erase is also
      // renamed "remove" since "erase" is very C++.
      .def("insert", +[](IFileTree* p, std::shared_ptr<FileTreeEntry> entry, IFileTree::InsertPolicy insertPolicy) {
          return p->insert(entry, insertPolicy) == p->end();
        }, bpy::arg("policy") = IFileTree::InsertPolicy::FAIL_IF_EXISTS)

      .def("remove", +[](IFileTree* p, QString name) { return p->erase(name).first != p->end(); })
      .def("remove", +[](IFileTree* p, std::shared_ptr<FileTreeEntry> entry) { return p->erase(entry) != p->end(); })

      .def("move", &IFileTree::move, bpy::arg("policy") = IFileTree::InsertPolicy::FAIL_IF_EXISTS)
      .def("copy", +[](IFileTree* w, std::shared_ptr<const FileTreeEntry> entry, QString path, IFileTree::InsertPolicy insertPolicy) {
        auto result = w->copy(entry, path, insertPolicy);
        if (result == nullptr) {
          throw std::logic_error("copy failed");
        }
        return result;
      }, ((bpy::arg("path") = ""), (bpy::arg("insert_policy") = IFileTree::InsertPolicy::FAIL_IF_EXISTS)))

      .def("clear", &IFileTree::clear)
      .def("removeAll", &IFileTree::removeAll)
      .def("removeIf", &IFileTree::removeIf)

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
      ;

  bpy::class_<IModRepositoryBridge, boost::noncopyable>("IModRepositoryBridge", bpy::no_init)
      .def("requestDescription", &IModRepositoryBridge::requestDescription)
      .def("requestFiles", &IModRepositoryBridge::requestFiles)
      .def("requestFileInfo", &IModRepositoryBridge::requestFileInfo)
      .def("requestDownloadURL", &IModRepositoryBridge::requestDownloadURL)
      .def("requestToggleEndorsement", &IModRepositoryBridge::requestToggleEndorsement)

      Q_DELEGATE(IModRepositoryBridge, QObject, "_object")
      ;

  bpy::class_<ModRepositoryFileInfo>("ModRepositoryFileInfo", bpy::no_init)
      .def(bpy::init<const ModRepositoryFileInfo &>())
      .def(bpy::init<bpy::optional<QString, int, int>>())
      .def("__str__", &ModRepositoryFileInfo::toString)
      .def("createFromJson", &ModRepositoryFileInfo::createFromJson).staticmethod("createFromJson")
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
      .def("startDownloadURLs", &IDownloadManager::startDownloadURLs)
      .def("startDownloadNexusFile", &IDownloadManager::startDownloadNexusFile)
      .def("downloadPath", &IDownloadManager::downloadPath)

      Q_DELEGATE(IDownloadManager, QObject, "_object")
      ;

  bpy::class_<IInstallationManager, boost::noncopyable>("IInstallationManager", bpy::no_init)
    .def("extractFile", &IInstallationManager::extractFile)
    .def("extractFiles", &IInstallationManager::extractFiles)
    .def("createFile", &IInstallationManager::createFile)
    .def("installArchive", &IInstallationManager::installArchive)
    .def("setURL", &IInstallationManager::setURL)
    ;

  bpy::class_<IModInterface, boost::noncopyable>("IModInterface", bpy::no_init)
      .def("name", &IModInterface::name)
      .def("absolutePath", &IModInterface::absolutePath)
      .def("setVersion", &IModInterface::setVersion)
      .def("setNewestVersion", &IModInterface::setNewestVersion)
      .def("setIsEndorsed", &IModInterface::setIsEndorsed)
      .def("setNexusID", &IModInterface::setNexusID)
      .def("addNexusCategory", &IModInterface::addNexusCategory)
      .def("addCategory", &IModInterface::addCategory)
      .def("removeCategory", &IModInterface::removeCategory)
      .def("categories", &IModInterface::categories)
      .def("setGameName", &IModInterface::setGameName)
      .def("setName", &IModInterface::setName)
      .def("remove", &IModInterface::remove)
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
      .def(bpy::init<QString const&, EGuessQuality>())
      .def("update",
           static_cast<GuessedValue<QString>& (GuessedValue<QString>::*)(const QString&)>(&GuessedValue<QString>::update),
           bpy::return_self<>())
      .def("update",
           static_cast<GuessedValue<QString>& (GuessedValue<QString>::*)(const QString&, EGuessQuality)>(&GuessedValue<QString>::update),
           bpy::return_self<>())

      // Methods to simulate the assignment operator:
      .def("reset", +[](GuessedValue<QString>* gv) { *gv = GuessedValue<QString>(); }, bpy::return_self<>())
      .def("reset", +[](GuessedValue<QString>* gv, const QString& value, EGuessQuality eq) { *gv = GuessedValue<QString>(value, eq); }, bpy::return_self<>())
      .def("reset", +[](GuessedValue<QString>* gv, const GuessedValue<QString>& other) { *gv = other; }, bpy::return_self<>())

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
      })

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
      .def("state", &MOBase::IPluginList::state)
      .def("priority", &MOBase::IPluginList::priority)
      .def("loadOrder", &MOBase::IPluginList::loadOrder)
      .def("isMaster", &MOBase::IPluginList::isMaster)
      .def("masters", &MOBase::IPluginList::masters)
      .def("origin", &MOBase::IPluginList::origin)
      .def("onRefreshed", &MOBase::IPluginList::onRefreshed)
      .def("onPluginMoved", &MOBase::IPluginList::onPluginMoved)
      .def("pluginNames", &MOBase::IPluginList::pluginNames)
      .def("setState", &MOBase::IPluginList::setState)
      .def("setLoadOrder", &MOBase::IPluginList::setLoadOrder)
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
      .def("displayName", &MOBase::IModList::displayName)
      .def("allMods", &MOBase::IModList::allMods)
      .def("state", &MOBase::IModList::state)
      .def("setActive", &MOBase::IModList::setActive)
      .def("priority", &MOBase::IModList::priority)
      .def("setPriority", &MOBase::IModList::setPriority)
      .def("onModStateChanged", &MOBase::IModList::onModStateChanged)
      .def("onModMoved", &MOBase::IModList::onModMoved)
      ;

  bpy::class_<IPluginWrapper, boost::noncopyable>("IPlugin")
    .def("init", bpy::pure_virtual(&MOBase::IPlugin::init))
    .def("name", bpy::pure_virtual(&MOBase::IPlugin::name))
    .def("author", bpy::pure_virtual(&MOBase::IPlugin::author))
    .def("description", bpy::pure_virtual(&MOBase::IPlugin::description))
    .def("version", bpy::pure_virtual(&MOBase::IPlugin::version))
    .def("isActive", bpy::pure_virtual(&MOBase::IPlugin::isActive))
    .def("settings", bpy::pure_virtual(&MOBase::IPlugin::settings))
    ;

  bpy::class_<IPluginDiagnoseWrapper, bpy::bases<IPlugin>, boost::noncopyable>("IPluginDiagnose")
      .def("activeProblems", bpy::pure_virtual(&MOBase::IPluginDiagnose::activeProblems))
      .def("shortDescription", bpy::pure_virtual(&MOBase::IPluginDiagnose::shortDescription))
      .def("fullDescription", bpy::pure_virtual(&MOBase::IPluginDiagnose::fullDescription))
      .def("hasGuidedFix", bpy::pure_virtual(&MOBase::IPluginDiagnose::hasGuidedFix))
      .def("startGuidedFix", bpy::pure_virtual(&MOBase::IPluginDiagnose::startGuidedFix))
      .def("_invalidate", &IPluginDiagnoseWrapper::invalidate)
      ;

  bpy::class_<Mapping>("Mapping")
      .def_readwrite("source", &Mapping::source)
      .def_readwrite("destination", &Mapping::destination)
      .def_readwrite("isDirectory", &Mapping::isDirectory)
      .def_readwrite("createTarget", &Mapping::createTarget)
      ;

  bpy::class_<IPluginFileMapperWrapper, bpy::bases<IPlugin>, boost::noncopyable>("IPluginFileMapper")
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
      .def("gameName", bpy::pure_virtual(&MOBase::IPluginGame::gameName))
      .def("initializeProfile", bpy::pure_virtual(&MOBase::IPluginGame::initializeProfile))
      .def("savegameExtension", bpy::pure_virtual(&MOBase::IPluginGame::savegameExtension))
      .def("savegameSEExtension", bpy::pure_virtual(&MOBase::IPluginGame::savegameSEExtension))
      .def("isInstalled", bpy::pure_virtual(&MOBase::IPluginGame::isInstalled))
      .def("gameIcon", bpy::pure_virtual(&MOBase::IPluginGame::gameIcon))
      .def("gameDirectory", bpy::pure_virtual(&MOBase::IPluginGame::gameDirectory))
      .def("dataDirectory", bpy::pure_virtual(&MOBase::IPluginGame::dataDirectory))
      .def("setGamePath", bpy::pure_virtual(&MOBase::IPluginGame::setGamePath))
      .def("documentsDirectory", bpy::pure_virtual(&MOBase::IPluginGame::documentsDirectory))
      .def("savesDirectory", bpy::pure_virtual(&MOBase::IPluginGame::savesDirectory))
      .def("executables", bpy::pure_virtual(&MOBase::IPluginGame::executables))
      .def("steamAPPId", bpy::pure_virtual(&MOBase::IPluginGame::steamAPPId))
      .def("primaryPlugins", bpy::pure_virtual(&MOBase::IPluginGame::primaryPlugins))
      .def("gameVariants", bpy::pure_virtual(&MOBase::IPluginGame::gameVariants))
      .def("setGameVariant", bpy::pure_virtual(&MOBase::IPluginGame::setGameVariant))
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
      .def("looksValid", bpy::pure_virtual(&MOBase::IPluginGame::looksValid))
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
      })
      ;

  bpy::enum_<MOBase::IPluginInstaller::EInstallResult>("InstallResult")
      .value("SUCCESS", MOBase::IPluginInstaller::RESULT_SUCCESS)
      .value("FAILED", MOBase::IPluginInstaller::RESULT_FAILED)
      .value("CANCELED", MOBase::IPluginInstaller::RESULT_CANCELED)
      .value("MANUAL_REQUESTED", MOBase::IPluginInstaller::RESULT_MANUALREQUESTED)
      .value("NOT_ATTEMPTED", MOBase::IPluginInstaller::RESULT_NOTATTEMPTED)
      ;

  bpy::class_<IPluginInstaller, bpy::bases<IPlugin>, boost::noncopyable>("IPluginInstaller", bpy::no_init)
    .def("isArchiveSupported", &IPluginInstaller::isArchiveSupported)
    .def("priority", &IPluginInstaller::priority)
    .def("isManualInstaller", &IPluginInstaller::isManualInstaller)
    .def("setParentWidget", &IPluginInstaller::setParentWidget)
    .def("setInstallationManager", &IPluginInstaller::setInstallationManager)
    ;

  bpy::class_<IPluginInstallerSimpleWrapper, bpy::bases<IPluginInstaller>, boost::noncopyable>("IPluginInstallerSimple")
    // Note: Keeping the variant here even if we always return a tuple to be consistent with the wrapper and
    // have proper stubs generation.
    .def("install", +[](IPluginInstallerSimple* p, GuessedValue<QString>& modName, std::shared_ptr<IFileTree>& tree, QString& version, int& nexusID)
      -> std::variant<IPluginInstaller::EInstallResult, std::shared_ptr<IFileTree>, std::tuple<IPluginInstaller::EInstallResult, std::shared_ptr<IFileTree>, QString, int>> {
        auto result = p->install(modName, tree, version, nexusID);
        return std::make_tuple(result, tree, version, nexusID);
      })
    .def("_parentWidget", &IPluginInstallerSimpleWrapper::parentWidget, bpy::return_value_policy<bpy::return_by_value>())
    .def("_manager", &IPluginInstallerSimpleWrapper::manager, bpy::return_value_policy<bpy::reference_existing_object>())
    ;

  bpy::class_<IPluginInstallerCustomWrapper, boost::noncopyable>("IPluginInstallerCustom")
    // Needs to add both otherwize boost does not understanda:    
    .def("isArchiveSupported", &IPluginInstaller::isArchiveSupported)
    .def("isArchiveSupported", &IPluginInstallerCustom::isArchiveSupported)
    .def("supportedExtensions", &IPluginInstallerCustom::supportedExtensions)
    .def("install", &IPluginInstallerCustom::install)
    .def("_parentWidget", &IPluginInstallerSimpleWrapper::parentWidget, bpy::return_value_policy<bpy::return_by_value>())
    .def("_manager", &IPluginInstallerCustomWrapper::manager, bpy::return_value_policy<bpy::reference_existing_object>())
    ;

  bpy::class_<IPluginModPageWrapper, boost::noncopyable>("IPluginModPage")
    .def("displayName", bpy::pure_virtual(&IPluginModPage::displayName))
    .def("icon", bpy::pure_virtual(&IPluginModPage::icon))
    .def("pageURL", bpy::pure_virtual(&IPluginModPage::pageURL))
    .def("useIntegratedBrowser", bpy::pure_virtual(&IPluginModPage::useIntegratedBrowser))
    .def("handlesDownload", bpy::pure_virtual(&IPluginModPage::handlesDownload))
    .def("setParentWidget", &IPluginModPage::setParentWidget, &IPluginModPageWrapper::setParentWidget_Default)
    .def("_parentWidget", &IPluginModPageWrapper::parentWidget, bpy::return_value_policy<bpy::return_by_value>())
    ;

  bpy::class_<IPluginPreviewWrapper, bpy::bases<IPlugin>, boost::noncopyable>("IPluginPreview")
    .def("supportedExtensions", bpy::pure_virtual(&IPluginPreview::supportedExtensions))
    .def("genFilePreview", bpy::pure_virtual(&IPluginPreview::genFilePreview), bpy::return_value_policy<bpy::return_by_value>())
    ;

  bpy::class_<IPluginToolWrapper, bpy::bases<IPlugin>, boost::noncopyable>("IPluginTool")
    .def("displayName", bpy::pure_virtual(&IPluginTool::displayName))
    .def("tooltip", bpy::pure_virtual(&IPluginTool::tooltip))
    .def("icon", bpy::pure_virtual(&IPluginTool::icon))
    .def("display", bpy::pure_virtual(&IPluginTool::display))
    .def("setParentWidget", &IPluginTool::setParentWidget, &IPluginToolWrapper::setParentWidget_Default)
    .def("_parentWidget", &IPluginToolWrapper::parentWidget, bpy::return_value_policy<bpy::return_by_value>())
    ;

  registerGameFeaturesPythonConverters();

  bpy::def("getFileVersion", &MOBase::getFileVersion);
  bpy::def("getProductVersion", &MOBase::getProductVersion);
  bpy::def("getIconForExecutable", &MOBase::iconForExecutable);
}

/**
 *
 */
class PythonRunner : public IPythonRunner
{

public:
  PythonRunner(const MOBase::IOrganizer* moInfo);
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
  const MOBase::IOrganizer* m_MOInfo;
  wchar_t* m_PythonHome;
};

IPythonRunner* CreatePythonRunner(MOBase::IOrganizer* moInfo, const QString& pythonDir)
{
  PythonRunner* result = new PythonRunner(moInfo);
  if (result->initPython(pythonDir)) {
    return result;
  }
  else {
    delete result;
    return nullptr;
  }
}

PythonRunner::PythonRunner(const MOBase::IOrganizer *moInfo)
  : m_MOInfo(moInfo)
{
  m_PythonHome = new wchar_t[MAX_PATH + 1];
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

    bpy::object mainModule = bpy::import("__main__");
    bpy::object mainNamespace = mainModule.attr("__dict__");
    mainNamespace["sys"] = bpy::import("sys");
    mainNamespace["moprivate"] = bpy::import("moprivate");
    mainNamespace["mobase"] = bpy::import("mobase");
    bpy::import("site");
    bpy::exec("sys.stdout = moprivate.PrintWrapper()\n"
              "sys.stderr = moprivate.ErrWrapper.instance()\n"
              "sys.excepthook = lambda x, y, z: sys.__excepthook__(x, y, z)\n",
                        mainNamespace);

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
    QCoreApplication::applicationDirPath() + "/pythoncore.zip",
    QCoreApplication::applicationDirPath() + "/pythoncore",
    m_MOInfo->pluginDataPath()
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
    GILock lock;

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

