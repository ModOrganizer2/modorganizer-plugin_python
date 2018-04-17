#include "proxypluginwrappers.h"
#include <utility.h>
#include "error.h"
#include "gilock.h"
#include <QWidget>

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

#define PYCATCH catch (const boost::python::error_already_set &) { reportPythonError(); throw MyException("unhandled exception"); }\
                catch (...) { throw MyException("An unknown exception was thrown in python code"); }


/////////////////////////////
/// IPlugin Wrapper


bool IPluginWrapper::init(MOBase::IOrganizer *moInfo)
{
  try {
    return this->get_override("init")(boost::python::ptr(moInfo));
  } PYCATCH;
}

QString IPluginWrapper::name() const
{
  try {
    return this->get_override("name")().as<QString>();
  } PYCATCH;
}

QString IPluginWrapper::author() const
{
  try {
    return this->get_override("author")().as<QString>();
  } PYCATCH;
}

QString IPluginWrapper::description() const
{
  try {
    return this->get_override("description")().as<QString>();
  } PYCATCH;
}

MOBase::VersionInfo IPluginWrapper::version() const
{
  try {
    return this->get_override("version")().as<MOBase::VersionInfo>();
  } PYCATCH;
}

bool IPluginWrapper::isActive() const
{
  try {
    return this->get_override("isActive")().as<bool>();
  } PYCATCH;
}

QList<MOBase::PluginSetting> IPluginWrapper::settings() const
{
  try {
    return this->get_override("settings")().as<QList<MOBase::PluginSetting>>();
  } PYCATCH;
}
/// end IPlugin Wrapper
/////////////////////////////////////
/// IPluginGame Wrapper


QString IPluginGameWrapper::gameName() const
{
  return this->get_override("gameName")();
}

void IPluginGameWrapper::initializeProfile(const QDir & directory, ProfileSettings settings) const
{
  this->get_override("initializeProfile")(directory, settings);
}

QString IPluginGameWrapper::savegameExtension() const
{
  return this->get_override("savegameExtension")();
}

QString IPluginGameWrapper::savegameSEExtension() const
{
  return this->get_override("savegameSEExtension")();
}

bool IPluginGameWrapper::isInstalled() const
{
  return this->get_override("isInstalled")();
}

QIcon IPluginGameWrapper::gameIcon() const
{
  return this->get_override("gameIcon")();
}

QDir IPluginGameWrapper::gameDirectory() const
{
  return this->get_override("gameDirectory")();
}

QDir IPluginGameWrapper::dataDirectory() const
{
  return this->get_override("dataDirectory")();
}

void IPluginGameWrapper::setGamePath(const QString & path)
{
  this->get_override("setGamePath")(path);
}

QDir IPluginGameWrapper::documentsDirectory() const
{
  return this->get_override("documentsDirectory")();
}

QDir IPluginGameWrapper::savesDirectory() const
{
  return this->get_override("savesDirectory")();
}

QList<MOBase::ExecutableInfo> IPluginGameWrapper::executables() const
{
  return this->get_override("executables")();
}

QString IPluginGameWrapper::steamAPPId() const
{
  return this->get_override("steamAPPId")();
}

QStringList IPluginGameWrapper::primaryPlugins() const
{
  return this->get_override("primaryPlugins")();
}

QStringList IPluginGameWrapper::gameVariants() const
{
  return this->get_override("gameVariants")();
}

void IPluginGameWrapper::setGameVariant(const QString & variant)
{
  this->get_override("setGameVariant")(variant);
}

QString IPluginGameWrapper::binaryName() const
{
  return this->get_override("binaryName")();
}

QString IPluginGameWrapper::gameShortName() const
{
  return this->get_override("gameShortName")();
}

QStringList IPluginGameWrapper::validShortNames() const
{
  return this->get_override("validShortNames")();
}

QString IPluginGameWrapper::gameNexusName() const
{
  return this->get_override("gameNexusName")();
}

QStringList IPluginGameWrapper::iniFiles() const
{
  return this->get_override("iniFiles")();
}

QStringList IPluginGameWrapper::DLCPlugins() const
{
  return this->get_override("DLCPlugins")();
}

QStringList IPluginGameWrapper::CCPlugins() const
{
  return this->get_override("CCPlugins")();
}

IPluginGame::LoadOrderMechanism IPluginGameWrapper::loadOrderMechanism() const
{
  return this->get_override("loadorderMechanism")();
}

IPluginGame::SortMechanism IPluginGameWrapper::sortMechanism() const
{
  return this->get_override("sortMechanism")();
}

int IPluginGameWrapper::nexusModOrganizerID() const
{
  return this->get_override("nexusModOrganizerID")();
}

int IPluginGameWrapper::nexusGameID() const
{
  return this->get_override("nexusGameID")();
}

bool IPluginGameWrapper::looksValid(QDir const & dir) const
{
  return this->get_override("looksValid")(dir);
}

QString IPluginGameWrapper::gameVersion() const
{
  return this->get_override("gameVersion")();
}

QString IPluginGameWrapper::getLauncherName() const
{
  return this->get_override("getLauncherName")();
}

bool IPluginGameWrapper::init(MOBase::IOrganizer * moInfo)
{
  return this->get_override("init")(moInfo);
}

QString IPluginGameWrapper::name() const
{
  return this->get_override("name")();
}

QString IPluginGameWrapper::author() const
{
  return this->get_override("author")();
}

QString IPluginGameWrapper::description() const
{
  return this->get_override("description")();
}

MOBase::VersionInfo IPluginGameWrapper::version() const
{
  return this->get_override("version")();
}

bool IPluginGameWrapper::isActive() const
{
  return this->get_override("isActive")();
}

QList<MOBase::PluginSetting> IPluginGameWrapper::settings() const
{
  return this->get_override("settings")();
}

std::map<std::type_index, boost::any> IPluginGameWrapper::featureList() const
{
  return this->get_override("_featureList")();
}
/// end IPluginGame Wrapper
/////////////////////////////////////
/// IPluginInstallerCustom Wrapper


bool IPluginInstallerCustomWrapper::init(MOBase::IOrganizer *moInfo)
{
  try {
    return this->get_override("init")(boost::python::ptr(moInfo));
  } PYCATCH;
}

QString IPluginInstallerCustomWrapper::name() const
{
  try {
    return this->get_override("name")().as<QString>();
  } PYCATCH;
}

QString IPluginInstallerCustomWrapper::author() const
{
  try {
    return this->get_override("author")().as<QString>();
  } PYCATCH;
}

QString IPluginInstallerCustomWrapper::description() const
{
  try {
    return this->get_override("description")().as<QString>();
  } PYCATCH;
}

MOBase::VersionInfo IPluginInstallerCustomWrapper::version() const
{
  try {
    return this->get_override("version")();
  } PYCATCH;
}

bool IPluginInstallerCustomWrapper::isActive() const
{
  try {
    return this->get_override("isActive")();
  } PYCATCH;
}

QList<MOBase::PluginSetting> IPluginInstallerCustomWrapper::settings() const
{
  try {
    return this->get_override("settings")().as<QList<MOBase::PluginSetting>>();
  } PYCATCH;
}

unsigned int IPluginInstallerCustomWrapper::priority() const
{
  try {
    return this->get_override("priority")();
  } PYCATCH;
}

bool IPluginInstallerCustomWrapper::isManualInstaller() const
{
  try {
    return this->get_override("isManualInstaller")();
  } PYCATCH;
}

bool IPluginInstallerCustomWrapper::isArchiveSupported(const DirectoryTree &) const
{
  try {
    //return this->get_override("isArchiveSupported")(tree);
    return false;
  } PYCATCH;
}

bool IPluginInstallerCustomWrapper::isArchiveSupported(const QString &archiveName) const
{
  try {
    return this->get_override("isArchiveSupported")(archiveName);
  } PYCATCH;
}

std::set<QString> IPluginInstallerCustomWrapper::supportedExtensions() const
{
  try {
    return this->get_override("supportedExtensions")().as<std::set<QString>>();
  } PYCATCH;
}


IPluginInstaller::EInstallResult IPluginInstallerCustomWrapper::install(GuessedValue<QString> &modName, const QString &archiveName,
                                                                        const QString &version, int modID)
{
  try {
    return this->get_override("install")(modName, archiveName, version, modID);
  } PYCATCH;
}


void IPluginInstallerCustomWrapper::setParentWidget(QWidget *parent)
{
  try {
    this->get_override("setParentWidget")(parent);
  } PYCATCH;
}
/// end IPluginInstallerCustom Wrapper
/////////////////////////////
/// IPluginTool Wrapper


bool IPluginToolWrapper::init(MOBase::IOrganizer *moInfo)
{
  try {
    return this->get_override("init")(boost::python::ptr(moInfo));
  } PYCATCH;
}

QString IPluginToolWrapper::name() const
{
  try {
    return this->get_override("name")().as<QString>();
  } PYCATCH;
}

QString IPluginToolWrapper::author() const
{
  try {
    return this->get_override("author")().as<QString>();
  } PYCATCH;
}

QString IPluginToolWrapper::description() const
{
  try {
    return this->get_override("description")().as<QString>();
  } PYCATCH;
}

MOBase::VersionInfo IPluginToolWrapper::version() const
{
  try {
    return this->get_override("version")().as<MOBase::VersionInfo>();
  } PYCATCH;
}

bool IPluginToolWrapper::isActive() const
{
  try {
    return this->get_override("isActive")().as<bool>();
  } PYCATCH;
}

QList<MOBase::PluginSetting> IPluginToolWrapper::settings() const
{
  try {
    return this->get_override("settings")().as<QList<MOBase::PluginSetting>>();
  } PYCATCH;
}

QString IPluginToolWrapper::displayName() const
{
  try {
    return this->get_override("displayName")().as<QString>();
  } PYCATCH;
}

QString IPluginToolWrapper::tooltip() const
{
  try {
    return this->get_override("tooltip")().as<QString>();
  } PYCATCH;
}

QIcon IPluginToolWrapper::icon() const
{
  try {
    return this->get_override("icon")().as<QIcon>();
  } PYCATCH;
}

void IPluginToolWrapper::setParentWidget(QWidget *parent)
{
  try {
    this->get_override("setParentWidget")(parent);
  } PYCATCH;
}

void IPluginToolWrapper::display() const
{
  try {
    GILock lock;

    this->get_override("display")();
  } PYCATCH;
}

/// end IPluginTool Wrapper

