#include "pythontoolwrapper.h"
#include <QApplication>
#include <QWidget>

using boost::python::extract;
using boost::python::handle_exception;

PythonToolWrapper::PythonToolWrapper(const boost::python::object &initFunction,
                                     const boost::python::object &nameFunction,
                                     const boost::python::object &authorFunction,
                                     const boost::python::object &descriptionFunction,
                                     const boost::python::object &versionFunction,
                                     const boost::python::object &isActiveFunction,
                                     const boost::python::object &settingsFunction,
                                     const boost::python::object &displayNameFunction,
                                     const boost::python::object &tooltipFunction,
                                     const boost::python::object &iconFunction,
                                     const boost::python::object &displayFunction)
  : PythonPluginWrapper(initFunction, nameFunction, authorFunction, descriptionFunction,
                        versionFunction, isActiveFunction, settingsFunction)
  , m_DisplayNameFunction(displayNameFunction)
  , m_ToolTipFunction(tooltipFunction)
  , m_IconFunction(iconFunction)
  , m_DisplayFunction(displayFunction)
{
}

QString PythonToolWrapper::displayName() const
{
  try {
    return boost::python::extract<QString>(m_DisplayNameFunction());
  } catch (const boost::python::error_already_set&) {
    reportPythonError();
    return QString();
  }
}

QString PythonToolWrapper::tooltip() const
{
  try {
    return boost::python::extract<QString>(m_ToolTipFunction());
  } catch (const boost::python::error_already_set&) {
    reportPythonError();
    return QString();
  }
}

QIcon PythonToolWrapper::icon() const
{
  try {
    std::vector<const char*> rows;
    boost::python::object l = m_IconFunction();
    if (!l.is_none()) {
      for (int i = 0; i < boost::python::len(l); ++i) {
        rows.push_back(extract<const char*>(l[i]));
      }
      return QIcon(QPixmap(&rows[0]));
    } else {
      return QIcon();
    }
  } catch (const boost::python::error_already_set&) {
    reportPythonError();
    return QIcon();
  }
}

void PythonToolWrapper::display() const
{
  try {
    // can't make the dialog properly modal but we can disable input on the main window
    // which has a similar effect
    QWidget *mainWindow = QApplication::activeWindow();
    if (mainWindow != NULL) mainWindow->setEnabled(false);
    m_DisplayFunction();
    if (mainWindow != NULL) mainWindow->setEnabled(true);
  } catch (const boost::python::error_already_set&) {
    reportPythonError();
  }
}
