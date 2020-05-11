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
#include <isavegame.h>
#include <isavegameinfowidget.h>
#include "ifiletree.h"

#include "error.h"
#include "gilock.h"
#include "pythonwrapperutilities.h"

extern MOBase::IOrganizer *s_Organizer;

struct IProfileWrapper: MOBase::IProfile, boost::python::wrapper<MOBase::IProfile>
{
  virtual QString name() const override { return this->get_override("name")(); }
  virtual QString absolutePath() const override { return this->get_override("absolutePath")(); }
  virtual bool localSavesEnabled() const override { return this->get_override("localSavesEnabled")(); }
  virtual bool localSettingsEnabled() const override { return this->get_override("localSettingsEnabled")(); }
  virtual bool invalidationActive(bool *supported) const override { return this->get_override("invalidationActive")(supported); }
};

struct IDownloadManagerWrapper: MOBase::IDownloadManager, boost::python::wrapper<MOBase::IDownloadManager>
{
  virtual int startDownloadURLs(const QStringList &urls) { return this->get_override("startDownloadURLs")(urls); }
  virtual int startDownloadNexusFile(int modID, int fileID) { return this->get_override("startDownloadNexusFile")(modID, fileID); }
  virtual QString downloadPath(int id) { return this->get_override("downloadPath")(id); }
};

struct IModRepositoryBridgeWrapper: MOBase::IModRepositoryBridge, boost::python::wrapper<MOBase::IModRepositoryBridge>
{
  using IModRepositoryBridge::IModRepositoryBridge;
  virtual void requestDescription(QString gameName, int modID, QVariant userData) { this->get_override("requestDescription")(gameName, modID, userData); }
  virtual void requestFiles(QString gameName, int modID, QVariant userData) { this->get_override("requestFiles")(gameName, modID, userData); }
  virtual void requestFileInfo(QString gameName, int modID, int fileID, QVariant userData) { this->get_override("requestFileInfo")(gameName, modID, fileID, userData); }
  virtual void requestDownloadURL(QString gameName, int modID, int fileID, QVariant userData) { this->get_override("requestDownloadURL")(gameName, modID, fileID, userData); }
  virtual void requestToggleEndorsement(QString gameName, int modID, QString modVersion, bool endorse, QVariant userData) { this->get_override("requestToggleEndorsement")(gameName, modID, endorse, userData); }
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
  virtual void setGamePlugin(const MOBase::IPluginGame *gamePlugin) override { this->get_override("setGamePlugin")(gamePlugin); }
  virtual bool setName(const QString &name) override { return this->get_override("setName")(name); }
  virtual bool remove() override { return this->get_override("remove")(); }
  virtual void addCategory(const QString &categoryName) override { this->get_override("addCategory")(categoryName); }
  virtual bool removeCategory(const QString &categoryName) override { return this->get_override("removeCategory")(categoryName); }
  virtual QStringList categories() const override { return this->get_override("categories")(); }
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


// This needs to be extendable in Python, so actually needs a wrapper (everything else probably doesn't):
class ISaveGameWrapper : public MOBase::ISaveGame, public boost::python::wrapper<MOBase::ISaveGame>
{
public:
  static constexpr const char* className = "ISaveGameWrapper";
  using boost::python::wrapper<MOBase::ISaveGame>::get_override;

  virtual QString getFilename() const override { return basicWrapperFunctionImplementation<ISaveGameWrapper, QString>(this, "getFilename"); };
  virtual QDateTime getCreationTime() const override { return basicWrapperFunctionImplementation<ISaveGameWrapper, QDateTime>(this, "getCreationTime"); };
  virtual QString getSaveGroupIdentifier() const override { return basicWrapperFunctionImplementation<ISaveGameWrapper, QString>(this, "getSaveGroupIdentifier"); };
  virtual QStringList allFiles() const override { return basicWrapperFunctionImplementation<ISaveGameWrapper, QStringList>(this, "allFiles"); };
  virtual bool hasScriptExtenderFile() const override { return basicWrapperFunctionImplementation<ISaveGameWrapper, bool>(this, "hasScriptExtenderFile"); };
};

// This needs a wrapper but currently I have no idea how to expose this properly to python:
class ISaveGameInfoWidgetWrapper : public MOBase::ISaveGameInfoWidget, public boost::python::wrapper<MOBase::ISaveGameInfoWidget>
{
public:
  static constexpr const char* className = "ISaveGameInfoWidgetWrapper";
  using boost::python::wrapper<MOBase::ISaveGameInfoWidget>::get_override;

  // Bring the constructor:
  using ISaveGameInfoWidget::ISaveGameInfoWidget;

  virtual void setSave(QString const& save) override { basicWrapperFunctionImplementation<ISaveGameInfoWidgetWrapper, void>(this, "setSave", save); };
};

#endif // UIBASEWRAPPERS_H
