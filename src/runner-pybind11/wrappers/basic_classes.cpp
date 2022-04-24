#include "wrappers.h"

#include <pybind11/functional.h>
#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "../pybind11_qt/pybind11_qt.h"

#include <executableinfo.h>
#include <filemapping.h>
#include <guessedvalue.h>
#include <imodinterface.h>
#include <imoinfo.h>
#include <ipluginlist.h>
#include <pluginsetting.h>
#include <versioninfo.h>

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
            .def(py::init<QString, VersionInfo::VersionScheme>(), py::arg("value"),
                 py::arg("scheme") = VersionInfo::SCHEME_DISCOVER)
            // note: order of the two init<> below is important because
            // ReleaseType is a simple enum with an implicit int conversion.
            .def(py::init<int, int, int, int, VersionInfo::ReleaseType>(),
                 py::arg("major"), py::arg("minor"), py::arg("subminor"),
                 py::arg("subsubminor"),
                 py::arg("release_type") = VersionInfo::RELEASE_FINAL)
            .def(py::init<int, int, int, VersionInfo::ReleaseType>(), py::arg("major"),
                 py::arg("minor"), py::arg("subminor"),
                 py::arg("release_type") = VersionInfo::RELEASE_FINAL)
            .def("clear", &VersionInfo::clear)
            .def("parse", &VersionInfo::parse, py::arg("value"),
                 py::arg("scheme")    = VersionInfo::SCHEME_DISCOVER,
                 py::arg("is_manual") = false)
            .def("canonicalString", &VersionInfo::canonicalString)
            .def("displayString", &VersionInfo::displayString,
                 py::arg("forced_segments") = 2)
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
            .def(py::init<const QString&, const QFileInfo&>(), py::arg("title"),
                 py::arg("binary"))
            .def("withArgument", &ExecutableInfo::withArgument, py::arg("argument"))
            .def("withWorkingDirectory", &ExecutableInfo::withWorkingDirectory,
                 py::arg("directory"))
            .def("withSteamAppId", &ExecutableInfo::withSteamAppId, py::arg("app_id"))
            .def("asCustom", &ExecutableInfo::asCustom)
            .def("isValid", &ExecutableInfo::isValid)
            .def("title", &ExecutableInfo::title)
            .def("binary", &ExecutableInfo::binary)
            .def("arguments", &ExecutableInfo::arguments)
            .def("workingDirectory", &ExecutableInfo::workingDirectory)
            .def("steamAppID", &ExecutableInfo::steamAppID)
            .def("isCustom", &ExecutableInfo::isCustom);

        py::class_<ExecutableForcedLoadSetting>(m, "ExecutableForcedLoadSetting")
            .def(py::init<const QString&, const QString&>(), py::arg("process"),
                 py::arg("library"))
            .def("withForced", &ExecutableForcedLoadSetting::withForced,
                 py::arg("forced"))
            .def("withEnabled", &ExecutableForcedLoadSetting::withEnabled,
                 py::arg("enabled"))
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

            .def("setVersion", &IModInterface::setVersion, py::arg("version"))
            .def("setNewestVersion", &IModInterface::setNewestVersion,
                 py::arg("version"))
            .def("setIsEndorsed", &IModInterface::setIsEndorsed, py::arg("endorsed"))
            .def("setNexusID", &IModInterface::setNexusID, py::arg("nexus_id"))
            .def("addNexusCategory", &IModInterface::addNexusCategory,
                 py::arg("category_id"))
            .def("addCategory", &IModInterface::addCategory, py::arg("name"))
            .def("removeCategory", &IModInterface::removeCategory, py::arg("name"))
            .def("setGameName", &IModInterface::setGameName, py::arg("name"))
            .def("setUrl", &IModInterface::setUrl, py::arg("url"))
            .def("pluginSetting", &IModInterface::pluginSetting, py::arg("plugin_name"),
                 py::arg("key"), py::arg("default") = QVariant())
            .def("pluginSettings", &IModInterface::pluginSettings,
                 py::arg("plugin_name"))
            .def("setPluginSetting", &IModInterface::setPluginSetting,
                 py::arg("plugin_name"), py::arg("key"), py::arg("value"))
            .def("clearPluginSettings", &IModInterface::clearPluginSettings,
                 py::arg("plugin_name"));
    }

    void add_modrepository_classes(py::module_ m)
    {
        // py::class_<IModRepositoryBridge,
        // boost::noncopyable>("IModRepositoryBridge", py::no_init)
        //     .def("requestDescription",
        //     &IModRepositoryBridge::requestDescription, (py::arg("game_name"),
        //     "mod_id", "user_data")) .def("requestFiles",
        //     &IModRepositoryBridge::requestFiles, (py::arg("game_name"),
        //     "mod_id", "user_data")) .def("requestFileInfo",
        //     &IModRepositoryBridge::requestFileInfo, (py::arg("game_name"),
        //     "mod_id", "file_id", "user_data")) .def("requestDownloadURL",
        //     &IModRepositoryBridge::requestDownloadURL, (py::arg("game_name"),
        //     "mod_id", "file_id", "user_data"))
        //     .def("requestToggleEndorsement",
        //     &IModRepositoryBridge::requestToggleEndorsement,
        //     (py::arg("game_name"), "mod_id", "mod_version", "endorse",
        //     "user_data"))

        //     Q_DELEGATE(IModRepositoryBridge, QObject, "_object")
        //     ;

        // py::class_<ModRepositoryFileInfo>("ModRepositoryFileInfo",
        // py::no_init)
        //     .def(py::init<const ModRepositoryFileInfo &>(py::arg("other")))
        //     .def(py::init<py::optional<QString, int,
        //     int>>((py::arg("game_name"), "mod_id", "file_id")))
        //     .def("__str__", &ModRepositoryFileInfo::toString)
        //     .def("createFromJson", &ModRepositoryFileInfo::createFromJson,
        //     py::arg("data")).staticmethod("createFromJson")
        //     .def_readwrite("name", &ModRepositoryFileInfo::name)
        //     .def_readwrite("uri", &ModRepositoryFileInfo::uri)
        //     .def_readwrite("description",
        //     &ModRepositoryFileInfo::description) .def_readwrite("version",
        //     &ModRepositoryFileInfo::version) .def_readwrite("newestVersion",
        //     &ModRepositoryFileInfo::newestVersion)
        //     .def_readwrite("categoryID", &ModRepositoryFileInfo::categoryID)
        //     .def_readwrite("modName", &ModRepositoryFileInfo::modName)
        //     .def_readwrite("gameName", &ModRepositoryFileInfo::gameName)
        //     .def_readwrite("modID", &ModRepositoryFileInfo::modID)
        //     .def_readwrite("fileID", &ModRepositoryFileInfo::fileID)
        //     .def_readwrite("fileSize", &ModRepositoryFileInfo::fileSize)
        //     .def_readwrite("fileName", &ModRepositoryFileInfo::fileName)
        //     .def_readwrite("fileCategory",
        //     &ModRepositoryFileInfo::fileCategory) .def_readwrite("fileTime",
        //     &ModRepositoryFileInfo::fileTime) .def_readwrite("repository",
        //     &ModRepositoryFileInfo::repository) .def_readwrite("userData",
        //     &ModRepositoryFileInfo::userData)
        //     ;
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

        // py::class_<MOBase::GuessedValue<QString>,
        // boost::noncopyable>("GuessedString")
        //     .def(py::init<>())
        //     .def(py::init<QString const&, EGuessQuality>((py::arg("value"),
        //     py::arg("quality") = EGuessQuality::GUESS_USER))) .def("update",
        //          static_cast<GuessedValue<QString>&
        //          (GuessedValue<QString>::*)(const
        //          QString&)>(&GuessedValue<QString>::update),
        //          py::return_self<>(), py::arg("value"))
        //     .def("update",
        //          static_cast<GuessedValue<QString>&
        //          (GuessedValue<QString>::*)(const QString&,
        //          EGuessQuality)>(&GuessedValue<QString>::update),
        //          py::return_self<>(), (py::arg("value"), "quality"))

        //     // Methods to simulate the assignment operator:
        //     .def("reset", +[](GuessedValue<QString>* gv) {
        //       *gv = GuessedValue<QString>(); }, py::return_self<>())
        //     .def("reset", +[](GuessedValue<QString>* gv, const QString&
        //     value, EGuessQuality eq) {
        //       *gv = GuessedValue<QString>(value, eq); }, py::return_self<>(),
        //       (py::arg("value"), "quality"))
        //     .def("reset", +[](GuessedValue<QString>* gv, const
        //     GuessedValue<QString>& other) {
        //       *gv = other; }, py::return_self<>(), py::arg("other"))

        //     // Use an intermediate lambda to avoid having to register the
        //     std::function conversion: .def("setFilter",
        //     +[](GuessedValue<QString>* gv,
        //     std::function<std::variant<QString, bool>(QString const&)> fn) {
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

        //     // This makes a copy in python but it more practical than
        //     exposing an iterator: .def("variants",
        //     &GuessedValue<QString>::variants,
        //     py::return_value_policy<py::copy_const_reference>())
        //     .def("__str__", &MOBase::GuessedValue<QString>::operator const
        //     QString&, py::return_value_policy<py::copy_const_reference>())
        //     ;
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

        // py::class_<IPluginList, boost::noncopyable>("IPluginList",
        // py::no_init)
        //     .def("state", &MOBase::IPluginList::state, py::arg("name"))
        //     .def("priority", &MOBase::IPluginList::priority, py::arg("name"))
        //     .def("setPriority", &MOBase::IPluginList::setPriority,
        //     (py::arg("name"), "priority")) .def("loadOrder",
        //     &MOBase::IPluginList::loadOrder, py::arg("name"))
        //     .def("isMaster", &MOBase::IPluginList::isMaster, py::arg("name"))
        //     .def("masters", &MOBase::IPluginList::masters, py::arg("name"))
        //     .def("origin", &MOBase::IPluginList::origin, py::arg("name"))
        //     .def("onRefreshed", &MOBase::IPluginList::onRefreshed,
        //     py::arg("callback")) .def("onPluginMoved",
        //     &MOBase::IPluginList::onPluginMoved, py::arg("callback"))

        //     // Kept but deprecated for backward compatibility:
        //     .def("onPluginStateChanged", +[](IPluginList* modList, const
        //     std::function<void(const QString&, IPluginList::PluginStates)>&
        //     fn) {
        //       utils::show_deprecation_warning("onPluginStateChanged",
        //         "onPluginStateChanged(Callable[[str,
        //         IPluginList.PluginStates], None]) is deprecated, " "use
        //         onPluginStateChanged(Callable[[Dict[str,
        //         IPluginList.PluginStates], None]) instead.");
        //       return modList->onPluginStateChanged([fn](auto const& map) {
        //         for (const auto& entry : map) {
        //           fn(entry.first, entry.second);
        //         }
        //         });
        //         }, py::arg("callback"))
        //     .def("onPluginStateChanged",
        //     &MOBase::IPluginList::onPluginStateChanged, py::arg("callback"))
        //     .def("pluginNames", &MOBase::IPluginList::pluginNames)
        //     .def("setState", &MOBase::IPluginList::setState,
        //     (py::arg("name"), "state")) .def("setLoadOrder",
        //     &MOBase::IPluginList::setLoadOrder, py::arg("loadorder"))
        //     ;
    }

    void add_imodlist_classes(py::module_ m)
    {
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
        //     .def("displayName", &MOBase::IModList::displayName,
        //     py::arg("name")) .def("allMods", &MOBase::IModList::allMods)
        //     .def("allModsByProfilePriority",
        //     &MOBase::IModList::allModsByProfilePriority, py::arg("profile") =
        //     py::ptr((IProfile*)nullptr))

        //     .def("getMod", &MOBase::IModList::getMod,
        //     py::return_value_policy<py::reference_existing_object>(),
        //     py::arg("name")) .def("removeMod", &MOBase::IModList::removeMod,
        //     py::arg("mod")) .def("renameMod", &MOBase::IModList::renameMod,
        //     py::return_value_policy<py::reference_existing_object>(),
        //     (py::arg("mod"), py::arg("name")))

        //     .def("state", &MOBase::IModList::state, py::arg("name"))
        //     .def("setActive",
        //       static_cast<int(IModList::*)(QStringList const&,
        //       bool)>(&MOBase::IModList::setActive), (py::arg("names"),
        //       "active"))
        //     .def("setActive",
        //       static_cast<bool(IModList::*)(QString const&,
        //       bool)>(&MOBase::IModList::setActive), (py::arg("name"),
        //       "active"))
        //     .def("priority", &MOBase::IModList::priority, py::arg("name"))
        //     .def("setPriority", &MOBase::IModList::setPriority,
        //     (py::arg("name"), "priority"))

        //     // Kept but deprecated for backward compatibility:
        //     .def("onModStateChanged", +[](IModList* modList, const
        //     std::function<void(const QString&, IModList::ModStates)>& fn) {
        //       utils::show_deprecation_warning("onModStateChanged",
        //         "onModStateChanged(Callable[[str, IModList.ModStates], None])
        //         is deprecated, " "use onModStateChanged(Callable[[Dict[str,
        //         IModList.ModStates], None]) instead.");
        //       return modList->onModStateChanged([fn](auto const& map) {
        //         for (const auto& entry : map) {
        //           fn(entry.first, entry.second);
        //         }
        //         });
        //       }, py::arg("callback"))

        //     .def("onModInstalled", &MOBase::IModList::onModInstalled,
        //     py::arg("callback")) .def("onModRemoved",
        //     &MOBase::IModList::onModRemoved, py::arg("callback"))
        //     .def("onModStateChanged", &MOBase::IModList::onModStateChanged,
        //     py::arg("callback")) .def("onModMoved",
        //     &MOBase::IModList::onModMoved, py::arg("callback"))
        //     ;
    }

    void add_iorganizer_classes(py::module_ m)
    {
        // py::class_<IOrganizer::FileInfo>("FileInfo", py::init<>())
        //   .add_property("filePath",
        //     +[](const IOrganizer::FileInfo& info) { return info.filePath; },
        //     +[](IOrganizer::FileInfo& info, QString value) { info.filePath =
        //     value;
        //     })
        //   .add_property("archive",
        //     +[](const IOrganizer::FileInfo& info) { return info.archive; },
        //     +[](IOrganizer::FileInfo& info, QString value) { info.archive =
        //     value;
        //     })
        //   .add_property("origins",
        //     +[](const IOrganizer::FileInfo& info) { return info.origins; },
        //     +[](IOrganizer::FileInfo& info, QStringList value) { info.origins
        //     = value; })
        //   ;

        // py::class_<IOrganizer, boost::noncopyable>("IOrganizer", py::no_init)
        //     .def("createNexusBridge", &IOrganizer::createNexusBridge,
        //     py::return_value_policy<py::reference_existing_object>())
        //     .def("profileName", &IOrganizer::profileName)
        //     .def("profilePath", &IOrganizer::profilePath)
        //     .def("downloadsPath", &IOrganizer::downloadsPath)
        //     .def("overwritePath", &IOrganizer::overwritePath)
        //     .def("basePath", &IOrganizer::basePath)
        //     .def("modsPath", &IOrganizer::modsPath)
        //     .def("appVersion", &IOrganizer::appVersion)
        //     .def("createMod", &IOrganizer::createMod,
        //     py::return_value_policy<py::reference_existing_object>(),
        //     py::arg("name")) .def("getGame", &IOrganizer::getGame,
        //     py::return_value_policy<py::reference_existing_object>(),
        //     py::arg("name")) .def("modDataChanged",
        //     &IOrganizer::modDataChanged, py::arg("mod"))
        //     .def("isPluginEnabled", +[](IOrganizer* o, IPlugin* plugin) {
        //     return o->isPluginEnabled(plugin); }, py::arg("plugin"))
        //     .def("isPluginEnabled", +[](IOrganizer* o, QString const& plugin)
        //     { return o->isPluginEnabled(plugin); }, py::arg("plugin"))
        //     .def("pluginSetting", &IOrganizer::pluginSetting,
        //     (py::arg("plugin_name"), "key")) .def("setPluginSetting",
        //     &IOrganizer::setPluginSetting, (py::arg("plugin_name"), "key",
        //     "value")) .def("persistent", &IOrganizer::persistent,
        //     (py::arg("plugin_name"), "key", py::arg("default") = QVariant()))
        //     .def("setPersistent", &IOrganizer::setPersistent,
        //     (py::arg("plugin_name"), "key", "value", py::arg("sync") = true))
        //     .def("pluginDataPath", &IOrganizer::pluginDataPath)
        //     .def("installMod", &IOrganizer::installMod,
        //     py::return_value_policy<py::reference_existing_object>(),
        //     (py::arg("filename"), py::arg("name_suggestion") = ""))
        //     .def("resolvePath", &IOrganizer::resolvePath,
        //     py::arg("filename")) .def("listDirectories",
        //     &IOrganizer::listDirectories, py::arg("directory"))

        //     // Provide multiple overloads of findFiles:
        //     .def("findFiles", +[](const IOrganizer* o, QString const& p,
        //     std::function<bool(QString const&)> f) { return o->findFiles(p,
        //     f);
        //     },
        //       (py::arg("path"), "filter"))

        //     // In C++, it is possible to create a QStringList implicitly from
        //     a single QString. This is not possible with the current
        //     // converters in python (and I do not think it is a good idea to
        //     have it everywhere), but here it is nice to be able to
        //     // pass a single string, so we add an extra overload.
        //     // Important: the order matters, because a Python string can be
        //     converted to a QStringList since it is a sequence of
        //     // single-character strings:
        //     .def("findFiles", +[](const IOrganizer* o, QString const& p,
        //     const QStringList& gf) { return o->findFiles(p, gf); },
        //       (py::arg("path"), "patterns"))
        //     .def("findFiles", +[](const IOrganizer* o, QString const& p,
        //     const QString& f) { return o->findFiles(p, QStringList{ f }); },
        //       (py::arg("path"), "pattern"))

        //     .def("getFileOrigins", &IOrganizer::getFileOrigins,
        //     py::arg("filename")) .def("findFileInfos",
        //     &IOrganizer::findFileInfos, (py::arg("path"), "filter"))

        //     .def("virtualFileTree", &IOrganizer::virtualFileTree)

        //     .def("downloadManager", &IOrganizer::downloadManager,
        //     py::return_value_policy<py::reference_existing_object>())
        //     .def("pluginList", &IOrganizer::pluginList,
        //     py::return_value_policy<py::reference_existing_object>())
        //     .def("modList", &IOrganizer::modList,
        //     py::return_value_policy<py::reference_existing_object>())
        //     .def("profile", &IOrganizer::profile,
        //     py::return_value_policy<py::reference_existing_object>())

        //     // Custom implementation for startApplication and
        //     waitForApplication because 1) HANDLE (= void*) is not properly
        //     // converted from/to python, and 2) we need to convert the by-ptr
        //     argument to a return-tuple for waitForApplication:
        //     .def("startApplication",
        //       +[](IOrganizer* o, const QString& executable, const
        //       QStringList& args, const QString& cwd, const QString& profile,
        //         const QString& forcedCustomOverwrite, bool
        //         ignoreCustomOverwrite)
        //         {
        //           return (std::uintptr_t) o->startApplication(executable,
        //           args, cwd, profile, forcedCustomOverwrite,
        //           ignoreCustomOverwrite);
        //       }, (py::arg("executable"), (py::arg("args") = QStringList()),
        //       (py::arg("cwd") = ""), (py::arg("profile") = ""),
        //           (py::arg("forcedCustomOverwrite") = ""),
        //           (py::arg("ignoreCustomOverwrite") = false)),
        //           py::return_value_policy<py::return_by_value>())
        //     .def("waitForApplication", +[](IOrganizer *o, std::uintptr_t
        //     handle, bool refresh) {
        //         DWORD returnCode;
        //         bool result = o->waitForApplication((HANDLE)handle, refresh,
        //         &returnCode); return std::make_tuple(result, returnCode);
        //       }, (py::arg("handle"), py::arg("refresh") = true))
        //     .def("refresh", &IOrganizer::refresh, (py::arg("save_changes") =
        //     true)) .def("managedGame", &IOrganizer::managedGame,
        //     py::return_value_policy<py::reference_existing_object>())

        //     .def("onAboutToRun", &IOrganizer::onAboutToRun,
        //     py::arg("callback")) .def("onFinishedRun",
        //     &IOrganizer::onFinishedRun, py::arg("callback"))
        //     .def("onUserInterfaceInitialized",
        //     &IOrganizer::onUserInterfaceInitialized, py::arg("callback"))
        //     .def("onProfileCreated", &IOrganizer::onProfileCreated,
        //     py::arg("callback")) .def("onProfileRenamed",
        //     &IOrganizer::onProfileRenamed, py::arg("callback"))
        //     .def("onProfileRemoved", &IOrganizer::onProfileRemoved,
        //     py::arg("callback")) .def("onProfileChanged",
        //     &IOrganizer::onProfileChanged, py::arg("callback"))

        //     .def("onPluginSettingChanged",
        //     &IOrganizer::onPluginSettingChanged, py::arg("callback"))
        //     .def("onPluginEnabled", +[](IOrganizer* o,
        //     std::function<void(const IPlugin*)> const& func) {
        //         o->onPluginEnabled(func);
        //       }, py::arg("callback"))
        //     .def("onPluginEnabled", +[](IOrganizer* o, QString const& name,
        //     std::function<void()> const& func) {
        //         o->onPluginEnabled(name, func);
        //       }, (py::arg("name"), py::arg("callback")))
        //     .def("onPluginDisabled", +[](IOrganizer* o,
        //     std::function<void(const IPlugin*)> const& func) {
        //         o->onPluginDisabled(func);
        //       }, py::arg("callback"))
        //     .def("onPluginDisabled", +[](IOrganizer* o, QString const& name,
        //     std::function<void()> const& func) {
        //         o->onPluginDisabled(name, func);
        //       }, (py::arg("name"), py::arg("callback")))

        //     // DEPRECATED:
        //     .def("getMod", +[](IOrganizer* o, QString const& name) {
        //         utils::show_deprecation_warning("getMod",
        //           "IOrganizer::getMod(str) is deprecated, use
        //           IModList::getMod(str) instead.");
        //         return o->modList()->getMod(name);
        //     }, py::return_value_policy<py::reference_existing_object>(),
        //     py::arg("name")) .def("removeMod", +[](IOrganizer* o,
        //     IModInterface *mod) {
        //         utils::show_deprecation_warning("removeMod",
        //           "IOrganizer::removeMod(IModInterface) is deprecated, use
        //           IModList::removeMod(IModInterface) instead.");
        //         return o->modList()->removeMod(mod);
        //     }, py::arg("mod"))
        //     .def("modsSortedByProfilePriority", +[](IOrganizer* o) {
        //         utils::show_deprecation_warning("modsSortedByProfilePriority",
        //           "IOrganizer::modsSortedByProfilePriority() is deprecated,
        //           use IModList::allModsByProfilePriority() instead.");
        //         return o->modList()->allModsByProfilePriority();
        //     })
        //     .def("refreshModList", +[](IOrganizer* o, bool s) {
        //       utils::show_deprecation_warning("refreshModList",
        //         "IOrganizer::refreshModList(bool) is deprecated, use
        //         IOrganizer::refresh(bool) instead.");
        //       o->refresh(s);
        //     }, (py::arg("save_changes") = true))
        //     .def("onModInstalled", +[](IOrganizer* organizer, const
        //     std::function<void(QString const&)>& func) {
        //       utils::show_deprecation_warning("onModInstalled",
        //         "IOrganizer::onModInstalled(Callable[[str], None]) is
        //         deprecated, " "use
        //         IModList::onModInstalled(Callable[[IModInterface], None])
        //         instead.");
        //       return
        //       organizer->modList()->onModInstalled([func](MOBase::IModInterface*
        //       m) { func(m->name()); });;
        //     }, py::arg("callback"))

        //     .def("getPluginDataPath", &IOrganizer::getPluginDataPath)
        //     .staticmethod("getPluginDataPath")

        //     ;
    }

    void add_idownload_manager_classes(py::module_ m)
    {
        // py::class_<IDownloadManager, boost::noncopyable>("IDownloadManager",
        // py::no_init)
        //     .def("startDownloadURLs", &IDownloadManager::startDownloadURLs,
        //     py::arg("urls")) .def("startDownloadNexusFile",
        //     &IDownloadManager::startDownloadNexusFile, (py::arg("mod_id"),
        //     "file_id")) .def("downloadPath", &IDownloadManager::downloadPath,
        //     py::arg("id")) .def("onDownloadComplete",
        //     &IDownloadManager::onDownloadComplete, py::arg("callback"))
        //     .def("onDownloadPaused", &IDownloadManager::onDownloadPaused,
        //     py::arg("callback")) .def("onDownloadFailed",
        //     &IDownloadManager::onDownloadFailed, py::arg("callback"))
        //     .def("onDownloadRemoved", &IDownloadManager::onDownloadRemoved,
        //     py::arg("callback"))
        //     ;
    }

    void add_iinstallation_manager_classes(py::module_ m)
    {
        // py::class_<IInstallationManager,
        // boost::noncopyable>("IInstallationManager", py::no_init)
        //   .def("getSupportedExtensions",
        //   &IInstallationManager::getSupportedExtensions) .def("extractFile",
        //   &IInstallationManager::extractFile, (py::arg("entry"),
        //   py::arg("silent") = false)) .def("extractFiles",
        //   &IInstallationManager::extractFiles, (py::arg("entries"),
        //   py::arg("silent") = false)) .def("createFile",
        //   +[](IInstallationManager* m, std::shared_ptr<const
        //   MOBase::FileTreeEntry> entry) {
        //       return m->createFile(utils::clean_shared_ptr(entry));
        //     }, py::arg("entry"))

        //   // accept both QString and GuessedValue<QString> since the
        //   conversion is not automatic in Python, and
        //   // return a tuple to get back the mod name and the mod ID
        //   .def("installArchive", +[](IInstallationManager* m,
        //   std::variant<QString, GuessedValue<QString>> modName, QString
        //   archive, int modId) {
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
    }

    void add_basic_bindings(py::module_ m)
    {
        add_versioninfo_classes(m);
        add_executable_classes(m);

        add_ifiletree_bindings(m);

        add_modinterface_classes(m);
        add_modrepository_classes(m);

        add_ipluginlist_classes(m);
        add_imodlist_classes(m);
        add_iorganizer_classes(m);
        add_idownload_manager_classes(m);
        add_iinstallation_manager_classes(m);

        py::class_<PluginSetting>(m, "PluginSetting")
            .def(py::init<const QString&, const QString&, const QVariant&>(),
                 py::arg("key"), py::arg("description"), py::arg("default_value"))
            .def_readwrite("key", &PluginSetting::key)
            .def_readwrite("description", &PluginSetting::description)
            .def_readwrite("default_value", &PluginSetting::defaultValue);

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
            .def("absoluteIniFilePath", &IProfile::absoluteIniFilePath,
                 py::arg("inifile"));

        // py::class_<PluginRequirementFactory,
        // boost::noncopyable>("PluginRequirementFactory")
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
        //   .def("diagnose", &PluginRequirementFactory::diagnose,
        //   py::arg("diagnose")) .staticmethod("diagnose")
        //   // basic
        //   .def("basic", &PluginRequirementFactory::basic,
        //   (py::arg("checker"), "description")) .staticmethod("basic");
        // py::class_<Mapping>("Mapping", py::init<>())
        //     .def("__init__", py::make_constructor(+[](QString src, QString
        //     dst, bool dir, bool crt) -> Mapping* {
        //       return new Mapping{ src, dst, dir, crt };
        //       }, py::default_call_policies(),
        //         (py::arg("source"), py::arg("destination"),
        //         py::arg("is_directory"), py::arg("create_target") = false)))
        //     .def_readwrite("source", &Mapping::source)
        //     .def_readwrite("destination", &Mapping::destination)
        //     .def_readwrite("isDirectory", &Mapping::isDirectory)
        //     .def_readwrite("createTarget", &Mapping::createTarget)
        //     .def("__str__", +[](Mapping * m) {
        //       return fmt::format(L"Mapping({}, {}, {}, {})",
        //       m->source.toStdWString(), m->destination.toStdWString(),
        //       m->isDirectory, m->createTarget);
        //     })
        //     ;

        // py::class_<Mapping>("Mapping", py::init<>())
        //     .def("__init__", py::make_constructor(+[](QString src, QString
        //     dst, bool dir, bool crt) -> Mapping* {
        //       return new Mapping{ src, dst, dir, crt };
        //       }, py::default_call_policies(),
        //         (py::arg("source"), py::arg("destination"),
        //         py::arg("is_directory"), py::arg("create_target") = false)))
        //     .def_readwrite("source", &Mapping::source)
        //     .def_readwrite("destination", &Mapping::destination)
        //     .def_readwrite("isDirectory", &Mapping::isDirectory)
        //     .def_readwrite("createTarget", &Mapping::createTarget)
        //     .def("__str__", +[](Mapping * m) {
        //       return fmt::format(L"Mapping({}, {}, {}, {})",
        //       m->source.toStdWString(), m->destination.toStdWString(),
        //       m->isDirectory, m->createTarget);
        //     })
        //     ;
    }

}  // namespace mo2::python
