#include "pythonpluginwrapper.h"


using boost::python::extract;
using boost::python::handle_exception;


PythonPluginWrapper::PythonPluginWrapper(const boost::python::object &initFunction,
                                         const boost::python::object &nameFunction,
                                         const boost::python::object &authorFunction,
                                         const boost::python::object &descriptionFunction,
                                         const boost::python::object &versionFunction,
                                         const boost::python::object &isActiveFunction,
                                         const boost::python::object &settingsFunction)
  : m_InitFunction(initFunction)
  , m_NameFunction(nameFunction)
  , m_AuthorFunction(authorFunction)
  , m_DescriptionFunction(descriptionFunction)
  , m_VersionFunction(versionFunction)
  , m_IsActiveFunction(isActiveFunction)
  , m_SettingsFunction(settingsFunction)
{
}


void PythonPluginWrapper::reportPythonError() const
{
  if (PyErr_Occurred()) {
    PyErr_Print();
  } else {
    qCritical("An unexpected C++ exception was thrown in python code");
  }
}


bool PythonPluginWrapper::init(MOBase::IOrganizer *moInfo)
{
  try {
   return boost::python::extract<bool>(m_InitFunction(boost::python::ptr(moInfo)));
  } catch (const boost::python::error_already_set&) {
    reportPythonError();
    return false;
  }
}

QString PythonPluginWrapper::name() const
{
  try {
    return boost::python::extract<QString>(m_NameFunction());
  } catch (const boost::python::error_already_set&) {
    reportPythonError();
    return QString();
  }
}

QString PythonPluginWrapper::author() const
{
  try {
    return boost::python::extract<QString>(m_AuthorFunction());
  } catch (const boost::python::error_already_set&) {
    reportPythonError();
    return QString();
  }
}

QString PythonPluginWrapper::description() const
{
  try {
    return boost::python::extract<QString>(m_DescriptionFunction());
  } catch (const boost::python::error_already_set&) {
    reportPythonError();
    return QString();
  }
}

MOBase::VersionInfo PythonPluginWrapper::version() const
{
  try {
    return boost::python::extract<MOBase::VersionInfo>(m_VersionFunction());
  } catch (const boost::python::error_already_set&) {
    reportPythonError();
    return MOBase::VersionInfo();
  }
}

bool PythonPluginWrapper::isActive() const
{
  try {
    return boost::python::extract<bool>(m_IsActiveFunction());
  } catch (const boost::python::error_already_set&) {
    reportPythonError();
    return false;
  }
}

QList<MOBase::PluginSetting> PythonPluginWrapper::settings() const
{
  QList<MOBase::PluginSetting> result;
  try {
    boost::python::object l = m_SettingsFunction();
    if (!l.is_none()) {
//    boost::python::list l = extract<boost::python::list>(temp);
      for (int i = 0; i < boost::python::len(l); ++i) {
        result.append(extract<MOBase::PluginSetting>(l[i]));
      }
    }
  } catch (const boost::python::error_already_set&) {
    reportPythonError();
  }

  return result;
}
