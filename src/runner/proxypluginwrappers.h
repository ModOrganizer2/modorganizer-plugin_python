#ifndef PROXYPLUGINWRAPPERS_H
#define PROXYPLUGINWRAPPERS_H


#include <iplugintool.h>
#include <iplugininstallersimple.h>
#include <iplugininstallercustom.h>
#include <iplugindiagnose.h>

#ifndef Q_MOC_RUN
#include <boost/python.hpp>
#endif


class IPluginWrapper : public boost::python::wrapper<MOBase::IPlugin>
{
public:
  virtual bool init(MOBase::IOrganizer *moInfo);
  virtual QString name() const;
  virtual QString author() const;
  virtual QString description() const;
  virtual MOBase::VersionInfo version() const;
  virtual bool isActive() const;
  virtual QList<MOBase::PluginSetting> settings() const;
};


class IPluginToolWrapper: public MOBase::IPluginTool, public boost::python::wrapper<MOBase::IPluginTool>
{
  Q_OBJECT
  Q_INTERFACES(MOBase::IPlugin MOBase::IPluginTool)

public:
  virtual bool init(MOBase::IOrganizer *moInfo);
  virtual QString name() const;
  virtual QString author() const;
  virtual QString description() const;
  virtual MOBase::VersionInfo version() const;
  virtual bool isActive() const;
  virtual QList<MOBase::PluginSetting> settings() const;

  virtual QString displayName() const;
  virtual QString tooltip() const;
  virtual QIcon icon() const;
  virtual void setParentWidget(QWidget *parent);

public slots:
  virtual void display() const;
};


class IPluginInstallerCustomWrapper: public MOBase::IPluginInstallerCustom, public boost::python::wrapper<MOBase::IPluginInstallerCustom>
{
  Q_OBJECT
  Q_INTERFACES(MOBase::IPlugin MOBase::IPluginInstaller MOBase::IPluginInstallerCustom)

public:
  virtual bool init(MOBase::IOrganizer *moInfo);
  virtual QString name() const;
  virtual QString author() const;
  virtual QString description() const;
  virtual MOBase::VersionInfo version() const;
  virtual bool isActive() const;
  virtual QList<MOBase::PluginSetting> settings() const;

  virtual unsigned int priority() const;
  virtual bool isManualInstaller() const;
  virtual bool isArchiveSupported(const MOBase::DirectoryTree &tree) const;
  virtual bool isArchiveSupported(const QString &archiveName) const;
  virtual std::set<QString> supportedExtensions() const;
  virtual EInstallResult install(MOBase::GuessedValue<QString> &modName, const QString &archiveName,
                                 const QString &version, int modID);
  virtual void setParentWidget(QWidget *parent);

};




#endif // PROXYPLUGINWRAPPERS_H
