#ifndef PROXYPYTHON_H
#define PROXYPYTHON_H


#include <ipluginproxy.h>
#include <imoinfo.h>


class ProxyPython : public MOBase::IPluginProxy
{
  Q_OBJECT
  Q_INTERFACES(MOBase::IPlugin MOBase::IPluginProxy)

public:
  ProxyPython();

  virtual bool init(MOBase::IOrganizer *moInfo);
  virtual QString name() const;
  virtual QString author() const;
  virtual QString description() const;
  virtual MOBase::VersionInfo version() const;
  virtual bool isActive() const;
  virtual QList<MOBase::PluginSetting> settings() const;

  QStringList pluginList(const QString &pluginPath) const;
  QObject *instantiate(const QString &pluginName) const;

private:
  const MOBase::IOrganizer *m_MOInfo;
};

#endif // PROXYPYTHON_H
