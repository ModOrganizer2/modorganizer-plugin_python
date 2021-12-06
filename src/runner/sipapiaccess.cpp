#include "sipapiaccess.h"
#include <boost/python.hpp>
#include <QString>
#include <utility.h>

const sipAPIDef* sipAPIAccess::sipAPI()
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
                boost::python::handle<> h_type(type);
                boost::python::handle<> h_val(value);
                boost::python::handle<> h_tb(traceback);
                boost::python::object tb(boost::python::import("traceback"));
                boost::python::object fmt_exp(tb.attr("format_exception"));
                boost::python::object exp_list(fmt_exp(h_type, h_val, h_tb));
                boost::python::object exp_str(boost::python::str("\n").join(exp_list));
                boost::python::extract<std::string> returned(exp_str);
                exception = QString::fromStdString(returned());
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
                    boost::python::handle<> h_type(type);
                    boost::python::handle<> h_val(value);
                    boost::python::handle<> h_tb(traceback);
                    boost::python::object tb(boost::python::import("traceback"));
                    boost::python::object fmt_exp(tb.attr("format_exception"));
                    boost::python::object exp_list(fmt_exp(h_type, h_val, h_tb));
                    boost::python::object exp_str(boost::python::str("\n").join(exp_list));
                    boost::python::extract<std::string> returned(exp_str);
                    exception = QString::fromStdString(returned());
                }
                PyErr_Restore(type, value, traceback);
            }
            throw MOBase::Exception(QString("Failed to load SIP API: %1").arg(exception));
        }
    }

    return sipApi;
}