#ifndef UIBASEWRAPPERS_H
#define UIBASEWRAPPERS_H


#ifndef Q_MOC_RUN
#include <boost/python.hpp>
#endif

#include <QString>

#include <imoinfo.h>
#include <igameinfo.h>
#include <imodrepositorybridge.h>


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
  virtual MOBase::IModInterface *createMod(const QString &name) { return this->get_override("createMod")(name); }
  virtual bool removeMod(MOBase::IModInterface *mod) { return this->get_override("removeMod")(mod); }
  virtual void modDataChanged(MOBase::IModInterface *mod) { this->get_override("modDataChanged")(mod); }
  virtual QVariant pluginSetting(const QString &pluginName, const QString &key) const { return this->get_override("pluginSetting")(pluginName, key); }
  virtual QString pluginDataPath() const { return this->get_override("pluginDataPath")(); }
};


struct IGameInfoWrapper: MOBase::IGameInfo, boost::python::wrapper<MOBase::IGameInfo>
{
  virtual Type type() const { return this->get_override("type")(); }
  virtual QString path() const { return this->get_override("path")(); }
  virtual QString binaryName() const { return this->get_override("binaryName")(); }
};


struct INexusBridgeWrapper: MOBase::IModRepositoryBridge, boost::python::wrapper<MOBase::IModRepositoryBridge>
{
  virtual void requestDescription(int modID, QVariant userData)
    { this->get_override("requestDescription")(modID, userData); }
  virtual void requestFiles(int modID, QVariant userData)
    { this->get_override("requestFiles")(modID, userData); }
  virtual void requestFileInfo(int modID, int fileID, QVariant userData)
    { this->get_override("requestFileInfo")(modID, fileID, userData); }
  virtual void requestDownloadURL(int modID, int fileID, QVariant userData)
    { this->get_override("requestDownloadURL")(modID, fileID, userData); }
  virtual void requestToggleEndorsement(int modID, bool endorse, QVariant userData)
    { this->get_override("requestToggleEndorsement")(modID, endorse, userData); }

};


#endif // UIBASEWRAPPERS_H
