#include "converters_qt_sip.h"

#include <QString>

#include <utility.h>

namespace py = pybind11;

namespace mo2::details {

  const sipAPIDef* sipAPI()
  {
    QString exception;
    static const sipAPIDef* sipApi = nullptr;
    if (sipApi == nullptr) {
      PyImport_ImportModule("PyQt6.sip");

      auto errorObj = PyErr_Occurred();
      if (errorObj != NULL) {
        PyObject* type, * value, * traceback;
        PyErr_Fetch(&type, &value, &traceback);
        PyErr_NormalizeException(&type, &value, &traceback);
        if (traceback != NULL) {
          py::handle h_type(type);
          py::handle h_val(value);
          py::handle h_tb(traceback);
          py::object tb(py::module_::import("traceback"));
          py::object fmt_exp(tb.attr("format_exception"));
          py::object exp_list(fmt_exp(h_type, h_val, h_tb));
          py::object exp_str(py::str("\n").attr("join")(exp_list));
          exception = QString::fromStdString(exp_str.cast<std::string>());
        }
        PyErr_Restore(type, value, traceback);
        throw MOBase::Exception(QString("Failed to load PyQt6: %1").arg(exception));
      }

      sipApi = (const sipAPIDef*)PyCapsule_Import("PyQt6.sip._C_API", 0);
      if (sipApi == NULL) {
        auto errorObj = PyErr_Occurred();
        if (errorObj != NULL) {
          PyObject* type, * value, * traceback;
          PyErr_Fetch(&type, &value, &traceback);
          PyErr_NormalizeException(&type, &value, &traceback);
          if (traceback != NULL) {
              py::handle h_type(type);
              py::handle h_val(value);
              py::handle h_tb(traceback);
              py::object tb(py::module_::import("traceback"));
              py::object fmt_exp(tb.attr("format_exception"));
              py::object exp_list(fmt_exp(h_type, h_val, h_tb));
              py::object exp_str(py::str("\n").attr("join")(exp_list));
              exception = QString::fromStdString(exp_str.cast<std::string>());
          }
          PyErr_Restore(type, value, traceback);
        }
        throw MOBase::Exception(QString("Failed to load SIP API: %1").arg(exception));
      }
    }
    return sipApi;
  }

}
