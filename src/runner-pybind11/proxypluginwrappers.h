#ifndef PROXYPLUGINWRAPPERS_H
#define PROXYPLUGINWRAPPERS_H


#include <iplugin.h>
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
#include <boost/preprocessor/control/expr_if.hpp>
#endif

// The wrapper for IPluginGame cannot override requirements or enabledByDefault since they're final,
// so we need to be able to exclude the declarations.
#define COMMON_I_PLUGIN_WRAPPER_DECLARATIONS_IMPL(include_requirements) \
public: \
virtual bool init(MOBase::IOrganizer *moInfo) override; \
virtual QString name() const override; \
virtual QString localizedName() const override; \
virtual QString master() const override; \
virtual QString author() const override; \
virtual QString description() const override; \
virtual MOBase::VersionInfo version() const override; \
virtual QList<MOBase::PluginSetting> settings() const override; \
QString localizedName_Default() const; \
QString master_Default() const; \
BOOST_PP_EXPR_IF(include_requirements, \
  virtual std::vector<std::shared_ptr<const MOBase::IPluginRequirement>> requirements() const override; \
  std::vector<std::shared_ptr<const MOBase::IPluginRequirement>> requirements_Default() const; \
  virtual bool enabledByDefault() const override; \
  bool enabledByDefault_Default() const;)

#define COMMON_I_PLUGIN_WRAPPER_DECLARATIONS COMMON_I_PLUGIN_WRAPPER_DECLARATIONS_IMPL(1)

// Even though the base interface is not a QObject, this has to be because we have no way to pass Mod Organizer a plugin that implements multiple interfaces.
// QObject must be the first base class because moc assumes the first base class is a QObject
class IPluginWrapper : public QObject, public MOBase::IPlugin, public boost::python::wrapper<MOBase::IPlugin>
{
  Q_OBJECT
  Q_INTERFACES(MOBase::IPlugin)

  COMMON_I_PLUGIN_WRAPPER_DECLARATIONS
public:
  static constexpr const char* className = "IPluginWrapper";
  using boost::python::wrapper<MOBase::IPlugin>::get_override;
};


// Even though the base interface is not an IPlugin or QObject, this has to be because we have no way to pass Mod Organizer a plugin that implements multiple interfaces.
// QObject must be the first base class because moc assumes the first base class is a QObject
class IPluginDiagnoseWrapper : public QObject, public MOBase::IPluginDiagnose, public MOBase::IPlugin, public boost::python::wrapper<MOBase::IPluginDiagnose>
{
  Q_OBJECT
  Q_INTERFACES(MOBase::IPlugin MOBase::IPluginDiagnose)

public:
  static constexpr const char* className = "IPluginDiagnoseWrapper";
  using boost::python::wrapper<MOBase::IPluginDiagnose>::get_override;

  // Bring in public scope:
  using IPluginDiagnose::invalidate;

  virtual std::vector<unsigned int> activeProblems() const override;
  virtual QString shortDescription(unsigned int key) const override;
  virtual QString fullDescription(unsigned int key) const override;
  virtual bool hasGuidedFix(unsigned int key) const override;
  virtual void startGuidedFix(unsigned int key) const override;

  COMMON_I_PLUGIN_WRAPPER_DECLARATIONS
};


// Even though the base interface is not an IPlugin or QObject, this has to be because we have no way to pass Mod Organizer a plugin that implements multiple interfaces.
// QObject must be the first base class because moc assumes the first base class is a QObject
class IPluginFileMapperWrapper : public QObject, public MOBase::IPluginFileMapper, public MOBase::IPlugin, public boost::python::wrapper<MOBase::IPluginFileMapper>
{
  Q_OBJECT
  Q_INTERFACES(MOBase::IPlugin MOBase::IPluginFileMapper)

public:
  static constexpr const char* className = "IPluginFileMapperWrapper";
  using boost::python::wrapper<MOBase::IPluginFileMapper>::get_override;

  virtual MappingType mappings() const override;

  COMMON_I_PLUGIN_WRAPPER_DECLARATIONS
};


class IPluginGameWrapper : public MOBase::IPluginGame, public boost::python::wrapper<MOBase::IPluginGame> {
  Q_OBJECT
  Q_INTERFACES(MOBase::IPlugin MOBase::IPluginGame)

public:
  static constexpr const char* className = "IPluginGameWrapper";
  using boost::python::wrapper<MOBase::IPluginGame>::get_override;

  virtual void detectGame() override;
  virtual QString gameName() const override;
  virtual void initializeProfile(const QDir &directory, ProfileSettings settings) const override;
  virtual std::vector<std::shared_ptr<const MOBase::ISaveGame>> listSaves(QDir folder) const override;
  virtual bool isInstalled() const override;
  virtual QIcon gameIcon() const override;
  virtual QDir gameDirectory() const override;
  virtual QDir dataDirectory() const override;
  virtual void setGamePath(const QString &path) override;
  virtual QDir documentsDirectory() const override;
  virtual QDir savesDirectory() const override;
  virtual QList<MOBase::ExecutableInfo> executables() const override;
  virtual QList<MOBase::ExecutableForcedLoadSetting> executableForcedLoads() const override;
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

  COMMON_I_PLUGIN_WRAPPER_DECLARATIONS_IMPL(0)

protected:
  // Apparently, Python developers interpret an underscore in a function name as it being protected
  virtual std::map<std::type_index, std::any> featureList() const override;

  // Thankfully, the default implementation of the templated 'T *feature()' function should allow us to get away without overriding it.
};


#define COMMON_I_PLUGIN_INSTALLER_WRAPPER_DECLARATIONS public: \
using IPluginInstaller::parentWidget; \
using IPluginInstaller::manager; \
virtual unsigned int priority() const override; \
virtual bool isManualInstaller() const override; \
virtual void onInstallationStart(QString const& archive, bool reinstallation, MOBase::IModInterface* currentMod) override; \
void onInstallationStart_Default(QString const& archive, bool reinstallation, MOBase::IModInterface* currentMod) { \
  return IPluginInstaller::onInstallationStart(archive, reinstallation, currentMod); } \
virtual void onInstallationEnd(EInstallResult result, MOBase::IModInterface* newMod) override; \
void onInstallationEnd_Default(EInstallResult result, MOBase::IModInterface* newMod) { \
  return IPluginInstaller::onInstallationEnd(result, newMod); } \
virtual bool isArchiveSupported(std::shared_ptr<const MOBase::IFileTree> tree) const override;


class IPluginInstallerSimpleWrapper : public MOBase::IPluginInstallerSimple, public boost::python::wrapper<MOBase::IPluginInstallerSimple>
{
  Q_OBJECT
  Q_INTERFACES(MOBase::IPlugin MOBase::IPluginInstaller MOBase::IPluginInstallerSimple)

  COMMON_I_PLUGIN_WRAPPER_DECLARATIONS
  COMMON_I_PLUGIN_INSTALLER_WRAPPER_DECLARATIONS

public:
  static constexpr const char* className = "IPluginInstallerSimpleWrapper";
  using boost::python::wrapper<MOBase::IPluginInstallerSimple>::get_override;

  virtual EInstallResult install(MOBase::GuessedValue<QString>& modName, std::shared_ptr<MOBase::IFileTree>& tree,
    QString& version, int& nexusID) override;
};

class IPluginInstallerCustomWrapper : public MOBase::IPluginInstallerCustom, public boost::python::wrapper<MOBase::IPluginInstallerCustom>
{
  Q_OBJECT
  Q_INTERFACES(MOBase::IPlugin MOBase::IPluginInstaller MOBase::IPluginInstallerCustom)

  COMMON_I_PLUGIN_WRAPPER_DECLARATIONS

  COMMON_I_PLUGIN_INSTALLER_WRAPPER_DECLARATIONS

public:
  static constexpr const char* className = "IPluginInstallerCustomWrapper";
  using boost::python::wrapper<MOBase::IPluginInstallerCustom>::get_override;

  virtual bool isArchiveSupported(const QString &archiveName) const override;
  virtual std::set<QString> supportedExtensions() const override;
  virtual EInstallResult install(MOBase::GuessedValue<QString> &modName, QString gameName, const QString &archiveName,
                                 const QString &version, int modID) override;
};


class IPluginModPageWrapper : public MOBase::IPluginModPage, public boost::python::wrapper<MOBase::IPluginModPage>
{
  Q_OBJECT
  Q_INTERFACES(MOBase::IPlugin MOBase::IPluginModPage)

  COMMON_I_PLUGIN_WRAPPER_DECLARATIONS
public:
  static constexpr const char* className = "IPluginModPageWrapper";
  using boost::python::wrapper<MOBase::IPluginModPage>::get_override;

  // Bring in public scope:
  using IPluginModPage::parentWidget;

  virtual QString displayName() const override;
  virtual QIcon icon() const override;
  virtual QUrl pageURL() const override;
  virtual bool useIntegratedBrowser() const override;
  virtual bool handlesDownload(const QUrl &pageURL, const QUrl &downloadURL, MOBase::ModRepositoryFileInfo &fileInfo) const override;
  virtual void setParentWidget(QWidget *widget) override;

  void setParentWidget_Default(QWidget* parent) {
    IPluginModPage::setParentWidget(parent);
  }
};


class IPluginPreviewWrapper : public MOBase::IPluginPreview, public boost::python::wrapper<MOBase::IPluginPreview>
{
  Q_OBJECT
  Q_INTERFACES(MOBase::IPlugin MOBase::IPluginPreview)

  COMMON_I_PLUGIN_WRAPPER_DECLARATIONS
public:
  static constexpr const char* className = "IPluginPreviewWrapper";
  using boost::python::wrapper<MOBase::IPluginPreview>::get_override;

  virtual std::set<QString> supportedExtensions() const override;
  virtual QWidget *genFilePreview(const QString &fileName, const QSize &maxSize) const override;
};


class IPluginToolWrapper: public MOBase::IPluginTool, public boost::python::wrapper<MOBase::IPluginTool>
{
  Q_OBJECT
  Q_INTERFACES(MOBase::IPlugin MOBase::IPluginTool)

  COMMON_I_PLUGIN_WRAPPER_DECLARATIONS
public:
  static constexpr const char* className = "IPluginToolWrapper";
  using boost::python::wrapper<MOBase::IPluginTool>::get_override;

  // Bring in public scope:
  using IPluginTool::parentWidget;

  virtual QString displayName() const override;
  virtual QString tooltip() const override;
  virtual QIcon icon() const override;
  virtual void setParentWidget(QWidget *parent) override;

  void setParentWidget_Default(QWidget* parent) {
    IPluginTool::setParentWidget(parent);
  }

public Q_SLOTS:
  virtual void display() const override;
};




#endif // PROXYPLUGINWRAPPERS_H
