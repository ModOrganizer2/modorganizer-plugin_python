#define HAVE_ROUND
#include "proxypluginwrappers.h"
#include <utility.h>
#include "error.h"
#include "gilock.h"

namespace bpy = boost::python;

using namespace MOBase;


#define PYCATCH catch (const bpy::error_already_set &) { reportPythonError(); throw MyException("unhandled exception"); }\
                catch (...) { throw MyException("An unknown exception was thrown in python code"); }


/////////////////////////////
/// IPluginTool Wrapper


bool IPluginToolWrapper::init(MOBase::IOrganizer *moInfo)
{
  try {
    return this->get_override("init")(bpy::ptr(moInfo));
  } PYCATCH;
}

QString IPluginToolWrapper::name() const
{
  try {
    return this->get_override("name")();
  } PYCATCH;
}

QString IPluginToolWrapper::author() const
{
  try {
    return this->get_override("author")();
  } PYCATCH;
}

QString IPluginToolWrapper::description() const
{
  try {
    return this->get_override("description")();
  } PYCATCH;
}

MOBase::VersionInfo IPluginToolWrapper::version() const
{
  try {
    return this->get_override("version")();
  } PYCATCH;
}

bool IPluginToolWrapper::isActive() const
{
  try {
    return this->get_override("isActive")();
  } PYCATCH;
}

QList<MOBase::PluginSetting> IPluginToolWrapper::settings() const
{
  try {
    return this->get_override("settings")();
  } PYCATCH;
}

QString IPluginToolWrapper::displayName() const
{
  try {
    return this->get_override("displayName")();
  } PYCATCH;
}

QString IPluginToolWrapper::tooltip() const
{
  try {
    return this->get_override("tooltip")();
  } PYCATCH;
}

QIcon IPluginToolWrapper::icon() const
{
  try {
    return this->get_override("icon")();
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
/////////////////////////////////////
/// IPluginInstallerCustom Wrapper


bool IPluginInstallerCustomWrapper::init(MOBase::IOrganizer *moInfo)
{
  try {
    return this->get_override("init")(bpy::ptr(moInfo));
  } PYCATCH;
}

QString IPluginInstallerCustomWrapper::name() const
{
  try {
    return this->get_override("name")();
  } PYCATCH;
}

QString IPluginInstallerCustomWrapper::author() const
{
  try {
    return this->get_override("author")();
  } PYCATCH;
}

QString IPluginInstallerCustomWrapper::description() const
{
  try {
    return this->get_override("description")();
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
    return this->get_override("settings")();
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
    return this->get_override("supportedExtensions")();
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


