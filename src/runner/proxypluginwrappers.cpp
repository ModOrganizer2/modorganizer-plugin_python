#include "proxypluginwrappers.h"

#include "gilock.h"
#include <QUrl>
#include <QWidget>

#include "pythonwrapperutilities.h"

#include <variant>
#include <tuple>

namespace boost
{
 // See bug https://connect.microsoft.com/VisualStudio/Feedback/Details/2852624
#if (_MSC_VER == 1900)
	template<> const volatile MOBase::IOrganizer* get_pointer(const volatile MOBase::IOrganizer* p) { return p; }
	template<> const volatile MOBase::IModInterface* get_pointer(const volatile MOBase::IModInterface* p) { return p; }
	template<> const volatile MOBase::IPluginGame* get_pointer(const volatile MOBase::IPluginGame* p) { return p; }
	template<> const volatile MOBase::IProfile* get_pointer(const volatile MOBase::IProfile* p) { return p; }
	template<> const volatile MOBase::IModList* get_pointer(const volatile MOBase::IModList* p) { return p; }
	template<> const volatile MOBase::IPluginList* get_pointer(const volatile MOBase::IPluginList* p) { return p; }
	template<> const volatile MOBase::IDownloadManager* get_pointer(const volatile MOBase::IDownloadManager* p) { return p; }
	template<> const volatile MOBase::IModRepositoryBridge* get_pointer(const volatile MOBase::IModRepositoryBridge* p) { return p; }
#endif
}

using namespace MOBase;


#define COMMON_I_PLUGIN_WRAPPER_DEFINITIONS_(class_name, include_requirements) \
bool class_name::init(MOBase::IOrganizer *moInfo) \
{ \
  return basicWrapperFunctionImplementation<bool>(this, "init", boost::python::ptr(moInfo)); \
} \
 \
QString class_name::name() const \
{ \
  return basicWrapperFunctionImplementation<QString>(this, "name"); \
} \
 \
QString class_name::localizedName() const \
{ \
  return basicWrapperFunctionImplementationWithDefault<QString>(this, &class_name::localizedName_Default, "localizedName"); \
} \
 \
QString class_name::master() const \
{ \
  return basicWrapperFunctionImplementationWithDefault<QString>(this, &class_name::master_Default, "master"); \
} \
 \
QString class_name::author() const \
{ \
  return basicWrapperFunctionImplementation<QString>(this, "author"); \
} \
 \
QString class_name::description() const \
{ \
  return basicWrapperFunctionImplementation<QString>(this, "description"); \
} \
 \
MOBase::VersionInfo class_name::version() const \
{ \
  return basicWrapperFunctionImplementation<MOBase::VersionInfo>(this, "version"); \
} \
 \
QList<MOBase::PluginSetting> class_name::settings() const \
{ \
  return basicWrapperFunctionImplementation<QList<MOBase::PluginSetting>>(this, "settings"); \
} \
QString class_name::localizedName_Default() const { return IPlugin::localizedName(); } \
QString class_name::master_Default() const { return IPlugin::master(); } \
BOOST_PP_EXPR_IF(include_requirements, \
  QList<IPluginRequirement*> class_name::requirements() const { \
    return basicWrapperFunctionImplementationWithDefault<QList<IPluginRequirement*>>( \
    this, &class_name::requirements_Default, m_Requirements, "requirements"); \
  } \
  QList<IPluginRequirement*> class_name::requirements_Default() const { return IPlugin::requirements(); })

#define COMMON_I_PLUGIN_WRAPPER_DEFINITIONS(class_name) COMMON_I_PLUGIN_WRAPPER_DEFINITIONS_(class_name, 1)

/// end COMMON_I_PLUGIN_WRAPPER_DEFINITIONS
/////////////////////////////
/// IPlugin Wrapper


COMMON_I_PLUGIN_WRAPPER_DEFINITIONS(IPluginWrapper)
/// end IPlugin Wrapper
/////////////////////////////////////
/// IPluginDiagnose Wrapper


COMMON_I_PLUGIN_WRAPPER_DEFINITIONS(IPluginDiagnoseWrapper)

std::vector<unsigned int> IPluginDiagnoseWrapper::activeProblems() const
{
  return basicWrapperFunctionImplementation<std::vector<unsigned int>>(this, "activeProblems");
}

QString IPluginDiagnoseWrapper::shortDescription(unsigned int key) const
{
  return basicWrapperFunctionImplementation<QString>(this, "shortDescription", key);
}

QString IPluginDiagnoseWrapper::fullDescription(unsigned int key) const
{
  return basicWrapperFunctionImplementation<QString>(this, "fullDescription", key);
}

bool IPluginDiagnoseWrapper::hasGuidedFix(unsigned int key) const
{
  return basicWrapperFunctionImplementation<bool>(this, "hasGuidedFix", key);
}

void IPluginDiagnoseWrapper::startGuidedFix(unsigned int key) const
{
  basicWrapperFunctionImplementation<void>(this, "startGuidedFix", key);
}

/// end IPluginDiagnose Wrapper
/////////////////////////////////////
/// IPluginFileMapper Wrapper


COMMON_I_PLUGIN_WRAPPER_DEFINITIONS(IPluginFileMapperWrapper)

MappingType IPluginFileMapperWrapper::mappings() const
{
  return basicWrapperFunctionImplementation<MappingType>(this, "mappings");
}
/// end IPluginFileMapper Wrapper
/////////////////////////////////////
/// IPluginGame Wrapper

void IPluginGameWrapper::detectGame()
{
  return basicWrapperFunctionImplementation<void>(this, "detectGame");
}

QString IPluginGameWrapper::gameName() const
{
  return basicWrapperFunctionImplementation<QString>(this, "gameName");
}

void IPluginGameWrapper::initializeProfile(const QDir & directory, ProfileSettings settings) const
{
  basicWrapperFunctionImplementation<void>(this, "initializeProfile", directory, settings);
}

QString IPluginGameWrapper::savegameExtension() const
{
  return basicWrapperFunctionImplementation<QString>(this, "savegameExtension");
}

QString IPluginGameWrapper::savegameSEExtension() const
{
  return basicWrapperFunctionImplementation<QString>(this, "savegameSEExtension");
}

bool IPluginGameWrapper::isInstalled() const
{
  return basicWrapperFunctionImplementation<bool>(this, "isInstalled");
}

QIcon IPluginGameWrapper::gameIcon() const
{
  return basicWrapperFunctionImplementation<QIcon>(this, "gameIcon");
}

QDir IPluginGameWrapper::gameDirectory() const
{
  return basicWrapperFunctionImplementation<QDir>(this, "gameDirectory");
}

QDir IPluginGameWrapper::dataDirectory() const
{
  return basicWrapperFunctionImplementation<QDir>(this, "dataDirectory");
}

void IPluginGameWrapper::setGamePath(const QString & path)
{
  basicWrapperFunctionImplementation<void>(this, "setGamePath", path);
}

QDir IPluginGameWrapper::documentsDirectory() const
{
  return basicWrapperFunctionImplementation<QDir>(this, "documentsDirectory");
}

QDir IPluginGameWrapper::savesDirectory() const
{
  return basicWrapperFunctionImplementation<QDir>(this, "savesDirectory");
}

QList<MOBase::ExecutableInfo> IPluginGameWrapper::executables() const
{
  return basicWrapperFunctionImplementation<QList<MOBase::ExecutableInfo>>(this, "executables");
}

QList<MOBase::ExecutableForcedLoadSetting> IPluginGameWrapper::executableForcedLoads() const
{
  return basicWrapperFunctionImplementation<QList<MOBase::ExecutableForcedLoadSetting>>(this, "executableForcedLoads");
}

QString IPluginGameWrapper::steamAPPId() const
{
  return basicWrapperFunctionImplementation<QString>(this, "steamAPPId");
}

QStringList IPluginGameWrapper::primaryPlugins() const
{
  return basicWrapperFunctionImplementation<QStringList>(this, "primaryPlugins");
}

QStringList IPluginGameWrapper::gameVariants() const
{
  return basicWrapperFunctionImplementation<QStringList>(this, "gameVariants");
}

void IPluginGameWrapper::setGameVariant(const QString & variant)
{
  basicWrapperFunctionImplementation<void>(this, "setGameVariant", variant);
}

QString IPluginGameWrapper::binaryName() const
{
  return basicWrapperFunctionImplementation<QString>(this, "binaryName");
}

QString IPluginGameWrapper::gameShortName() const
{
  return basicWrapperFunctionImplementation<QString>(this, "gameShortName");
}

QStringList IPluginGameWrapper::primarySources() const
{
  return basicWrapperFunctionImplementation<QStringList>(this, "primarySources");
}

QStringList IPluginGameWrapper::validShortNames() const
{
  return basicWrapperFunctionImplementation<QStringList>(this, "validShortNames");
}

QString IPluginGameWrapper::gameNexusName() const
{
  return basicWrapperFunctionImplementation<QString>(this, "gameNexusName");
}

QStringList IPluginGameWrapper::iniFiles() const
{
  return basicWrapperFunctionImplementation<QStringList>(this, "iniFiles");
}

QStringList IPluginGameWrapper::DLCPlugins() const
{
  return basicWrapperFunctionImplementation<QStringList>(this, "DLCPlugins");
}

QStringList IPluginGameWrapper::CCPlugins() const
{
  return basicWrapperFunctionImplementation<QStringList>(this, "CCPlugins");
}

IPluginGame::LoadOrderMechanism IPluginGameWrapper::loadOrderMechanism() const
{
  return basicWrapperFunctionImplementation<IPluginGame::LoadOrderMechanism>(this, "loadOrderMechanism");
}

IPluginGame::SortMechanism IPluginGameWrapper::sortMechanism() const
{
  return basicWrapperFunctionImplementation<IPluginGame::SortMechanism>(this, "sortMechanism");
}

int IPluginGameWrapper::nexusModOrganizerID() const
{
  return basicWrapperFunctionImplementation<int>(this, "nexusModOrganizerID");
}

int IPluginGameWrapper::nexusGameID() const
{
  return basicWrapperFunctionImplementation<int>(this, "nexusGameID");
}

bool IPluginGameWrapper::looksValid(QDir const & dir) const
{
  return basicWrapperFunctionImplementation<bool>(this, "looksValid", dir);
}

QString IPluginGameWrapper::gameVersion() const
{
  return basicWrapperFunctionImplementation<QString>(this, "gameVersion");
}

QString IPluginGameWrapper::getLauncherName() const
{
  return basicWrapperFunctionImplementation<QString>(this, "getLauncherName");
}

COMMON_I_PLUGIN_WRAPPER_DEFINITIONS_(IPluginGameWrapper, 0)

std::map<std::type_index, boost::any> IPluginGameWrapper::featureList() const
{
  return basicWrapperFunctionImplementation<std::map<std::type_index, boost::any>>(this, "_featureList");
}
/// end IPluginGame Wrapper
/////////////////////////////////////
/// IPluginInstaller macro

#define COMMON_I_PLUGIN_INSTALLER_WRAPPER_DEFINITIONS(class_name) \
unsigned int class_name::priority() const { return basicWrapperFunctionImplementation<unsigned int>(this, "priority"); } \
bool class_name::isManualInstaller() const { return basicWrapperFunctionImplementation<bool>(this, "isManualInstaller"); } \
void class_name::onInstallationStart(QString const& archive, bool reinstallation, MOBase::IModInterface* currentMod) { \
  basicWrapperFunctionImplementationWithDefault<void>(this, &class_name::onInstallationStart_Default, "onInstallationStart", archive, reinstallation, boost::python::ptr(currentMod)); } \
void class_name::onInstallationEnd(EInstallResult result, MOBase::IModInterface* newMod) { \
  basicWrapperFunctionImplementationWithDefault<void>(this, &class_name::onInstallationEnd_Default, "onInstallationEnd", result, boost::python::ptr(newMod)); } \
bool class_name::isArchiveSupported(std::shared_ptr<const IFileTree> tree) const { return basicWrapperFunctionImplementation<bool>(this, "isArchiveSupported", tree); }

/// end IPluginInstaller macro
/////////////////////////////////////
/// IPluginInstaller Wrapper

COMMON_I_PLUGIN_WRAPPER_DEFINITIONS(IPluginInstallerSimpleWrapper)
COMMON_I_PLUGIN_INSTALLER_WRAPPER_DEFINITIONS(IPluginInstallerSimpleWrapper)

IPluginInstaller::EInstallResult IPluginInstallerSimpleWrapper::install(
  GuessedValue<QString>& modName, std::shared_ptr<IFileTree>& tree,
  QString& version, int& nexusID)
{
  namespace bpy = boost::python;

  using return_type = std::variant<
    IPluginInstaller::EInstallResult,
    std::shared_ptr<IFileTree>,
    std::tuple<IPluginInstaller::EInstallResult, std::shared_ptr<IFileTree>, QString, int>> ;
  auto ret = basicWrapperFunctionImplementation<return_type>(this, "install", boost::ref(modName), tree, version, nexusID);

  return std::visit([&](auto const& t) {
    using type = std::decay_t<decltype(t)>;
    if constexpr (std::is_same_v<type, IPluginInstaller::EInstallResult>) {
      return t;
    }
    else if constexpr (std::is_same_v<type, std::shared_ptr<IFileTree>>) {
      tree = t;
      return IPluginInstaller::RESULT_SUCCESS;
    }
    else if constexpr (std::is_same_v<type, std::tuple<IPluginInstaller::EInstallResult, std::shared_ptr<IFileTree>, QString, int>>) {
      tree = std::get<1>(t);
      version = std::get<2>(t);
      nexusID = std::get<3>(t);
      return std::get<0>(t);
    }
  }, ret);
}

/// end IPluginInstallerSimple Wrapper
/////////////////////////////////////
/// IPluginInstallerCustom Wrapper
COMMON_I_PLUGIN_WRAPPER_DEFINITIONS(IPluginInstallerCustomWrapper)
COMMON_I_PLUGIN_INSTALLER_WRAPPER_DEFINITIONS(IPluginInstallerCustomWrapper)

bool IPluginInstallerCustomWrapper::isArchiveSupported(const QString &archiveName) const
{
  return basicWrapperFunctionImplementation<bool>(this, "isArchiveSupported", archiveName);
}

std::set<QString> IPluginInstallerCustomWrapper::supportedExtensions() const
{
  return basicWrapperFunctionImplementation<std::set<QString>>(this, "supportedExtensions");
}

IPluginInstaller::EInstallResult IPluginInstallerCustomWrapper::install(
  GuessedValue<QString> &modName, QString gameName, const QString &archiveName, const QString &version, int modID)
{
  // Note: This requires far more less trouble than the "Simple" installer version since 1) there is no tree
  // and 2) there version and modId cannot be modified:
  return basicWrapperFunctionImplementation<IPluginInstaller::EInstallResult>(
    this, "install", boost::ref(modName), gameName, archiveName, version, modID);
}

/// end IPluginInstallerCustom Wrapper
/////////////////////////////
/// IPluginModPage Wrapper


COMMON_I_PLUGIN_WRAPPER_DEFINITIONS(IPluginModPageWrapper)

QString IPluginModPageWrapper::displayName() const
{
  return basicWrapperFunctionImplementation<QString>(this, "displayName");
}

QIcon IPluginModPageWrapper::icon() const
{
  return basicWrapperFunctionImplementation<QIcon>(this, "icon");
}

QUrl IPluginModPageWrapper::pageURL() const
{
  return basicWrapperFunctionImplementation<QUrl>(this, "pageURL");
}

bool IPluginModPageWrapper::useIntegratedBrowser() const
{
  return basicWrapperFunctionImplementation<bool>(this, "useIntegratedBrowser");
}

bool IPluginModPageWrapper::handlesDownload(const QUrl & pageURL, const QUrl & downloadURL, MOBase::ModRepositoryFileInfo & fileInfo) const
{
  return basicWrapperFunctionImplementation<bool>(this, "handlesDownload", pageURL, downloadURL, fileInfo);
}

void IPluginModPageWrapper::setParentWidget(QWidget * widget)
{
  basicWrapperFunctionImplementationWithDefault<void>(this, &IPluginModPageWrapper::setParentWidget_Default, "setParentWidget", widget);
}
/// end IPluginModPage Wrapper
/////////////////////////////
/// IPluginPreview Wrapper


COMMON_I_PLUGIN_WRAPPER_DEFINITIONS(IPluginPreviewWrapper)

std::set<QString> IPluginPreviewWrapper::supportedExtensions() const
{
  return basicWrapperFunctionImplementation<std::set<QString>>(this, "supportedExtensions");
}

QWidget *IPluginPreviewWrapper::genFilePreview(const QString &fileName, const QSize &maxSize) const
{
  // We need responsibility for deleting the QWidget to be transferred to C++:
  return wrapperFunctionImplementationWithApiTransfer<QWidget*>(this, "genFilePreview", fileName, maxSize);
}
/// end IPluginPreview Wrapper
/////////////////////////////
/// IPluginTool Wrapper


COMMON_I_PLUGIN_WRAPPER_DEFINITIONS(IPluginToolWrapper)

QString IPluginToolWrapper::displayName() const
{
  return basicWrapperFunctionImplementation<QString>(this, "displayName");
}

QString IPluginToolWrapper::tooltip() const
{
  return basicWrapperFunctionImplementation<QString>(this, "tooltip");
}

QIcon IPluginToolWrapper::icon() const
{
  return basicWrapperFunctionImplementation<QIcon>(this, "icon");
}

void IPluginToolWrapper::setParentWidget(QWidget *parent)
{
  basicWrapperFunctionImplementationWithDefault<void>(this, &IPluginToolWrapper::setParentWidget_Default, "setParentWidget", parent);
}

void IPluginToolWrapper::display() const
{
  basicWrapperFunctionImplementation<void>(this, "display");
}

/// end IPluginTool Wrapper
