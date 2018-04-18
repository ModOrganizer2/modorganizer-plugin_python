#ifndef UIBASEWRAPPERS_H
#define UIBASEWRAPPERS_H


#ifndef Q_MOC_RUN
#pragma warning (push, 0)
#include <boost/python.hpp>
#pragma warning (pop)
#endif

#include <QIcon>
#include <QString>
#include <QUrl>
#include <QDir>

#include "iplugingame.h"
#include <imoinfo.h>
#include <imodrepositorybridge.h>
#include <imodinterface.h>
#include <iinstallationmanager.h>
#include <idownloadmanager.h>
#include <ipluginlist.h>
#include <imodlist.h>
#include "error.h"
#include "gilock.h"

extern MOBase::IOrganizer *s_Organizer;

using MOBase::ModRepositoryFileInfo;

/**
 * @brief Wrapper class for the bridge to a mod repository. Awkward: This may be
 *  unnecessary but I didn't manage to figure out how to correctly connect python
 *  code to C++ signals
 */
class ModRepositoryBridgeWrapper : public QObject
{
  Q_OBJECT
public:

  ModRepositoryBridgeWrapper()
  : m_Wrapped(s_Organizer->createNexusBridge())
  {
  }

  ModRepositoryBridgeWrapper(MOBase::IModRepositoryBridge *wrapped)
    : m_Wrapped(wrapped)
  {
  }

  ~ModRepositoryBridgeWrapper()
  {
    delete m_Wrapped;
  }

  void requestDescription(QString gameName, int modID, QVariant userData)
    { m_Wrapped->requestDescription(gameName, modID, userData); }
  void requestFiles(QString gameName, int modID, QVariant userData)
    { m_Wrapped->requestFiles(gameName, modID, userData); }
  void requestFileInfo(QString gameName, int modID, int fileID, QVariant userData)
    { m_Wrapped->requestFileInfo(gameName, modID, fileID, userData); }
  void requestToggleEndorsement(QString gameName, int modID, bool endorse, QVariant userData)
    { m_Wrapped->requestToggleEndorsement(gameName, modID, endorse, userData); }

  void onFilesAvailable(boost::python::object callback) {
    m_FilesAvailableHandler = callback;
    connect(m_Wrapped, SIGNAL(filesAvailable(int,QVariant,const QList<ModRepositoryFileInfo>&)),
            this, SLOT(filesAvailable(int,QVariant,const QList<ModRepositoryFileInfo>&)),
            Qt::UniqueConnection);
  }

  void onDescriptionAvailable(boost::python::object callback) {
    m_DescriptionAvailableHandler = callback;
    connect(m_Wrapped, SIGNAL(descriptionAvailable(int,QVariant,QVariant)),
            this, SLOT(descriptionAvailable(int,QVariant,QVariant)),
            Qt::UniqueConnection);
  }

  void onFileInfoAvailable(boost::python::object callback) {
    m_FileInfoHandler = callback;
    connect(m_Wrapped, SIGNAL(fileInfoAvailable(int,int,QVariant,QVariant)),
            this, SLOT(fileInfoAvailable(int,int,QVariant,QVariant)),
            Qt::UniqueConnection);
  }

  void onEndorsementToggled(boost::python::object callback) {
    m_EndorsementToggledHandler = callback;
    connect(m_Wrapped, SIGNAL(endorsementToggled(int,QVariant,QVariant)),
            this, SLOT(endorsementToggled(int,QVariant,QVariant)),
            Qt::UniqueConnection);
  }

  void onRequestFailed(boost::python::object callback) {
    m_FailedHandler = callback;
    connect(m_Wrapped, SIGNAL(requestFailed(int,int,QVariant,QString)),
            this, SLOT(requestFailed(int,int,QVariant,QString)),
            Qt::UniqueConnection);
  }

private:

  Q_DISABLE_COPY(ModRepositoryBridgeWrapper)

private slots:

  void filesAvailable(int modID, QVariant userData, const QList<ModRepositoryFileInfo> &resultData)
  {
    if (m_FilesAvailableHandler.is_none()) {
      qCritical("no handler connected");
      return;
    }
    try {
      GILock lock;
      m_FilesAvailableHandler(modID, userData, resultData);
    } catch (const boost::python::error_already_set&) {
      reportPythonError();
    }
  }

  void descriptionAvailable(int modID, QVariant userData, const QVariant resultData)
  {
    try {
      if (m_DescriptionAvailableHandler.is_none()) {
        qCritical("no handler connected");
        return;
      }
      try {
        GILock lock;
        m_DescriptionAvailableHandler(modID, userData, resultData);
      } catch (const boost::python::error_already_set&) {
        reportPythonError();
      }
    } catch (const std::exception &e) {
      qCritical("failed to report event: %s", e.what());
    } catch (...) {
      qCritical("failed to report event");
    }
  }

  void fileInfoAvailable(int modID, int fileID, QVariant userData, const QVariant resultData) {
    try {
      if (m_FileInfoHandler.is_none()) {
        qCritical("no handler connected");
        return;
      }
      try {
        GILock lock;
        m_FileInfoHandler(modID, fileID, userData, resultData);
      } catch (const boost::python::error_already_set&) {
        reportPythonError();
      }
    } catch (const std::exception &e) {
      qCritical("failed to report event: %s", e.what());
    } catch (...) {
      qCritical("failed to report event");
    }
  }

  void endorsementToggled(int modID, QVariant userData, const QVariant resultData)
  {
    try {
      if (m_EndorsementToggledHandler.is_none()) {
        qCritical("no handler connected");
        return;
      }
      try {
        GILock lock;
        m_EndorsementToggledHandler(modID, userData, resultData);
      } catch (const boost::python::error_already_set&) {
        reportPythonError();
      }
    } catch (const std::exception &e) {
      qCritical("failed to report event: %s", e.what());
    } catch (...) {
      qCritical("failed to report event");
    }
  }

  void requestFailed(int modID, int fileID, QVariant userData, const QString &errorMessage)
  {
    try {
      GILock lock;
      m_FailedHandler(modID, fileID, userData, errorMessage);
    } catch (const boost::python::error_already_set&) {
      reportPythonError();
    }
  }

private:

  MOBase::IModRepositoryBridge *m_Wrapped;
  boost::python::object m_FilesAvailableHandler;
  boost::python::object m_DescriptionAvailableHandler;
  boost::python::object m_FileInfoHandler;
  boost::python::object m_EndorsementToggledHandler;
  boost::python::object m_FailedHandler;

};

struct IOrganizerWrapper : MOBase::IOrganizer,
                           boost::python::wrapper<MOBase::IOrganizer> {
  virtual MOBase::IModRepositoryBridge *createNexusBridge() const override
  {
    return this->get_override("createNexusBridge")();
  }
  virtual QString profileName() const override
  {
    return this->get_override("profileName")();
  }
  virtual QString profilePath() const override
  {
    return this->get_override("profilePath")();
  }
  virtual QString downloadsPath() const override
  {
    return this->get_override("downloadsPath")();
  }
  virtual QString overwritePath() const override
  {
    return this->get_override("overwritePath")();
  }
  virtual QString basePath() const // override
  {
	return this->get_override("basePath")();
  }
  virtual MOBase::VersionInfo appVersion() const override
  {
    return this->get_override("appVersion")();
  }
  virtual MOBase::IModInterface *getMod(const QString &name) const override
  {
    return this->get_override("getMod")(name);
  }
  virtual MOBase::IModInterface *
  createMod(MOBase::GuessedValue<QString> &name) override
  {
    return this->get_override("createMod")(name);
  }
  virtual bool removeMod(MOBase::IModInterface *mod) override
  {
    return this->get_override("removeMod")(mod);
  }
  virtual void modDataChanged(MOBase::IModInterface *mod) override
  {
    this->get_override("modDataChanged")(mod);
  }
  virtual QVariant pluginSetting(const QString &pluginName,
                                 const QString &key) const override
  {
    return this->get_override("pluginSetting")(pluginName, key).as<QVariant>();
  }
  virtual void setPluginSetting(const QString &pluginName, const QString &key,
                                const QVariant &value) override
  {
    this->get_override("setPluginSetting")(pluginName, key, value);
  }
  virtual QVariant persistent(const QString &pluginName, const QString &key,
                              const QVariant &def = QVariant()) const override
  {
    return this->get_override("persistent")(pluginName, key, def).as<QVariant>();
  }
  virtual void setPersistent(const QString &pluginName, const QString &key,
                             const QVariant &value, bool sync = true) override
  {
    this->get_override("setPersistent")(pluginName, key, value, sync);
  }
  virtual QString pluginDataPath() const override
  {
    return this->get_override("pluginDataPath")();
  }
  virtual MOBase::IModInterface *installMod(const QString &fileName,
                                            const QString &nameSuggestion
                                            = QString()) override
  {
    return this->get_override("installMod")(fileName, nameSuggestion);
  }
  virtual MOBase::IDownloadManager *downloadManager() const override
  {
    return this->get_override("downloadManager")();
  }
  virtual MOBase::IPluginList *pluginList() const override
  {
    return this->get_override("pluginList")();
  }
  virtual MOBase::IModList *modList() const override
  {
    return this->get_override("modList")();
  }
  virtual QString resolvePath(const QString &fileName) const override
  {
    return this->get_override("resolvePath")(fileName);
  }
  virtual QStringList
  listDirectories(const QString &directoryName) const override
  {
    return this->get_override("listDirectories")(directoryName);
  }
  virtual QStringList
  findFiles(const QString &path,
            const std::function<bool(const QString &)> &filter) const override
  {
    return this->get_override("findFiles")(path, filter);
  }
  virtual QStringList getFileOrigins(const QString &fileName) const override
  {
    return this->get_override("getFileOrigins")(fileName);
  }
  virtual QList<FileInfo> findFileInfos(
      const QString &path,
      const std::function<bool(const FileInfo &)> &filter) const override
  {
    return this->get_override("findFileInfos")(path, filter);
  }
  virtual HANDLE startApplication(const QString &executable,
                                  const QStringList &args = QStringList(),
                                  const QString &cwd      = "",
                                  const QString &profile = "") override
  {
    return reinterpret_cast<HANDLE>(this->get_override("startApplication")(executable, args, cwd, profile).as<unsigned long>());
  }
  virtual bool waitForApplication(HANDLE handle,
                                  LPDWORD exitCode = nullptr) const override
  {
    return this->get_override("waitForApplication")(reinterpret_cast<unsigned long>(handle), exitCode);
  }
  virtual void refreshModList(bool saveChanges = true) override
  {
    this->get_override("refreshModList")(saveChanges);
  }
  virtual bool
  onAboutToRun(const std::function<bool(const QString &)> &func) override
  {
    return this->get_override("onAboutToRun")(func);
  }
  virtual bool onFinishedRun(
      const std::function<void(const QString &, unsigned int)> &func) override
  {
    return this->get_override("onFinishedRun")(func);
  }
  virtual bool
  onModInstalled(const std::function<void(const QString &)> &func) override
  {
    return this->get_override("onModInstalled")(func);
  }
  virtual MOBase::IProfile *profile() const override
  {
    return this->get_override("profile")();
  }
  virtual MOBase::IPluginGame const *managedGame() const override
  {
    return this->get_override("managedGame")();
  }
  virtual QStringList modsSortedByProfilePriority() const override
  {
    return this->get_override("modsSortedByProfilePriority")();
  }
};

struct IProfileWrapper: MOBase::IProfile, boost::python::wrapper<MOBase::IProfile>
{
  virtual QString name() const override { return this->get_override("name")(); }
  virtual QString absolutePath() const override { return this->get_override("absolutePath")(); }
  virtual bool localSavesEnabled() const override { return this->get_override("localSavesEnabled")(); }
  virtual bool localSettingsEnabled() const override { return this->get_override("localSettingsEnabled")(); }
};

struct IDownloadManagerWrapper: MOBase::IDownloadManager, boost::python::wrapper<MOBase::IDownloadManager>
{
  virtual int startDownloadURLs(const QStringList &urls) { return this->get_override("startDownloadURLs")(urls); }
  virtual int startDownloadNexusFile(int modID, int fileID) { return this->get_override("startDownloadNexusFile")(modID, fileID); }
  virtual QString downloadPath(int id) { return this->get_override("downloadPath")(id); }
};

struct IModRepositoryBridgeWrapper: MOBase::IModRepositoryBridge, boost::python::wrapper<MOBase::IModRepositoryBridge>
{
  virtual void requestDescription(QString gameName, int modID, QVariant userData) { this->get_override("requestDescription")(gameName, modID, userData); }
  virtual void requestFiles(QString gameName, int modID, QVariant userData) { this->get_override("requestFiles")(gameName, modID, userData); }
  virtual void requestFileInfo(QString gameName, int modID, int fileID, QVariant userData) { this->get_override("requestFileInfo")(gameName, modID, fileID, userData); }
  virtual void requestDownloadURL(QString gameName, int modID, int fileID, QVariant userData) { this->get_override("requestDownloadURL")(gameName, modID, fileID, userData); }
  virtual void requestToggleEndorsement(QString gameName, int modID, bool endorse, QVariant userData) { this->get_override("requestToggleEndorsement")(gameName, modID, endorse, userData); }
};

struct IInstallationManagerWrapper: MOBase::IInstallationManager, boost::python::wrapper<MOBase::IInstallationManager>
{
  virtual QString extractFile(const QString &fileName) { return this->get_override("extractFile")(fileName); }
  virtual QStringList extractFiles(const QStringList &files, bool flatten) { return this->get_override("extractFiles")(files, flatten); }
  virtual MOBase::IPluginInstaller::EInstallResult installArchive(MOBase::GuessedValue<QString> &modName, const QString &archiveFile) { return this->get_override("installArchive")(modName, archiveFile); }
  virtual void setURL(QString const &url) { this->get_override("setURL")(url); }
};

struct IModInterfaceWrapper: MOBase::IModInterface, boost::python::wrapper<MOBase::IModInterface>
{
  virtual QString name() const override { return this->get_override("name")();  }
  virtual QString absolutePath() const override { return this->get_override("absolutePath")(); }
  virtual void setVersion(const MOBase::VersionInfo &version) override { this->get_override("setVersion")(version); }
  virtual void setNewestVersion(const MOBase::VersionInfo &version) override { this->get_override("setNewestVersion")(version); }
  virtual void setIsEndorsed(bool endorsed) override { this->get_override("setIsEndorsed")(endorsed); }
  virtual void setNexusID(int nexusID) override { this->get_override("setNexusID")(nexusID); }
  virtual void setInstallationFile(const QString &fileName) override { this->get_override("setInstallationFile")(fileName); }
  virtual void addNexusCategory(int categoryID) override { this->get_override("addNexusCategory")(categoryID); }
  virtual bool setName(const QString &name) override { return this->get_override("setName")(name); }
  virtual bool remove() override { return this->get_override("remove")(); }
  virtual void addCategory(const QString &categoryName) override { this->get_override("addCategory")(categoryName); }
  virtual bool removeCategory(const QString &categoryName) override { return this->get_override("removeCategory")(categoryName); }
  virtual QStringList categories() override { return this->get_override("categories")(); }
};


struct IPluginListWrapper: MOBase::IPluginList, boost::python::wrapper<MOBase::IPluginList> {
  virtual PluginStates state(const QString &name) const { return this->get_override("state")(name); }
  virtual int priority(const QString &name) const { return this->get_override("priority")(name); }
  virtual int loadOrder(const QString &name) const { return this->get_override("loadOrder")(name); }
  virtual bool isMaster(const QString &name) const { return this->get_override("isMaster")(name); }
  virtual QStringList masters(const QString &name) const { return this->get_override("masters")(name); }
  virtual QString origin(const QString &name) const { return this->get_override("origin")(name); }
  virtual bool onRefreshed(const std::function<void ()> &callback) { return this->get_override("onRefreshed")(callback); }
  virtual bool onPluginMoved(const std::function<void (const QString &, int, int)> &callback) { return this->get_override("onPluginMoved")(callback); }
  virtual bool onPluginStateChanged(const std::function<void (const QString &, PluginStates)> &callback) override { return this->get_override("onPluginStateChanged")(callback); }
  virtual QStringList pluginNames() const override { return this->get_override("pluginNames")(); }
  virtual void setState(const QString &name, PluginStates state) override { this->get_override("setState")(name, state); }
  virtual void setLoadOrder(const QStringList &pluginList) override { this->get_override("setLoadOrder")(pluginList); }
};


struct IModListWrapper: MOBase::IModList, boost::python::wrapper<MOBase::IModList> {
  virtual QString displayName(const QString &internalName) const override { return this->get_override("displayName")(internalName); }
  virtual QStringList allMods() const override { return this->get_override("allMods")(); }
  virtual ModStates state(const QString &name) const override { return this->get_override("state")(name); }
  virtual int priority(const QString &name) const override { return this->get_override("priority")(name); }
  virtual bool setActive(const QString &name, bool active) override { return this->get_override("setActive")(name, active); }
  virtual bool setPriority(const QString &name, int newPriority) override { return this->get_override("setPriority")(name, newPriority); }
  virtual bool onModStateChanged(const std::function<void (const QString &, ModStates)> &func) override { return this->get_override("onModStateChanged")(func); }
  virtual bool onModMoved(const std::function<void (const QString &, int, int)> &func) override { return this->get_override("onModMoved")(func); }
};


struct IPluginGameWrapper: MOBase::IPluginGame, boost::python::wrapper<MOBase::IPluginGame> {
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

#endif // UIBASEWRAPPERS_H
