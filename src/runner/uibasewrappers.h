#ifndef UIBASEWRAPPERS_H
#define UIBASEWRAPPERS_H


#ifndef Q_MOC_RUN
#include <boost/python.hpp>
#endif

#include <QString>

#include <imoinfo.h>
#include <igameinfo.h>
#include <imodrepositorybridge.h>
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
  void requestDownloadURL(int modID, int fileID, QVariant userData)
    { m_Wrapped->requestDownloadURL(modID, fileID, userData); }
  void requestToggleEndorsement(int modID, bool endorse, QVariant userData)
    { m_Wrapped->requestToggleEndorsement(modID, endorse, userData); }

  void onFilesAvailable(boost::python::object callback) {
    m_FilesAvailableHandler = callback;
    connect(m_Wrapped, SIGNAL(filesAvailable(int,QVariant,const QList<ModRepositoryFileInfo>&)),
            this, SLOT(filesAvailable(int,QVariant,const QList<ModRepositoryFileInfo>&)),
            Qt::UniqueConnection);
  }

  void onRequestFailed(boost::python::object callback) {
    m_FailedHandler = callback;
    connect(m_Wrapped, SIGNAL(requestFailed(int,QVariant,QString)),
            this, SLOT(requestFailed(int,QVariant,QString)),
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
//    try {
      GILock lock;
      m_FilesAvailableHandler(modID, userData, resultData);
//    } catch (const boost::python::error_already_set&) {
//      qDebug("error");
      //reportPythonError();
//    }
  }

  void requestFailed(int modID, QVariant userData, const QString &errorMessage)
  {
    try {
      GILock lock;
      m_FailedHandler(modID, userData, errorMessage);
    } catch (const boost::python::error_already_set&) {
      reportPythonError();
    }
  }

private:

  MOBase::IModRepositoryBridge *m_Wrapped;
  boost::python::object m_FilesAvailableHandler;
  boost::python::object m_FailedHandler;

};


struct IOrganizerWrapper: MOBase::IOrganizer, boost::python::wrapper<MOBase::IOrganizer>
{
  virtual MOBase::IGameInfo &gameInfo() const {
    MOBase::IGameInfo *result = this->get_override("gameInfo")();
    return *result;
  }
  virtual MOBase::IModRepositoryBridge *createNexusBridge() const {
    return this->get_override("createNexusBridge")();
  }

  virtual QString profileName() const { return this->get_override("profileName")(); }
  virtual QString profilePath() const { return this->get_override("profilePath")(); }
  virtual QString downloadsPath() const { return this->get_override("downloadsPath")(); }
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
  virtual void installMod(const QString &fileName) { this->get_override("installMod")(fileName); }
  virtual MOBase::IDownloadManager *downloadManager() { return this->get_override("downloadManager")(); }
  virtual MOBase::IPluginList *pluginList() { return this->get_override("pluginList")(); }
  virtual MOBase::IModList *modList() { return this->get_override("modList")(); }
  virtual QString resolvePath(const QString &fileName) const { return this->get_override("resolvePath")(fileName); }
  virtual QStringList listDirectories(const QString &directoryName) const { return this->get_override("listDirectories")(directoryName); }
  virtual QStringList findFiles(const QString &path, const std::function<bool(const QString&)> &filter) const { return this->get_override("findFiles")(path, filter); }
  virtual HANDLE startApplication(const QString &executable, const QStringList &args = QStringList(), const QString &cwd = "", const QString &profile = "") { return this->get_override("startApplication")(executable, args, cwd, profile); }
  virtual bool onAboutToRun(const std::function<bool(const QString&)> &func) { return this->get_override("onAboutToRun")(func); }

};

struct IDownloadManagerWrapper: MOBase::IDownloadManager, boost::python::wrapper<MOBase::IDownloadManager>
{
  virtual int startDownloadURLs(const QStringList &urls) { return this->get_override("downloadURLs")(urls); }
  virtual int startDownloadNexusFile(int modID, int fileID) { return this->get_override("downloadNexusFile")(modID, fileID); }
  virtual QString downloadPath(int id) { return this->get_override("downloadPath")(id); }
private:
  boost::python::object m_DownloadCompleteHandler;
};

struct IGameInfoWrapper: MOBase::IGameInfo, boost::python::wrapper<MOBase::IGameInfo>
{
  virtual Type type() const { return this->get_override("type")(); }
  virtual QString path() const { return this->get_override("path")(); }
  virtual QString binaryName() const { return this->get_override("binaryName")(); }
};


#endif // UIBASEWRAPPERS_H
