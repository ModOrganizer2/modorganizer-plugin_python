#ifndef PROXYPYTHON_H
#define PROXYPYTHON_H


#include <ipluginproxy.h>
#include <map>

#ifndef Q_MOC_RUN
#include <boost/python.hpp>
#endif


class ProxyPython : public QObject, MOBase::IPluginProxy
{
  Q_OBJECT
  Q_INTERFACES(MOBase::IPlugin MOBase::IPluginProxy)
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
  Q_PLUGIN_METADATA(IID "org.tannin.ProxyPython" FILE "proxypython.json")
#endif

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
  QObject *instantiate(const QString &pluginName);

  /**
   * @return the parent widget for newly created dialogs
   * @note needs to be public so it can be exposed to plugins
   */
  virtual QWidget *getParentWidget() { return parentWidget(); }

private:

  const MOBase::IOrganizer *m_MOInfo;

  std::map<QString, boost::python::object> m_PythonObjects;

};

#endif // PROXYPYTHON_H
