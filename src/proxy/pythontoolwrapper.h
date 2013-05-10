#ifndef PYTHONTOOLWRAPPER_H
#define PYTHONTOOLWRAPPER_H


#include <iplugintool.h>
#ifndef Q_MOC_RUN
#include <boost/python.hpp>
#endif

#include "pythonpluginwrapper.h"


class PythonToolWrapper : public QObject, public MOBase::IPluginTool, public PythonPluginWrapper
{

  Q_OBJECT
  Q_INTERFACES(MOBase::IPlugin MOBase::IPluginTool)

public:
  PythonToolWrapper(const boost::python::object &initFunction,
                    const boost::python::object &nameFunction,
                    const boost::python::object &authorFunction,
                    const boost::python::object &descriptionFunction,
                    const boost::python::object &versionFunction,
                    const boost::python::object &isActiveFunction,
                    const boost::python::object &settingsFunction,
                    const boost::python::object &displayNameFunction,
                    const boost::python::object &tooltipFunction,
                    const boost::python::object &iconFunction,
                    const boost::python::object &displayFunction,
                    const boost::python::object &setParentWidget);

  using PythonPluginWrapper::init;
  using PythonPluginWrapper::name;

  virtual QString displayName() const;
  virtual QString tooltip() const;
  virtual QIcon icon() const;

public slots:

   virtual void display() const;

private:

  boost::python::object m_DisplayNameFunction;
  boost::python::object m_ToolTipFunction;
  boost::python::object m_IconFunction;
  boost::python::object m_DisplayFunction;

  boost::python::object m_SetParentWidget;
};



#endif // PYTHONTOOLWRAPPER_H
