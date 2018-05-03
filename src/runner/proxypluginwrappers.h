#ifndef PROXYPLUGINWRAPPERS_H
#define PROXYPLUGINWRAPPERS_H


#include <iplugindiagnose.h>
#include <ipluginfilemapper.h>
#include <iplugingame.h>
#include <iplugininstallersimple.h>
#include <iplugininstallercustom.h>
#include <ipluginmodpage.h>
#include <ipluginpreview.h>
#include <iplugintool.h>

#ifndef Q_MOC_RUN
#include <boost/python.hpp>
#endif


#define COMMON_I_PLUGIN_WRAPPER_DECLARATIONS public: \
virtual bool init(MOBase::IOrganizer *moInfo) override; \
virtual QString name() const override; \
virtual QString author() const override; \
virtual QString description() const override; \
virtual MOBase::VersionInfo version() const override; \
virtual bool isActive() const override; \
virtual QList<MOBase::PluginSetting> settings() const override;


class IPluginWrapper : public MOBase::IPlugin, public boost::python::wrapper<MOBase::IPlugin>
{
  Q_INTERFACES(MOBase::IPlugin)

  COMMON_I_PLUGIN_WRAPPER_DECLARATIONS
};


// Even though the base interface is not an IPlugin or QObject, this has to be because we have no way to pass Mod Organizer a plugin that implements multiple interfaces.
// QObject must be the first base class because moc assumes the first base class is a QObject
class IPluginDiagnoseWrapper : public QObject, public MOBase::IPluginDiagnose, public MOBase::IPlugin, public boost::python::wrapper<MOBase::IPluginDiagnose>
{
  Q_OBJECT
  Q_INTERFACES(MOBase::IPlugin MOBase::IPluginDiagnose)

public:
  virtual std::vector<unsigned int> activeProblems() const override;
  virtual QString shortDescription(unsigned int key) const override;
  virtual QString fullDescription(unsigned int key) const override;
  virtual bool hasGuidedFix(unsigned int key) const override;
  virtual void startGuidedFix(unsigned int key) const override;
  // Other functions exist, but shouldn't need wrapping as a default implementation exists
  // This was protected, but Python doesn't have that, so it needs making public
  virtual void invalidate();

  COMMON_I_PLUGIN_WRAPPER_DECLARATIONS
};


// Even though the base interface is not an IPlugin or QObject, this has to be because we have no way to pass Mod Organizer a plugin that implements multiple interfaces.
// QObject must be the first base class because moc assumes the first base class is a QObject
class IPluginFileMapperWrapper : public QObject, public MOBase::IPluginFileMapper, public MOBase::IPlugin, public boost::python::wrapper<MOBase::IPluginFileMapper>
{
  Q_OBJECT
  Q_INTERFACES(MOBase::IPlugin MOBase::IPluginFileMapper)

public:
  virtual MappingType mappings() const override;

  COMMON_I_PLUGIN_WRAPPER_DECLARATIONS
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
  virtual QStringList primarySources() const override;
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

  COMMON_I_PLUGIN_WRAPPER_DECLARATIONS

protected:
  // Apparently, Python developers interpret an underscore in a function name as it being protected
  virtual std::map<std::type_index, boost::any> featureList() const override;

  // Thankfully, the default implementation of the templated 'T *feature()' function should allow us to get away without overriding it.
};


class IPluginInstallerCustomWrapper : public MOBase::IPluginInstallerCustom, public boost::python::wrapper<MOBase::IPluginInstallerCustom>
{
  Q_OBJECT
  Q_INTERFACES(MOBase::IPlugin MOBase::IPluginInstaller MOBase::IPluginInstallerCustom)

  COMMON_I_PLUGIN_WRAPPER_DECLARATIONS
public:
  virtual unsigned int priority() const;
  virtual bool isManualInstaller() const;
  virtual bool isArchiveSupported(const MOBase::DirectoryTree &tree) const;
  virtual bool isArchiveSupported(const QString &archiveName) const;
  virtual std::set<QString> supportedExtensions() const;
  virtual EInstallResult install(MOBase::GuessedValue<QString> &modName, const QString &archiveName,
                                 const QString &version, int modID);
  virtual void setParentWidget(QWidget *parent);

};


class IPluginModPageWrapper : public MOBase::IPluginModPage, public boost::python::wrapper<MOBase::IPluginModPage>
{
  Q_OBJECT
  Q_INTERFACES(MOBase::IPlugin MOBase::IPluginModPage)

  COMMON_I_PLUGIN_WRAPPER_DECLARATIONS
public:
  virtual QString displayName() const override;
  virtual QIcon icon() const override;
  virtual QUrl pageURL() const override;
  virtual bool useIntegratedBrowser() const override;
  virtual bool handlesDownload(const QUrl &pageURL, const QUrl &downloadURL, MOBase::ModRepositoryFileInfo &fileInfo) const override;
  virtual void setParentWidget(QWidget *widget) override;
};


class IPluginPreviewWrapper : public MOBase::IPluginPreview, public boost::python::wrapper<MOBase::IPluginPreview>
{
  Q_OBJECT
  Q_INTERFACES(MOBase::IPlugin MOBase::IPluginPreview)

  COMMON_I_PLUGIN_WRAPPER_DECLARATIONS
public:
  virtual std::set<QString> supportedExtensions() const override;
  virtual QWidget *genFilePreview(const QString &fileName, const QSize &maxSize) const override;
};


class IPluginToolWrapper: public MOBase::IPluginTool, public boost::python::wrapper<MOBase::IPluginTool>
{
  Q_OBJECT
  Q_INTERFACES(MOBase::IPlugin MOBase::IPluginTool)

  COMMON_I_PLUGIN_WRAPPER_DECLARATIONS
public:
  virtual QString displayName() const;
  virtual QString tooltip() const;
  virtual QIcon icon() const;
  virtual void setParentWidget(QWidget *parent);

public slots:
  virtual void display() const;
};




#endif // PROXYPLUGINWRAPPERS_H
