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
      #if defined(SIP_USE_PYCAPSULE)
          PyImport_ImportModule("PyQt5.sip");

          auto errorObj = PyErr_Occurred();
          if (errorObj != NULL) {
              PyObject* type, * value, * traceback;
              PyErr_Fetch(&type, &value, &traceback);
              PyErr_NormalizeException(&type, &value, &traceback);
              if (traceback != NULL) {
                  py::handle h_type(type);
                  py::handle h_val(value);
                  py::handle h_tb(traceback);
                  py::module_ tb = py::module_::import("traceback");
                  py::object fmt_exp(tb.attr("format_exception"));
                  py::object exp_list(fmt_exp(h_type, h_val, h_tb));
                  py::object exp_str(py::str("\n").attr("join")(exp_list));
                  exception = QString::fromStdString(exp_str.cast<std::string>());
              }
              PyErr_Restore(type, value, traceback);
              throw MOBase::Exception(QString("Failed to load PyQt5: %1").arg(exception));
          }

          sipApi = (const sipAPIDef*)PyCapsule_Import("PyQt5.sip._C_API", 0);
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
                      py::module_ tb = py::module_::import("traceback");
                      py::object fmt_exp(tb.attr("format_exception"));
                      py::object exp_list(fmt_exp(h_type, h_val, h_tb));
                      py::object exp_str(py::str("\n").attr("join")(exp_list));
                      exception = QString::fromStdString(exp_str.cast<std::string>());
                  }
                  PyErr_Restore(type, value, traceback);
              }
              throw MOBase::Exception(QString("Failed to load SIP API: %1").arg(exception));
          }
      #else
          PyObject* sip_module;
          PyObject* sip_module_dict;
          PyObject* c_api;

          /* Import the SIP module. */
          sip_module = PyImport_ImportModule("PyQt5.sip");

          if (sip_module == NULL)
              return NULL;

          /* Get the module's dictionary. */
          sip_module_dict = PyModule_GetDict(sip_module);

          /* Get the "_C_API" attribute. */
          c_api = PyDict_GetItemString(sip_module_dict, "_C_API");

          if (c_api == NULL)
              return NULL;

          /* Sanity check that it is the right type. */
          if (!PyCObject_Check(c_api))
              return NULL;

          /* Get the actual pointer from the object. */
          sipApi = (const sipAPIDef*)PyCObject_AsVoidPtr(c_api);
      #endif
      }

      return sipApi;
  }

}
