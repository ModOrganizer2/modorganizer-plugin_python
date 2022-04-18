#include "pythonutils.h"

#include <filesystem>
#include <set>

#include <pybind11/pybind11.h>

#include <QCoreApplication>

#include "log.h"

namespace py = pybind11;

namespace mo2::python {

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
  void emit_function(py::object self, py::object record)
  {

    // There are other parameters that could be used, but this is minimal for
    // now (filename, line number, etc.).
    const int level = record.attr("levelno").cast<int>();
    const std::wstring msg = py::str(record.attr("msg")).cast<std::wstring>();

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

  void configure_python_logging(py::module_ mobase)
  {
    // most of this is dealing with actual Python objects since it is not possible
    // to derive from logging.Handler in C++ using Boost.Python, and since a lot of
    // this would require extra register only for this.

    // see also https://github.com/pybind/pybind11/issues/1193#issuecomment-429451094

    // retrieve the logging module and the Handler class.
    auto logging = py::module_::import("logging");
    auto Handler = logging.attr("Handler");

    // this is ugly but that's how it's done in C Python
    auto type = py::reinterpret_borrow<py::object>((PyObject*)&PyType_Type);

    // Create the "MO2Handler" python class:
    auto methods = py::dict();
    methods["emit"] = py::cpp_function(emit_function);
    auto MO2Handler = type("LogHandler", py::make_tuple(Handler), methods);

    // Create the default logger:
    auto handler = MO2Handler();
    handler.attr("setLevel")(PyLogLevel::DEBUG);
    auto logger = logging.attr("getLogger")(py::object(mobase.attr("__name__")));
    logger.attr("setLevel")(PyLogLevel::DEBUG);
    logger.attr("addHandler")(handler);

    // Set mobase attributes:
    mobase.attr("LogHandler") = MO2Handler;
    mobase.attr("logger") = logger;
  }

  void show_deprecation_warning(std::string_view name, std::string_view message, bool show_once)
  {

    // Contains the list of filename / line number for which a deprecation warning has already been shown.
    static std::set<std::pair<std::string, int>> DeprecatedLines;

    // Find the caller:
    auto inspect = py::module_::import("inspect");
    auto current_frame = inspect.attr("currentframe")();
    py::sequence callable_frame = inspect.attr("getouterframes")(current_frame, 2);
    auto filename = callable_frame[-1].attr("filename").cast<std::string>();
    auto function = callable_frame[-1].attr("function").cast<std::string>();
    auto lineno = callable_frame[-1].attr("lineno").cast<int>();

    // Only show once if requested:
    if (show_once && DeprecatedLines.contains({ filename, lineno })) {
      return;
    }

    // Register the deprecation:
    DeprecatedLines.emplace(filename, lineno);

    auto path = relative(std::filesystem::path(filename), QCoreApplication::applicationDirPath().toStdWString());

    // Show the message:
    if (message.empty()) {
      MOBase::log::warn(
        "[deprecated] {} in {} [{}:{}].", name, function, path.native(), lineno);
    }
    else {
      MOBase::log::warn(
        "[deprecated] {} in {} [{}:{}]: {}", name, function, path.native(), lineno, message);
    }
  }

}
