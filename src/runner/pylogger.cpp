#include "pylogger.h"

#include "log.h"

#include <boost/python.hpp>

namespace bpy = boost::python;

// Small structure to hold the levels - There are copy paste from
// my Python version and I assume these will not change soon:
struct PyLogLevel {
  static constexpr int CRITICAL = 50;
  static constexpr int ERROR = 40;
  static constexpr int WARNING = 30;
  static constexpr int INFO = 20;
  static constexpr int DEBUG = 10;
};

// This is the function we are going to use as our Handler .emit
// method.
void emit_function(bpy::object self, bpy::object record) {

  // There are other parameters that could be used, but this is minimal for
  // now (filename, line number, etc.).
  const int level = bpy::extract<int>(record.attr("levelno"));
  const std::wstring msg = bpy::extract<std::wstring>(bpy::str(record.attr("msg")));

  switch (level) {
  case PyLogLevel::CRITICAL:
  case PyLogLevel::ERROR:
    MOBase::log::error("{}", msg);
    break;
  case PyLogLevel::WARNING:
    MOBase::log::warn("{}", msg);
    break;
  case PyLogLevel::INFO:
    MOBase::log::info("{}", msg);
    break;
  case PyLogLevel::DEBUG:
  default: // There is a "NOTSET" level in theory:
    MOBase::log::debug("{}", msg);
    break;
  }
};

void configure_python_logging()
{
  // Most of this is dealing with actual Python objects since it is not possible
  // to derive from logging.Handler in C++ using Boost.Python, and since a lot of
  // this would require extra register only for this.

  // Retrieve the logging module and the Handler class.
  auto logging = bpy::import("logging");
  auto handler = logging.attr("Handler");

  // This is ugly but that's how it's done in C Python.
  auto type = (PyObject*)&PyType_Type;

  // Create the "MO2Handler" python class:
  auto methods = bpy::dict();
  methods["emit"] = bpy::make_function(emit_function);
  auto MO2Handler = bpy::call<bpy::object>(type, "MO2Handler", bpy::make_tuple(handler), methods);

  // Call basicConfig() with a new instance of our handler.
  auto kwargs = bpy::dict();
  kwargs["handlers"] = bpy::make_tuple(MO2Handler());
  kwargs["level"] = 0;
  logging.attr("basicConfig")(*bpy::make_tuple(), **kwargs);
}