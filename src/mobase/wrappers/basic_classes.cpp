#include "wrappers.h"

#include "../pybind11_all.h"

#include <fmt/format.h>
#include <fmt/xchar.h>

#include <executableinfo.h>
#include <filemapping.h>
#include <guessedvalue.h>
#include <idownloadmanager.h>
#include <iinstallationmanager.h>
#include <imodinterface.h>
#include <imodrepositorybridge.h>
#include <imoinfo.h>
#include <iplugin.h>
#include <iplugindiagnose.h>
#include <iplugingame.h>
#include <ipluginlist.h>
#include <pluginsetting.h>
#include <versioninfo.h>

#include "../deprecation.h"
#include "pyfiletree.h"

using namespace MOBase;

namespace mo2::python {

    namespace py = pybind11;

    using namespace pybind11::literals;

    void add_versioninfo_classes(py::module_ m)
    {
        py::enum_<MOBase::VersionInfo::ReleaseType>(m, "ReleaseType")
            .value("final", MOBase::VersionInfo::RELEASE_FINAL)
            .value("candidate", MOBase::VersionInfo::RELEASE_CANDIDATE)
            .value("beta", MOBase::VersionInfo::RELEASE_BETA)
            .value("alpha", MOBase::VersionInfo::RELEASE_ALPHA)
            .value("prealpha", MOBase::VersionInfo::RELEASE_PREALPHA)

            .value("FINAL", MOBase::VersionInfo::RELEASE_FINAL)
            .value("CANDIDATE", MOBase::VersionInfo::RELEASE_CANDIDATE)
            .value("BETA", MOBase::VersionInfo::RELEASE_BETA)
            .value("ALPHA", MOBase::VersionInfo::RELEASE_ALPHA)
            .value("PRE_ALPHA", MOBase::VersionInfo::RELEASE_PREALPHA);

        py::enum_<MOBase::VersionInfo::VersionScheme>(m, "VersionScheme")
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
            .value("LITERAL", MOBase::VersionInfo::SCHEME_LITERAL);

        py::class_<VersionInfo>(m, "VersionInfo")
            .def(py::init<QString, VersionInfo::VersionScheme>(), "value"_a,
                 "scheme"_a = VersionInfo::SCHEME_DISCOVER)
            // note: order of the two init<> below is important because
            // ReleaseType is a simple enum with an implicit int conversion.
            .def(py::init<int, int, int, int, VersionInfo::ReleaseType>(), "major"_a,
                 "minor"_a, "subminor"_a, "subsubminor"_a,
                 "release_type"_a = VersionInfo::RELEASE_FINAL)
            .def(py::init<int, int, int, VersionInfo::ReleaseType>(), "major"_a,
                 "minor"_a, "subminor"_a, "release_type"_a = VersionInfo::RELEASE_FINAL)
            .def("clear", &VersionInfo::clear)
            .def("parse", &VersionInfo::parse, "value"_a,
                 "scheme"_a = VersionInfo::SCHEME_DISCOVER, "is_manual"_a = false)
            .def("canonicalString", &VersionInfo::canonicalString)
            .def("displayString", &VersionInfo::displayString, "forced_segments"_a = 2)
            .def("isValid", &VersionInfo::isValid)
            .def("scheme", &VersionInfo::scheme)
            .def("__str__", &VersionInfo::canonicalString)
            .def(py::self < py::self)
            .def(py::self > py::self)
            .def(py::self <= py::self)
            .def(py::self >= py::self)
            .def(py::self != py::self)
            .def(py::self == py::self);
    }

    void add_executable_classes(py::module_ m)
    {
        py::class_<ExecutableInfo>(m, "ExecutableInfo")
            .def(py::init<const QString&, const QFileInfo&>(), "title"_a, "binary"_a)
            .def("withArgument", &ExecutableInfo::withArgument, "argument"_a)
            .def("withWorkingDirectory", &ExecutableInfo::withWorkingDirectory,
                 "directory"_a)
            .def("withSteamAppId", &ExecutableInfo::withSteamAppId, "app_id"_a)
            .def("asCustom", &ExecutableInfo::asCustom)
            .def("isValid", &ExecutableInfo::isValid)
            .def("title", &ExecutableInfo::title)
            .def("binary", &ExecutableInfo::binary)
            .def("arguments", &ExecutableInfo::arguments)
            .def("workingDirectory", &ExecutableInfo::workingDirectory)
            .def("steamAppID", &ExecutableInfo::steamAppID)
            .def("isCustom", &ExecutableInfo::isCustom);

        py::class_<ExecutableForcedLoadSetting>(m, "ExecutableForcedLoadSetting")
            .def(py::init<const QString&, const QString&>(), "process"_a, "library"_a)
            .def("withForced", &ExecutableForcedLoadSetting::withForced, "forced"_a)
            .def("withEnabled", &ExecutableForcedLoadSetting::withEnabled, "enabled"_a)
            .def("enabled", &ExecutableForcedLoadSetting::enabled)
            .def("forced", &ExecutableForcedLoadSetting::forced)
            .def("library", &ExecutableForcedLoadSetting::library)
            .def("process", &ExecutableForcedLoadSetting::process);
    }

    void add_modinterface_classes(py::module_ m)
    {
        py::enum_<EndorsedState>(m, "EndorsedState")
            .value("ENDORSED_FALSE", EndorsedState::ENDORSED_FALSE)
            .value("ENDORSED_TRUE", EndorsedState::ENDORSED_TRUE)
            .value("ENDORSED_UNKNOWN", EndorsedState::ENDORSED_UNKNOWN)
            .value("ENDORSED_NEVER", EndorsedState::ENDORSED_NEVER);

        py::enum_<TrackedState>(m, "TrackedState")
            .value("TRACKED_FALSE", TrackedState::TRACKED_FALSE)
            .value("TRACKED_TRUE", TrackedState::TRACKED_TRUE)
            .value("TRACKED_UNKNOWN", TrackedState::TRACKED_UNKNOWN);

        py::class_<IModInterface>(m, "IModInterface")
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
            .def("isOverwrite", &IModInterface::isOverwrite)
            .def("isBackup", &IModInterface::isBackup)
            .def("isSeparator", &IModInterface::isSeparator)
            .def("isForeign", &IModInterface::isForeign)

            .def("setVersion", &IModInterface::setVersion, "version"_a)
            .def("setNewestVersion", &IModInterface::setNewestVersion, "version"_a)
            .def("setIsEndorsed", &IModInterface::setIsEndorsed, "endorsed"_a)
            .def("setNexusID", &IModInterface::setNexusID, "nexus_id"_a)
            .def("addNexusCategory", &IModInterface::addNexusCategory, "category_id"_a)
            .def("addCategory", &IModInterface::addCategory, "name"_a)
            .def("removeCategory", &IModInterface::removeCategory, "name"_a)
            .def("setGameName", &IModInterface::setGameName, "name"_a)
            .def("setUrl", &IModInterface::setUrl, "url"_a)
            .def("pluginSetting", &IModInterface::pluginSetting, "plugin_name"_a,
                 "key"_a, "default"_a = QVariant())
            .def("pluginSettings", &IModInterface::pluginSettings, "plugin_name"_a)
            .def("setPluginSetting", &IModInterface::setPluginSetting, "plugin_name"_a,
                 "key"_a, "value"_a)
            .def("clearPluginSettings", &IModInterface::clearPluginSettings,
                 "plugin_name"_a);
    }

    void add_modrepository_classes(py::module_ m)
    {
        py::class_<IModRepositoryBridge> iModRepositoryBridge(m,
                                                              "IModRepositoryBridge");
        iModRepositoryBridge
            .def("requestDescription", &IModRepositoryBridge::requestDescription,
                 "game_name"_a, "mod_id"_a, "user_data"_a)
            .def("requestFiles", &IModRepositoryBridge::requestFiles, "game_name"_a,
                 "mod_id"_a, "user_data"_a)
            .def("requestFileInfo", &IModRepositoryBridge::requestFileInfo,
                 "game_name"_a, "mod_id"_a, "file_id"_a, "user_data"_a)
            .def("requestDownloadURL", &IModRepositoryBridge::requestDownloadURL,
                 "game_name"_a, "mod_id"_a, "file_id"_a, "user_data"_a)
            .def("requestToggleEndorsement",
                 &IModRepositoryBridge::requestToggleEndorsement, "game_name"_a,
                 "mod_id"_a, "mod_version"_a, "endorse"_a, "user_data"_a);

        py::qt::add_qt_delegate<QObject>(iModRepositoryBridge, "_object");

        py::class_<ModRepositoryFileInfo>(m, "ModRepositoryFileInfo")
            .def(py::init<const ModRepositoryFileInfo&>(), "other"_a)
            .def(py::init<QString, int, int>(), "game_name"_a = "", "mod_id"_a = 0,
                 "file_id"_a = 0)
            .def("__str__", &ModRepositoryFileInfo::toString)
            .def_static("createFromJson", &ModRepositoryFileInfo::createFromJson,
                        "data"_a)
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
            .def_readwrite("userData", &ModRepositoryFileInfo::userData);
    }

    void add_guessedstring_classes(py::module_ m)
    {
        py::enum_<MOBase::EGuessQuality>(m, "GuessQuality")
            .value("INVALID", MOBase::GUESS_INVALID)
            .value("FALLBACK", MOBase::GUESS_FALLBACK)
            .value("GOOD", MOBase::GUESS_GOOD)
            .value("META", MOBase::GUESS_META)
            .value("PRESET", MOBase::GUESS_PRESET)
            .value("USER", MOBase::GUESS_USER);

        py::class_<MOBase::GuessedValue<QString>>(m, "GuessedString")
            .def(py::init<>())
            .def(py::init<QString const&, EGuessQuality>(), "value"_a,
                 "quality"_a = EGuessQuality::GUESS_USER)
            .def("update",
                 py::overload_cast<const QString&>(&GuessedValue<QString>::update),
                 "value"_a)
            .def("update",
                 py::overload_cast<const QString&, EGuessQuality>(
                     &GuessedValue<QString>::update),
                 "value"_a, "quality"_a)

            // Methods to simulate the assignment operator:
            .def("reset",
                 [](GuessedValue<QString>* gv) {
                     *gv = GuessedValue<QString>();
                     return gv;
                 })
            .def(
                "reset",
                [](GuessedValue<QString>* gv, const QString& value, EGuessQuality eq) {
                    *gv = GuessedValue<QString>(value, eq);
                    return gv;
                },
                "value"_a, "quality"_a)
            .def(
                "reset",
                [](GuessedValue<QString>* gv, const GuessedValue<QString>& other) {
                    *gv = other;
                    return gv;
                },
                "other"_a)

            // use an intermediate lambda because we cannot have a function with a
            // non-const reference in Python - in Python, the function should returned a
            // bool or the modified value
            .def(
                "setFilter",
                [](GuessedValue<QString>* gv,
                   std::function<std::variant<QString, bool>(QString const&)> fn) {
                    gv->setFilter([fn](QString& s) {
                        auto ret = fn(s);
                        return std::visit(
                            [&s](auto v) {
                                if constexpr (std::is_same_v<decltype(v), QString>) {
                                    s = v;
                                    return true;
                                }
                                else if constexpr (std::is_same_v<decltype(v), bool>) {
                                    return v;
                                }
                            },
                            ret);
                    });
                },
                "filter"_a)

            // this makes a copy in python but it more practical than
            // exposing an iterator
            .def("variants", &GuessedValue<QString>::variants)
            .def("__str__", &MOBase::GuessedValue<QString>::operator const QString&);

        // implicit conversion from QString - this allows passing Python string to
        // function expecting GuessedValue<QString>
        py::implicitly_convertible<QString, GuessedValue<QString>>();
    }

    void add_ipluginlist_classes(py::module_ m)
    {
        py::enum_<IPluginList::PluginState>(m, "PluginState")
            .value("missing", IPluginList::STATE_MISSING)
            .value("inactive", IPluginList::STATE_INACTIVE)
            .value("active", IPluginList::STATE_ACTIVE)

            .value("MISSING", IPluginList::STATE_MISSING)
            .value("INACTIVE", IPluginList::STATE_INACTIVE)
            .value("ACTIVE", IPluginList::STATE_ACTIVE);

        py::class_<IPluginList>(m, "IPluginList")
            .def("state", &MOBase::IPluginList::state, "name"_a)
            .def("priority", &MOBase::IPluginList::priority, "name"_a)
            .def("setPriority", &MOBase::IPluginList::setPriority, "name"_a,
                 "priority"_a)
            .def("loadOrder", &MOBase::IPluginList::loadOrder, "name"_a)
            .def("isMaster", &MOBase::IPluginList::isMaster, "name"_a)
            .def("masters", &MOBase::IPluginList::masters, "name"_a)
            .def("origin", &MOBase::IPluginList::origin, "name"_a)
            .def("onRefreshed", &MOBase::IPluginList::onRefreshed, "callback"_a)
            .def("onPluginMoved", &MOBase::IPluginList::onPluginMoved, "callback"_a)

            // Kept but deprecated for backward compatibility:
            .def(
                "onPluginStateChanged",
                [](IPluginList* modList,
                   const std::function<void(const QString&, IPluginList::PluginStates)>&
                       fn) {
                    mo2::python::show_deprecation_warning(
                        "onPluginStateChanged",
                        "onPluginStateChanged(Callable[[str, "
                        "IPluginList.PluginStates], None]) is deprecated, "
                        "use onPluginStateChanged(Callable[[Dict[str, "
                        "IPluginList.PluginStates], None]) instead.");
                    return modList->onPluginStateChanged([fn](auto const& map) {
                        for (const auto& entry : map) {
                            fn(entry.first, entry.second);
                        }
                    });
                },
                "callback"_a)
            .def("onPluginStateChanged", &MOBase::IPluginList::onPluginStateChanged,
                 "callback"_a)
            .def("pluginNames", &MOBase::IPluginList::pluginNames)
            .def("setState", &MOBase::IPluginList::setState, ("name"_a, "state"))
            .def("setLoadOrder", &MOBase::IPluginList::setLoadOrder, "loadorder"_a);
    }

    void add_imodlist_classes(py::module_ m)
    {
        py::enum_<IModList::ModState>(m, "ModState")
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
            .value("ALTERNATE", IModList::STATE_ALTERNATE);

        py::class_<IModList>(m, "IModList")
            .def("displayName", &MOBase::IModList::displayName, "name"_a)
            .def("allMods", &MOBase::IModList::allMods)
            .def("allModsByProfilePriority",
                 &MOBase::IModList::allModsByProfilePriority,
                 "profile"_a = static_cast<IProfile*>(nullptr))
            .def("getMod", &MOBase::IModList::getMod,
                 py::return_value_policy::reference, "name"_a)
            .def("removeMod", &MOBase::IModList::removeMod, "mod"_a)
            .def("renameMod", &MOBase::IModList::renameMod,
                 py::return_value_policy::reference, "mod"_a, "name"_a)

            .def("state", &MOBase::IModList::state, "name"_a)
            .def("setActive",
                 py::overload_cast<QStringList const&, bool>(
                     &MOBase::IModList::setActive),
                 "names"_a, "active"_a)
            .def("setActive",
                 py::overload_cast<QString const&, bool>(&MOBase::IModList::setActive),
                 "name"_a, "active"_a)
            .def("priority", &MOBase::IModList::priority, "name"_a)
            .def("setPriority", &MOBase::IModList::setPriority, "name"_a, "priority"_a)

            // kept but deprecated for backward compatibility
            .def(
                "onModStateChanged",
                [](IModList* modList,
                   const std::function<void(const QString&, IModList::ModStates)>& fn) {
                    mo2::python::show_deprecation_warning(
                        "onModStateChanged",
                        "onModStateChanged(Callable[[str, IModList.ModStates], None]) "
                        "is deprecated, "
                        "use onModStateChanged(Callable[[Dict[str, "
                        "IModList.ModStates], None]) instead.");
                    return modList->onModStateChanged([fn](auto const& map) {
                        for (const auto& entry : map) {
                            fn(entry.first, entry.second);
                        }
                    });
                },
                "callback"_a)

            .def("onModInstalled", &MOBase::IModList::onModInstalled, "callback"_a)
            .def("onModRemoved", &MOBase::IModList::onModRemoved, "callback"_a)
            .def("onModStateChanged", &MOBase::IModList::onModStateChanged,
                 "callback"_a)
            .def("onModMoved", &MOBase::IModList::onModMoved, "callback"_a);
    }

    void add_iorganizer_classes(py::module_ m)
    {
        py::class_<IOrganizer::FileInfo>(m, "FileInfo")
            .def(py::init<>())
            .def_readwrite("filePath", &IOrganizer::FileInfo::filePath)
            .def_readwrite("archive", &IOrganizer::FileInfo::archive)
            .def_readwrite("origins", &IOrganizer::FileInfo::origins);

        py::class_<IOrganizer>(m, "IOrganizer")
            .def("createNexusBridge", &IOrganizer::createNexusBridge,
                 py::return_value_policy::reference)
            .def("profileName", &IOrganizer::profileName)
            .def("profilePath", &IOrganizer::profilePath)
            .def("downloadsPath", &IOrganizer::downloadsPath)
            .def("overwritePath", &IOrganizer::overwritePath)
            .def("basePath", &IOrganizer::basePath)
            .def("modsPath", &IOrganizer::modsPath)
            .def("appVersion", &IOrganizer::appVersion)
            .def("createMod", &IOrganizer::createMod,
                 py::return_value_policy::reference, "name"_a)
            .def("getGame", &IOrganizer::getGame, py::return_value_policy::reference,
                 "name"_a)
            .def("modDataChanged", &IOrganizer::modDataChanged, "mod"_a)
            .def("isPluginEnabled",
                 py::overload_cast<IPlugin*>(&IOrganizer::isPluginEnabled, py::const_),
                 "plugin"_a)
            .def("isPluginEnabled",
                 py::overload_cast<QString const&>(&IOrganizer::isPluginEnabled,
                                                   py::const_),
                 "plugin"_a)
            .def("pluginSetting", &IOrganizer::pluginSetting, "plugin_name"_a, "key"_a)
            .def("setPluginSetting", &IOrganizer::setPluginSetting, "plugin_name"_a,
                 "key"_a, "value"_a)
            .def("persistent", &IOrganizer::persistent, "plugin_name"_a, "key"_a,
                 "default"_a = QVariant())
            .def("setPersistent", &IOrganizer::setPersistent, "plugin_name"_a, "key"_a,
                 "value"_a, "sync"_a = true)
            .def("pluginDataPath", &IOrganizer::pluginDataPath)
            .def("installMod", &IOrganizer::installMod,
                 py::return_value_policy::reference, "filename"_a,
                 "name_suggestion"_a = "")
            .def("resolvePath", &IOrganizer::resolvePath, "filename"_a)
            .def("listDirectories", &IOrganizer::listDirectories, "directory"_a)

            // "provide multiple overloads of findFiles
            .def(
                "findFiles",
                [](const IOrganizer* o, QString const& p,
                   std::function<bool(QString const&)> const& f) {
                    return o->findFiles(p, f);
                },
                "path"_a, "filter"_a)

            // in C++, it is possible to create a QStringList implicitly from
            // a single QString, in Python is not possible with the current
            // converters in python (and I do not think it is a good idea to
            // have it everywhere), but here it is nice to be able to
            // pass a single string, so we add an extra overload
            //
            // important: the order matters, because a Python string can be
            // converted to a QStringList since it is a sequence of
            // single-character strings:
            .def(
                "findFiles",
                [](const IOrganizer* o, QString const& p, const QStringList& gf) {
                    return o->findFiles(p, gf);
                },
                "path"_a, "patterns"_a)
            .def(
                "findFiles",
                [](const IOrganizer* o, QString const& p, const QString& f) {
                    return o->findFiles(p, QStringList{f});
                },
                "path"_a, "pattern"_a)

            .def("getFileOrigins", &IOrganizer::getFileOrigins, "filename"_a)
            .def("findFileInfos", &IOrganizer::findFileInfos, "path"_a, "filter"_a)

            .def("virtualFileTree", &IOrganizer::virtualFileTree)

            .def("downloadManager", &IOrganizer::downloadManager,
                 py::return_value_policy::reference)
            .def("pluginList", &IOrganizer::pluginList,
                 py::return_value_policy::reference)
            .def("modList", &IOrganizer::modList, py::return_value_policy::reference)
            .def("profile", &IOrganizer::profile, py::return_value_policy::reference)

            // custom implementation for startApplication and
            // waitForApplication because 1) HANDLE (= void*) is not properly
            // converted from/to python, and 2) we need to convert the by-ptr
            // argument to a return-tuple for waitForApplication
            .def(
                "startApplication",
                [](IOrganizer* o, const QString& executable, const QStringList& args,
                   const QString& cwd, const QString& profile,
                   const QString& forcedCustomOverwrite,
                   bool ignoreCustomOverwrite) -> std::uintptr_t {
                    return (std::uintptr_t)o->startApplication(
                        executable, args, cwd, profile, forcedCustomOverwrite,
                        ignoreCustomOverwrite);
                },
                "executable"_a, "args"_a = QStringList(), "cwd"_a = "",
                "profile"_a = "", "forcedCustomOverwrite"_a = "",
                "ignoreCustomOverwrite"_a = false)
            .def(
                "waitForApplication",
                [](IOrganizer* o, std::uintptr_t handle, bool refresh) {
                    DWORD returnCode;
                    bool result =
                        o->waitForApplication((HANDLE)handle, refresh, &returnCode);
                    return std::make_tuple(result, returnCode);
                },
                "handle"_a, "refresh"_a = true)

            .def("refresh", &IOrganizer::refresh, "save_changes"_a = true)
            .def("managedGame", &IOrganizer::managedGame,
                 py::return_value_policy::reference)

            .def("onAboutToRun", &IOrganizer::onAboutToRun, "callback"_a)
            .def("onFinishedRun", &IOrganizer::onFinishedRun, "callback"_a)
            .def("onUserInterfaceInitialized", &IOrganizer::onUserInterfaceInitialized,
                 "callback"_a)
            .def("onProfileCreated", &IOrganizer::onProfileCreated, "callback"_a)
            .def("onProfileRenamed", &IOrganizer::onProfileRenamed, "callback"_a)
            .def("onProfileRemoved", &IOrganizer::onProfileRemoved, "callback"_a)
            .def("onProfileChanged", &IOrganizer::onProfileChanged, "callback"_a)

            .def("onPluginSettingChanged", &IOrganizer::onPluginSettingChanged,
                 "callback"_a)
            .def(
                "onPluginEnabled",
                [](IOrganizer* o, std::function<void(const IPlugin*)> const& func) {
                    o->onPluginEnabled(func);
                },
                "callback"_a)
            .def(
                "onPluginEnabled",
                [](IOrganizer* o, QString const& name,
                   std::function<void()> const& func) {
                    o->onPluginEnabled(name, func);
                },
                "name"_a, "callback"_a)
            .def(
                "onPluginDisabled",
                [](IOrganizer* o, std::function<void(const IPlugin*)> const& func) {
                    o->onPluginDisabled(func);
                },
                "callback"_a)
            .def(
                "onPluginDisabled",
                [](IOrganizer* o, QString const& name,
                   std::function<void()> const& func) {
                    o->onPluginDisabled(name, func);
                },
                "name"_a, "callback"_a)

            // DEPRECATED:
            .def(
                "getMod",
                [](IOrganizer* o, QString const& name) {
                    mo2::python::show_deprecation_warning(
                        "getMod", "IOrganizer::getMod(str) is deprecated, use "
                                  "IModList::getMod(str) instead.");
                    return o->modList()->getMod(name);
                },
                py::return_value_policy::reference, "name"_a)
            .def(
                "removeMod",
                [](IOrganizer* o, IModInterface* mod) {
                    mo2::python::show_deprecation_warning(
                        "removeMod",
                        "IOrganizer::removeMod(IModInterface) is deprecated, use "
                        "IModList::removeMod(IModInterface) instead.");
                    return o->modList()->removeMod(mod);
                },
                "mod"_a)
            .def("modsSortedByProfilePriority",
                 [](IOrganizer* o) {
                     mo2::python::show_deprecation_warning(
                         "modsSortedByProfilePriority",
                         "IOrganizer::modsSortedByProfilePriority() is deprecated, use "
                         "IModList::allModsByProfilePriority() instead.");
                     return o->modList()->allModsByProfilePriority();
                 })
            .def(
                "refreshModList",
                [](IOrganizer* o, bool s) {
                    mo2::python::show_deprecation_warning(
                        "refreshModList",
                        "IOrganizer::refreshModList(bool) is deprecated, use "
                        "IOrganizer::refresh(bool) instead.");
                    o->refresh(s);
                },
                "save_changes"_a = true)
            .def(
                "onModInstalled",
                [](IOrganizer* organizer,
                   const std::function<void(QString const&)>& func) {
                    mo2::python::show_deprecation_warning(
                        "onModInstalled",
                        "IOrganizer::onModInstalled(Callable[[str], None]) is "
                        "deprecated, "
                        "use IModList::onModInstalled(Callable[[IModInterface], None]) "
                        "instead.");
                    return organizer->modList()->onModInstalled(
                        [func](MOBase::IModInterface* m) {
                            func(m->name());
                        });
                    ;
                },
                "callback"_a)

            .def_static("getPluginDataPath", &IOrganizer::getPluginDataPath);
    }

    void add_idownload_manager_classes(py::module_ m)
    {
        py::class_<IDownloadManager>(m, "IDownloadManager")
            .def("startDownloadURLs", &IDownloadManager::startDownloadURLs, "urls"_a)
            .def("startDownloadNexusFile", &IDownloadManager::startDownloadNexusFile,
                 "mod_id"_a, "file_id"_a)
            .def("downloadPath", &IDownloadManager::downloadPath, "id"_a)
            .def("onDownloadComplete", &IDownloadManager::onDownloadComplete,
                 "callback"_a)
            .def("onDownloadPaused", &IDownloadManager::onDownloadPaused, "callback"_a)
            .def("onDownloadFailed", &IDownloadManager::onDownloadFailed, "callback"_a)
            .def("onDownloadRemoved", &IDownloadManager::onDownloadRemoved,
                 "callback"_a);
    }

    void add_iinstallation_manager_classes(py::module_ m)
    {
        py::class_<IInstallationManager>(m, "IInstallationManager")
            .def("getSupportedExtensions",
                 &IInstallationManager::getSupportedExtensions)
            .def("extractFile", &IInstallationManager::extractFile, "entry"_a,
                 "silent"_a = false)
            .def("extractFiles", &IInstallationManager::extractFiles, "entries"_a,
                 "silent"_a = false)
            .def("createFile", &IInstallationManager::createFile, "entry"_a)

            // return a tuple to get back the mod name and the mod ID
            .def(
                "installArchive",
                [](IInstallationManager* m, GuessedValue<QString> modName,
                   QString archive, int modId) {
                    auto result = m->installArchive(modName, archive, modId);
                    return std::make_tuple(result, static_cast<QString>(modName),
                                           modId);
                },
                "mod_name"_a, "archive"_a, "mod_id"_a = 0);
    }

    void add_basic_bindings(py::module_ m)
    {
        add_versioninfo_classes(m);
        add_executable_classes(m);
        add_guessedstring_classes(m);

        add_ifiletree_bindings(m);

        add_modinterface_classes(m);
        add_modrepository_classes(m);

        py::class_<PluginSetting>(m, "PluginSetting")
            .def(py::init<const QString&, const QString&, const QVariant&>(), "key"_a,
                 "description"_a, "default_value"_a)
            .def_readwrite("key", &PluginSetting::key)
            .def_readwrite("description", &PluginSetting::description)
            .def_readwrite("default_value", &PluginSetting::defaultValue);

        py::class_<PluginRequirementFactory>(m, "PluginRequirementFactory")
            // pluginDependency
            .def_static("pluginDependency",
                        py::overload_cast<QStringList const&>(
                            &PluginRequirementFactory::pluginDependency),
                        "plugins"_a)
            .def_static("pluginDependency",
                        py::overload_cast<QString const&>(
                            &PluginRequirementFactory::pluginDependency),
                        "plugins"_a)
            // gameDependency
            .def_static("gameDependency",
                        py::overload_cast<QStringList const&>(
                            &PluginRequirementFactory::gameDependency),
                        "plugins"_a)
            .def_static("gameDependency",
                        py::overload_cast<QString const&>(
                            &PluginRequirementFactory::gameDependency),
                        "plugins"_a)
            // diagnose
            .def_static("diagnose", &PluginRequirementFactory::diagnose, "diagnose"_a)
            // basic
            .def_static("basic", &PluginRequirementFactory::basic, "checker"_a,
                        "description"_a);

        py::class_<Mapping>(m, "Mapping")
            .def(py::init<>())
            .def(py::init([](QString src, QString dst, bool dir, bool crt) -> Mapping {
                     return {src, dst, dir, crt};
                 }),
                 "source"_a, "destination"_a, "is_directory"_a,
                 "create_target"_a = false)
            .def_readwrite("source", &Mapping::source)
            .def_readwrite("destination", &Mapping::destination)
            .def_readwrite("isDirectory", &Mapping::isDirectory)
            .def_readwrite("createTarget", &Mapping::createTarget)
            .def("__str__", [](Mapping const& m) {
                return fmt::format(L"Mapping({}, {}, {}, {})", m.source.toStdWString(),
                                   m.destination.toStdWString(), m.isDirectory,
                                   m.createTarget);
            });

        // must be done BEFORE imodlist because there is a default argument to a
        // IProfile* in the modlist class
        py::class_<IProfile>(m, "IProfile")
            .def("name", &IProfile::name)
            .def("absolutePath", &IProfile::absolutePath)
            .def("localSavesEnabled", &IProfile::localSavesEnabled)
            .def("localSettingsEnabled", &IProfile::localSettingsEnabled)
            .def("invalidationActive",
                 [](const IProfile* p) {
                     bool supported;
                     bool active = p->invalidationActive(&supported);
                     return py::make_tuple(active, supported);
                 })
            .def("absoluteIniFilePath", &IProfile::absoluteIniFilePath, "inifile"_a);

        add_ipluginlist_classes(m);
        add_imodlist_classes(m);
        add_idownload_manager_classes(m);
        add_iinstallation_manager_classes(m);
        add_iorganizer_classes(m);
    }

}  // namespace mo2::python
