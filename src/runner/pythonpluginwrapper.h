#ifndef PYTHONPLUGINWRAPPER_H
#define PYTHONPLUGINWRAPPER_H

#include <iplugin.h>
#ifndef Q_MOC_RUN
#include <boost/python.hpp>
#endif

class PythonPluginWrapper : public virtual MOBase::IPlugin
{

public:
  PythonPluginWrapper(const boost::python::object &initFunction,
                      const boost::python::object &nameFunction,
                      const boost::python::object &authorFunction,
                      const boost::python::object &descriptionFunction,
                      const boost::python::object &versionFunction,
                      const boost::python::object &isActiveFunction,
                      const boost::python::object &settingsFunction);

  virtual bool init(MOBase::IOrganizer *moInfo);
  virtual QString name() const;
  virtual QString author() const;
  virtual QString description() const;
  virtual MOBase::VersionInfo version() const;
  virtual bool isActive() const;
  virtual QList<MOBase::PluginSetting> settings() const;

protected:
  void reportPythonError() const;

private:

  boost::python::object m_InitFunction;
  boost::python::object m_NameFunction;
  boost::python::object m_AuthorFunction;
  boost::python::object m_DescriptionFunction;
  boost::python::object m_VersionFunction;
  boost::python::object m_IsActiveFunction;
  boost::python::object m_SettingsFunction;

};

#endif // PYTHONPLUGINWRAPPER_H
