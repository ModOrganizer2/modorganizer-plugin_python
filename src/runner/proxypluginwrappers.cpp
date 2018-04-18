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


#define COMMON_I_PLUGIN_WRAPPER_DEFINITIONS(class_name) \
bool class_name::init(MOBase::IOrganizer *moInfo) \
{ \
  try { \
    return this->get_override("init")(boost::python::ptr(moInfo)); \
  } PYCATCH; \
} \
 \
QString class_name::name() const \
{ \
  try { \
    return this->get_override("name")().as<QString>(); \
  } PYCATCH; \
} \
 \
QString class_name::author() const \
{ \
  try { \
    return this->get_override("author")().as<QString>(); \
  } PYCATCH; \
} \
 \
QString class_name::description() const \
{ \
  try { \
    return this->get_override("description")().as<QString>(); \
  } PYCATCH; \
} \
 \
MOBase::VersionInfo class_name::version() const \
{ \
  try { \
    return this->get_override("version")().as<MOBase::VersionInfo>(); \
  } PYCATCH; \
} \
 \
bool class_name::isActive() const \
{ \
  try { \
    return this->get_override("isActive")().as<bool>(); \
  } PYCATCH; \
} \
 \
QList<MOBase::PluginSetting> class_name::settings() const \
{ \
  try { \
    return this->get_override("settings")().as<QList<MOBase::PluginSetting>>(); \
  } PYCATCH; \
}

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
  try {
    GILock lock;

    return this->get_override("activeProblems")();
  } PYCATCH;
}

QString IPluginDiagnoseWrapper::shortDescription(unsigned int key) const
{
  try {
    return this->get_override("shortDescription")(key);
  } PYCATCH;
}

QString IPluginDiagnoseWrapper::fullDescription(unsigned int key) const
{
  try {
    return this->get_override("fullDescription")(key);
  } PYCATCH;
}

bool IPluginDiagnoseWrapper::hasGuidedFix(unsigned int key) const
{
  try {
    return this->get_override("hasGuidedFix")(key);
  } PYCATCH;
}

void IPluginDiagnoseWrapper::startGuidedFix(unsigned int key) const
{
  try {
    GILock lock;

    this->get_override("startGuidedFix")(key);
  } PYCATCH;
}

void IPluginDiagnoseWrapper::invalidate()
{
  IPluginDiagnose::invalidate();
}
/// end IPluginDiagnose Wrapper
/////////////////////////////////////
/// IPluginGame Wrapper


QString IPluginGameWrapper::gameName() const
{
  try {
    return this->get_override("gameName")();
  } PYCATCH;
}

void IPluginGameWrapper::initializeProfile(const QDir & directory, ProfileSettings settings) const
{
  try {
    this->get_override("initializeProfile")(directory, settings);
  } PYCATCH;
}

QString IPluginGameWrapper::savegameExtension() const
{
  try {
    return this->get_override("savegameExtension")();
  } PYCATCH;
}

QString IPluginGameWrapper::savegameSEExtension() const
{
  try {
    return this->get_override("savegameSEExtension")();
  } PYCATCH;
}

bool IPluginGameWrapper::isInstalled() const
{
  try {
    return this->get_override("isInstalled")();
  } PYCATCH;
}

QIcon IPluginGameWrapper::gameIcon() const
{
  try {
    return this->get_override("gameIcon")();
  } PYCATCH;
}

QDir IPluginGameWrapper::gameDirectory() const
{
  try {
    return this->get_override("gameDirectory")();
  } PYCATCH;
}

QDir IPluginGameWrapper::dataDirectory() const
{
  try {
    return this->get_override("dataDirectory")();
  } PYCATCH;
}

void IPluginGameWrapper::setGamePath(const QString & path)
{
  try {
    this->get_override("setGamePath")(path);
  } PYCATCH;
}

QDir IPluginGameWrapper::documentsDirectory() const
{
  try {
    return this->get_override("documentsDirectory")();
  } PYCATCH;
}

QDir IPluginGameWrapper::savesDirectory() const
{
  try {
    return this->get_override("savesDirectory")();
  } PYCATCH;
}

QList<MOBase::ExecutableInfo> IPluginGameWrapper::executables() const
{
  try {
    return this->get_override("executables")();
  } PYCATCH;
}

QString IPluginGameWrapper::steamAPPId() const
{
  try {
    return this->get_override("steamAPPId")();
  } PYCATCH;
}

QStringList IPluginGameWrapper::primaryPlugins() const
{
  try {
    return this->get_override("primaryPlugins")();
  } PYCATCH;
}

QStringList IPluginGameWrapper::gameVariants() const
{
  try {
    return this->get_override("gameVariants")();
  } PYCATCH;
}

void IPluginGameWrapper::setGameVariant(const QString & variant)
{
  try {
    this->get_override("setGameVariant")(variant);
  } PYCATCH;
}

QString IPluginGameWrapper::binaryName() const
{
  try {
    return this->get_override("binaryName")();
  } PYCATCH;
}

QString IPluginGameWrapper::gameShortName() const
{
  try {
    return this->get_override("gameShortName")();
  } PYCATCH;
}

QStringList IPluginGameWrapper::validShortNames() const
{
  try {
    return this->get_override("validShortNames")();
  } PYCATCH;
}

QString IPluginGameWrapper::gameNexusName() const
{
  try {
    return this->get_override("gameNexusName")();
  } PYCATCH;
}

QStringList IPluginGameWrapper::iniFiles() const
{
  try {
    return this->get_override("iniFiles")();
  } PYCATCH;
}

QStringList IPluginGameWrapper::DLCPlugins() const
{
  try {
    return this->get_override("DLCPlugins")();
  } PYCATCH;
}

QStringList IPluginGameWrapper::CCPlugins() const
{
  try {
    return this->get_override("CCPlugins")();
  } PYCATCH;
}

IPluginGame::LoadOrderMechanism IPluginGameWrapper::loadOrderMechanism() const
{
  try {
    return this->get_override("loadorderMechanism")();
  } PYCATCH;
}

IPluginGame::SortMechanism IPluginGameWrapper::sortMechanism() const
{
  try {
    return this->get_override("sortMechanism")();
  } PYCATCH;
}

int IPluginGameWrapper::nexusModOrganizerID() const
{
  try {
    return this->get_override("nexusModOrganizerID")();
  } PYCATCH;
}

int IPluginGameWrapper::nexusGameID() const
{
  try {
    return this->get_override("nexusGameID")();
  } PYCATCH;
}

bool IPluginGameWrapper::looksValid(QDir const & dir) const
{
  try {
    return this->get_override("looksValid")(dir);
  } PYCATCH;
}

QString IPluginGameWrapper::gameVersion() const
{
  try {
    return this->get_override("gameVersion")();
  } PYCATCH;
}

QString IPluginGameWrapper::getLauncherName() const
{
  try {
    return this->get_override("getLauncherName")();
  } PYCATCH;
}

COMMON_I_PLUGIN_WRAPPER_DEFINITIONS(IPluginGameWrapper)

std::map<std::type_index, boost::any> IPluginGameWrapper::featureList() const
{
  qCritical("Calling unproxied method IPluginGameWrapper::featureList()");
  try {
    return this->get_override("_featureList")();
  } PYCATCH;
}
/// end IPluginGame Wrapper
/////////////////////////////////////
/// IPluginInstallerCustom Wrapper


COMMON_I_PLUGIN_WRAPPER_DEFINITIONS(IPluginInstallerCustomWrapper)

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


COMMON_I_PLUGIN_WRAPPER_DEFINITIONS(IPluginToolWrapper)

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

