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
  virtual QString gameName() const override;
  virtual void initializeProfile(const QDir &directory, ProfileSettings settings) const override;
  virtual QString savegameExtension() const override;
  virtual QString savegameSEExtension() const override;
  virtual bool isInstalled() const override;
  virtual QIcon gameIcon() const override;
  virtual QDir gameDirectory() const override;
  virtual QDir dataDirectory() const override;
  virtual void setGamePath(const QString &path) override;
  virtual QDir documentsDirectory() const override;
  virtual QDir savesDirectory() const override;
  virtual QList<MOBase::ExecutableInfo> executables() const override;
  virtual QString steamAPPId() const override;
  virtual QStringList primaryPlugins() const override;
  virtual QStringList gameVariants() const override;
  virtual void setGameVariant(const QString &variant) override;
  virtual QString binaryName() const override;
  virtual QString gameShortName() const override;
  virtual QStringList validShortNames() const override;
  virtual QString gameNexusName() const override;
  virtual QStringList iniFiles() const override;
  virtual QStringList DLCPlugins() const override;
  virtual QStringList CCPlugins() const override;
  virtual LoadOrderMechanism loadOrderMechanism() const override;
  virtual SortMechanism sortMechanism() const override;
  virtual int nexusModOrganizerID() const override;
  virtual int nexusGameID() const override;
  virtual bool looksValid(QDir const &dir) const override;
  virtual QString gameVersion() const override;
  virtual QString getLauncherName() const override;

  //Plugin interface. Could this bit be implemented just once?
  virtual bool init(MOBase::IOrganizer *moInfo) override;
  virtual QString name() const override;
  virtual QString author() const override;
  virtual QString description() const override;
  virtual MOBase::VersionInfo version() const override;
  virtual bool isActive() const override;
  virtual QList<MOBase::PluginSetting> settings() const override;

protected:

  // TODO: implementing converters for this is required as otherwise Mod Organizer will crash on load when the game is being managed by this plugin
  // Apparently, Python developers interpret an underscore in a function name as it being protected
  virtual std::map<std::type_index, boost::any> featureList() const override;

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
