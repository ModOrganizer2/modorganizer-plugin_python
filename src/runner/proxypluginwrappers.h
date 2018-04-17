#ifndef PROXYPLUGINWRAPPERS_H
#define PROXYPLUGINWRAPPERS_H


#include <iplugintool.h>
#include <iplugingame.h>
#include <iplugininstallersimple.h>
#include <iplugininstallercustom.h>

#ifndef Q_MOC_RUN
#include <boost/python.hpp>
#endif


class IPluginWrapper : public MOBase::IPlugin, public boost::python::wrapper<MOBase::IPlugin>
{
  Q_INTERFACES(MOBase::IPlugin)

public:
  virtual bool init(MOBase::IOrganizer *moInfo);
  virtual QString name() const;
  virtual QString author() const;
  virtual QString description() const;
  virtual MOBase::VersionInfo version() const;
  virtual bool isActive() const;
  virtual QList<MOBase::PluginSetting> settings() const;
};


class IPluginGameWrapper : public MOBase::IPluginGame, public boost::python::wrapper<MOBase::IPluginGame> {
  Q_OBJECT
  Q_INTERFACES(MOBase::IPlugin MOBase::IPluginGame)

public:
  virtual QString gameName() const override { return this->get_override("gameName")(); }
  virtual void initializeProfile(const QDir &directory, ProfileSettings settings) const override {
    this->get_override("initializeProfile")(directory, settings);
  }
  virtual QString savegameExtension() const override { return this->get_override("savegameExtension")(); }
  virtual QString savegameSEExtension() const override { return this->get_override("savegameSEExtension")(); }
  virtual bool isInstalled() const override { return this->get_override("isInstalled")(); }
  virtual QIcon gameIcon() const override { return this->get_override("gameIcon")(); }
  virtual QDir gameDirectory() const override { return this->get_override("gameDirectory")(); }
  virtual QDir dataDirectory() const override { return this->get_override("dataDirectory")(); }
  virtual void setGamePath(const QString &path) override { this->get_override("setGamePath")(path); }
  virtual QDir documentsDirectory() const override { return this->get_override("documentsDirectory")(); }
  virtual QDir savesDirectory() const override { return this->get_override("savesDirectory")(); }
  virtual QList<MOBase::ExecutableInfo> executables() const override { return this->get_override("executables")(); }
  virtual QString steamAPPId() const override { return this->get_override("steamAPPId")(); }
  virtual QStringList primaryPlugins() const override { return this->get_override("primaryPlugins")(); }
  virtual QStringList gameVariants() const override { return this->get_override("gameVariants")(); }
  virtual void setGameVariant(const QString &variant) override { this->get_override("setGameVariant")(variant); }
  virtual QString binaryName() const override { return this->get_override("binaryName")(); }
  virtual QString gameShortName() const override { return this->get_override("gameShortName")(); }
  virtual QStringList validShortNames() const override { return this->get_override("validShortNames")(); }
  virtual QString gameNexusName() const override { return this->get_override("gameNexusName")(); }
  virtual QStringList iniFiles() const override { return this->get_override("iniFiles")(); }
  virtual QStringList DLCPlugins() const override { return this->get_override("DLCPlugins")(); }
  virtual QStringList CCPlugins() const override { return this->get_override("CCPlugins")(); }
  virtual LoadOrderMechanism loadOrderMechanism() const override { return this->get_override("loadorderMechanism")(); }
  virtual SortMechanism sortMechanism() const override { return this->get_override("sortMechanism")(); }
  virtual int nexusModOrganizerID() const override { return this->get_override("nexusModOrganizerID")(); }
  virtual int nexusGameID() const override { return this->get_override("nexusGameID")(); }
  virtual bool looksValid(QDir const &dir) const override { return this->get_override("looksValid")(dir); }
  virtual QString gameVersion() const override { return this->get_override("gameVersion")(); }
  virtual QString getLauncherName() const override { return this->get_override("getLauncherName")(); }

  //Plugin interface. Could this bit be implemented just once?
  virtual bool init(MOBase::IOrganizer *moInfo) override { return this->get_override("init")(moInfo); }
  virtual QString name() const override { return this->get_override("name")(); }
  virtual QString author() const override { return this->get_override("author")(); }
  virtual QString description() const override { return this->get_override("description")(); }
  virtual MOBase::VersionInfo version() const override { return this->get_override("version")(); }
  virtual bool isActive() const override { return this->get_override("isActive")(); }
  virtual QList<MOBase::PluginSetting> settings() const override { return this->get_override("settings")(); }

protected:

  virtual std::map<std::type_index, boost::any> featureList() const override { return this->get_override("featureList")(); }

};


class IPluginInstallerCustomWrapper : public MOBase::IPluginInstallerCustom, public boost::python::wrapper<MOBase::IPluginInstallerCustom>
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




#endif // PROXYPLUGINWRAPPERS_H
