#ifndef UIBASEWRAPPERS_H
#define UIBASEWRAPPERS_H


#ifndef Q_MOC_RUN
#pragma warning (push, 0)
#include <boost/python.hpp>
#pragma warning (pop)
#endif

#include <QString>

#include <imoinfo.h>
#include <igameinfo.h>
#include <imodrepositorybridge.h>
#include <imodinterface.h>
#include <iinstallationmanager.h>
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

  void requestDescription(int modID, QVariant userData)
    { m_Wrapped->requestDescription(modID, userData); }
  void requestFiles(int modID, QVariant userData)
    { m_Wrapped->requestFiles(modID, userData); }
  void requestFileInfo(int modID, int fileID, QVariant userData)
    { m_Wrapped->requestFileInfo(modID, fileID, userData); }
  void requestToggleEndorsement(int modID, bool endorse, QVariant userData)
    { m_Wrapped->requestToggleEndorsement(modID, endorse, userData); }

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

struct IOrganizerWrapper: MOBase::IOrganizer, boost::python::wrapper<MOBase::IOrganizer>
{
  virtual MOBase::IGameInfo &gameInfo() const {
    MOBase::IGameInfo *result = this->get_override("gameInfo")();
    return *result;
  }
  virtual MOBase::IModRepositoryBridge *createNexusBridge() const { return this->get_override("createNexusBridge")(); }
  virtual QString profileName() const { return this->get_override("profileName")(); }
  virtual QString profilePath() const { return this->get_override("profilePath")(); }
  virtual QString downloadsPath() const { return this->get_override("downloadsPath")(); }
  virtual QString overwritePath() const { return this->get_override("overwritePath")(); }
  virtual MOBase::VersionInfo appVersion() const { return this->get_override("appVersion")(); }
  virtual MOBase::IModInterface *getMod(const QString &name) { return this->get_override("getMod")(name); }
  virtual MOBase::IModInterface *createMod(MOBase::GuessedValue<QString> &name) { return this->get_override("createMod")(name); }
  virtual bool removeMod(MOBase::IModInterface *mod) { return this->get_override("removeMod")(mod); }
  virtual void modDataChanged(MOBase::IModInterface *mod) { this->get_override("modDataChanged")(mod); }
  virtual QVariant pluginSetting(const QString &pluginName, const QString &key) const { return this->get_override("pluginSetting")(pluginName, key); }
  virtual void setPluginSetting(const QString &pluginName, const QString &key, const QVariant &value) { this->get_override("setPluginSetting")(pluginName, key, value); }
  virtual QVariant persistent(const QString &pluginName, const QString &key, const QVariant &def = QVariant()) const { return this->get_override("persistent")(pluginName, key, def); }
  virtual void setPersistent(const QString &pluginName, const QString &key, const QVariant &value, bool sync = true) { this->get_override("setPersistent")(pluginName, key, value, sync); }
  virtual QString pluginDataPath() const { return this->get_override("pluginDataPath")(); }
  virtual MOBase::IModInterface *installMod(const QString &fileName) { return this->get_override("installMod")(fileName); }
  virtual MOBase::IDownloadManager *downloadManager() { return this->get_override("downloadManager")(); }
  virtual MOBase::IPluginList *pluginList() { return this->get_override("pluginList")(); }
  virtual MOBase::IModList *modList() { return this->get_override("modList")(); }
  virtual QString resolvePath(const QString &fileName) const { return this->get_override("resolvePath")(fileName); }
  virtual QStringList listDirectories(const QString &directoryName) const { return this->get_override("listDirectories")(directoryName); }
  virtual QStringList findFiles(const QString &path, const std::function<bool(const QString&)> &filter) const { return this->get_override("findFiles")(path, filter); }
  virtual QStringList getFileOrigins(const QString &fileName) const { return this->get_override("getFileOrigin")(fileName); }
  virtual QList<FileInfo> findFileInfos(const QString &path, const std::function<bool(const FileInfo&)> &filter) const { return this->get_override("findFileInfos")(path, filter); }
  virtual HANDLE startApplication(const QString &executable, const QStringList &args = QStringList(), const QString &cwd = "", const QString &profile = "") { return this->get_override("startApplication")(executable, args, cwd, profile); }
  virtual bool waitForApplication(HANDLE handle, LPDWORD exitCode = NULL) const { return this->get_override("waitForApplication")(handle, exitCode); }
  virtual void refreshModList(bool saveChanges = true) { this->get_override("refreshModList")(saveChanges); }
  virtual bool onAboutToRun(const std::function<bool(const QString&)> &func) { return this->get_override("onAboutToRun")(func); }
  virtual bool onModInstalled(const std::function<void(const QString&)> &func) { return this->get_override("onModInstalled")(func); }
};

struct IDownloadManagerWrapper: MOBase::IDownloadManager, boost::python::wrapper<MOBase::IDownloadManager>
{
  virtual int startDownloadURLs(const QStringList &urls) { return this->get_override("downloadURLs")(urls); }
  virtual int startDownloadNexusFile(int modID, int fileID) { return this->get_override("downloadNexusFile")(modID, fileID); }
  virtual QString downloadPath(int id) { return this->get_override("downloadPath")(id); }
};

struct IModRepositoryBridgeWrapper: MOBase::IModRepositoryBridge, boost::python::wrapper<MOBase::IModRepositoryBridge>
{
  virtual void requestDescription(int modID, QVariant userData) { this->get_override("requestDescription")(modID, userData); }
  virtual void requestFiles(int modID, QVariant userData) { this->get_override("requestFiles")(modID, userData); }
  virtual void requestFileInfo(int modID, int fileID, QVariant userData) { this->get_override("requestFileInfo")(modID, fileID, userData); }
  virtual void requestDownloadURL(int modID, int fileID, QVariant userData) { this->get_override("requestDownloadURL")(modID, fileID, userData); }
  virtual void requestToggleEndorsement(int modID, bool endorse, QVariant userData) { this->get_override("requestToggleEndorsement")(modID, endorse, userData); }
};

struct IInstallationManagerWrapper: MOBase::IInstallationManager, boost::python::wrapper<MOBase::IInstallationManager>
{
  virtual QString extractFile(const QString &fileName) { return this->get_override("extractFile")(fileName); }
  virtual QStringList extractFiles(const QStringList &files, bool flatten) { return this->get_override("extractFiles")(files, flatten); }
  virtual MOBase::IPluginInstaller::EInstallResult installArchive(MOBase::GuessedValue<QString> &modName, const QString &archiveFile) { return this->get_override("installArchive")(modName, archiveFile); }
};

struct IGameInfoWrapper: MOBase::IGameInfo, boost::python::wrapper<MOBase::IGameInfo>
{
  virtual Type type() const { return this->get_override("type")(); }
  virtual QString path() const { return this->get_override("path")(); }
  virtual QString binaryName() const { return this->get_override("binaryName")(); }
};

struct IModInterfaceWrapper: MOBase::IModInterface, boost::python::wrapper<MOBase::IModInterface>
{
  virtual QString name() const { return this->get_override("name")();  }
  virtual QString absolutePath() const { return this->get_override("absolutePath")(); }
  virtual void setVersion(const MOBase::VersionInfo &version) { this->get_override("setVersion")(version); }
  virtual void setNewestVersion(const MOBase::VersionInfo &version) { this->get_override("setNewestVersion")(version); }
  virtual void setIsEndorsed(bool endorsed) { this->get_override("setIsEndorsed")(endorsed); }
  virtual void setNexusID(int nexusID) { this->get_override("setNexusID")(nexusID); }
  virtual void addNexusCategory(int categoryID) { this->get_override("addNexusCategory")(categoryID); }
  virtual bool setName(const QString &name) { return this->get_override("setName")(name); }
  virtual bool remove() { return this->get_override("remove")(); }
};


struct IPluginListWrapper: MOBase::IPluginList, boost::python::wrapper<MOBase::IPluginList> {
  virtual PluginState state(const QString &name) const { return this->get_override("state")(name); }
  virtual int priority(const QString &name) const { return this->get_override("priority")(name); }
  virtual int loadOrder(const QString &name) const { return this->get_override("loadOrder")(name); }
  virtual bool isMaster(const QString &name) const { return this->get_override("isMaster")(name); }
  virtual QStringList masters(const QString &name) const { return this->get_override("masters")(name); }
  virtual QString origin(const QString &name) const { return this->get_override("origin")(name); }
  virtual bool onRefreshed(const std::function<void ()> &callback) { return this->get_override("onRefreshed")(callback); }
  virtual bool onPluginMoved(const std::function<void (const QString &, int, int)> &callback) { return this->get_override("onPluginMoved")(callback); }
};


struct IModListWrapper: MOBase::IModList, boost::python::wrapper<MOBase::IModList> {
  virtual QString displayName(const QString &internalName) const { return this->get_override("displayName")(internalName); }
  virtual ModStates state(const QString &name) const { return this->get_override("state")(name); }
  virtual int priority(const QString &name) const { return this->get_override("priority")(name); }
  virtual bool setPriority(const QString &name, int newPriority) { return this->get_override("setPriority")(name, newPriority); }
  virtual bool onModStateChanged(const std::function<void (const QString &, ModStates)> &func) { return this->get_override("onModStateChanged")(func); }
  virtual bool onModMoved(const std::function<void (const QString &, int, int)> &func) { return this->get_override("onModMoved")(func); }
};

#endif // UIBASEWRAPPERS_H
