#ifndef PYTHON_CONVERTERS_UTILS_HPP
#define PYTHON_CONVERTERS_UTILS_HPP

#include <pybind11/pybind11.h>

namespace mo2::details {

  template <class E>
  struct enum_type_name;

  // helper caster for enum-like types
  //
  template <class E>
  struct enum_type_caster {
    static_assert(std::is_enum_v<E>,
      "enum_type_caster should only be used with enum types");

    PYBIND11_TYPE_CASTER(E, pybind11::detail::const_name(enum_type_name<E>::name));

    bool load(pybind11::handle src, bool) {
      PyObject* tmp = PyNumber_Long(src.ptr());

      if (!tmp) {
        return false;
      }

      value = static_cast<E>(PyLong_AsLong(tmp));
      Py_DECREF(tmp);

      return !PyErr_Occurred();
    }

    static pybind11::handle cast(E src, pybind11::return_value_policy, pybind11::handle) {
      return PyLong_FromLongLong(static_cast<int>(src));
    }

  };

}

#endif
