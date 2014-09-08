#ifndef Q_MOC_RUN
#include <boost/python.hpp>
#endif
#include <QString>
#include <utility.h>

using namespace MOBase;
namespace bpy = boost::python;

void reportPythonError()
{
  if (PyErr_Occurred()) {
    // prints to s_ErrIO buffer
    PyErr_Print();
    // extract data from python buffer
    bpy::object mainModule = bpy::import("__main__");
    bpy::object mainNamespace = mainModule.attr("__dict__");
    bpy::object errMsgObj = bpy::eval("s_ErrIO.getvalue()", mainNamespace);
    QString errMsg = bpy::extract<QString>(errMsgObj.ptr());
    bpy::eval("s_ErrIO.truncate(0)", mainNamespace);

    throw MyException(errMsg);
  } else {
    throw MyException("An unexpected C++ exception was thrown in python code");
  }
}
