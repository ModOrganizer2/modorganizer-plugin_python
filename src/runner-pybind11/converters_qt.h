#ifndef PYTHON_CONVERTERS_QT_HPP
#define PYTHON_CONVERTERS_QT_HPP

#include <QFlag>
#include <QFlags>
#include <QList>
#include <QMap>
#include <QMessageBox>
#include <QSet>
#include <QString>
#include <QVariant>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "converters_utils.h"

// for enum casters
namespace mo2::details {
  template <>
  struct enum_type_name<QMessageBox::StandardButton> {
    constexpr static const char name[] = "QMessageBox.StandardButton";
  };

  template <>
  struct enum_type_name<QMessageBox::Icon> {
    constexpr static const char name[] = "QMessageBox.Icon";
  };
}

namespace pybind11::detail {

  // helper class for QMap because QMap do not follow the standard std:: maps interface,
  // for other containers, the pybind11 built-in xxx_caster works
  //
  // this code is basically a copy/paste from the pybind11 stl stuff with minor
  // modifications
  //
  template <typename Type, typename Key, typename Value>
  struct qmap_caster {
      using key_conv = make_caster<Key>;
      using value_conv = make_caster<Value>;

      bool load(handle src, bool convert) {
          if (!isinstance<dict>(src)) {
              return false;
          }
          auto d = reinterpret_borrow<dict>(src);
          value.clear();
          for (auto it : d) {
              key_conv kconv;
              value_conv vconv;
              if (!kconv.load(it.first.ptr(), convert) || !vconv.load(it.second.ptr(), convert)) {
                  return false;
              }
              value[cast_op<Key &&>(std::move(kconv))] = cast_op<Value &&>(std::move(vconv));
          }
          return true;
      }

      template <typename T>
      static handle cast(T &&src, return_value_policy policy, handle parent) {
          dict d;
          return_value_policy policy_key = policy;
          return_value_policy policy_value = policy;
          if (!std::is_lvalue_reference<T>::value) {
              policy_key = return_value_policy_override<Key>::policy(policy_key);
              policy_value = return_value_policy_override<Value>::policy(policy_value);
          }
          for (auto it = src.begin(); it != src.end(); ++it) {
              auto key = reinterpret_steal<object>(
                  key_conv::cast(forward_like<T>(it.key()), policy_key, parent));
              auto value = reinterpret_steal<object>(
                  value_conv::cast(forward_like<T>(it.value()), policy_value, parent));
              if (!key || !value) {
                  return handle();
              }
              d[key] = value;
          }
          return d.release();
      }

      PYBIND11_TYPE_CASTER(Type,
                          const_name("Dict[") + key_conv::name + const_name(", ") + value_conv::name
                              + const_name("]"));
  };

  // QString
  //
  template <>
  struct type_caster<QString> {
    PYBIND11_TYPE_CASTER(QString, const_name("str"));

    /**
     * Conversion part 1 (Python->C++): convert a PyObject into a QString
     * instance or return false upon failure. The second argument
     * indicates whether implicit conversions should be applied.
     */
    bool load(handle src, bool) {

      PyObject *objPtr = src.ptr();

      if (!PyBytes_Check(objPtr) && !PyUnicode_Check(objPtr)) {
        return false;
      }

      // Ensure the string uses 8-bit characters
      PyObject *strPtr = PyUnicode_Check(objPtr) ? PyUnicode_AsUTF8String(objPtr) : objPtr;

      // Extract the character data from the python string
      value = QString::fromUtf8(PyBytes_AsString(strPtr));

      // Deallocate local copy if one was made
      if (strPtr != objPtr) {
        Py_DecRef(strPtr);
      }

      return true;
    }

    /**
     * Conversion part 2 (C++ -> Python): convert an QString instance into
     * a Python object. The second and third arguments are used to
     * indicate the return value policy and parent object (for
     * ``return_value_policy::reference_internal``) and are generally
     * ignored by implicit casters.
     */
    static handle cast(QString src, return_value_policy /* policy */, handle /* parent */) {
      static_assert(sizeof(QChar) == 2);
      return PyUnicode_FromKindAndData(PyUnicode_2BYTE_KIND, src.constData(), src.length());
    }
  };

  // QVariant - this needs to be defined BEFORE QVariantList
  //
  template <>
  struct type_caster<QVariant> {
  public:
    PYBIND11_TYPE_CASTER(QVariant, const_name("MOVariant"));

    /**
     * Conversion part 1 (Python->C++): convert a PyObject into a QVariant
     * instance or return false upon failure. The second argument
     * indicates whether implicit conversions should be applied.
     */
    bool load(handle src, bool);

    /**
     * Conversion part 2 (C++ -> Python): convert an QString instance into
     * a Python object. The second and third arguments are used to
     * indicate the return value policy and parent object (for
     * ``return_value_policy::reference_internal``) and are generally
     * ignored by implicit casters.
     */
    static handle cast(QVariant var, return_value_policy policy, handle parent);
  };

  // QList
  //
  template <class T>
  struct type_caster<QList<T>> : list_caster<QList<T>, T> {};

  // QSet
  //
  template <class T>
  struct type_caster<QSet<T>> : set_caster<QList<T>, T> {};

  // QMap
  //
  template <class K, class V>
  struct type_caster<QMap<K, V>> : qmap_caster<QMap<K, V>, K, V> {};

  // QStringList
  //
  template <>
  struct type_caster<QStringList> : list_caster<QStringList, QString> {};

  // QVariantList
  //
  template <>
  struct type_caster<QVariantList> : list_caster<QVariantList, QVariant> {};

  // QVariantMap
  //
  template <>
  struct type_caster<QVariantMap> : qmap_caster<QVariantMap, QString, QVariant> {};

  bool type_caster<QVariant>::load(handle src, bool implicit) {
    if (PyList_Check(src.ptr())) {
      // we could check if all the elements can be converted to QString and store a QStringList
      // in the QVariant but I am not sure that is really useful.
      value = src.cast<QVariantList>();
      return true;
    }
    else if (src == Py_None) {
      value = QVariant();
      return true;
    }
    else if (PyDict_Check(src.ptr())) {
      value = src.cast<QVariantMap>();
      return true;
    }
    else if (PyBytes_Check(src.ptr()) || PyUnicode_Check(src.ptr())) {
      value = src.cast<QString>();
      return true;
    }
    // PyBool will also return true for PyLong_Check but not the other way around, so
    // the order here is relevant.
    else if (PyBool_Check(src.ptr())) {
      value = src.cast<bool>();
      return true;
    }
    else if (PyLong_Check(src.ptr())) {
      // QVariant doesn't have long. It has int or long long. Given that on m/s,
      // long is 32 bits for 32- and 64- bit code...
      value = src.cast<int>();
      return true;
    }
    else {
      return false;
    }
  }

  handle type_caster<QVariant>::cast(QVariant var, return_value_policy policy, handle parent) {
    switch (var.type()) {
      case QVariant::Invalid: return Py_None;
      case QVariant::Int: return PyLong_FromLong(var.toInt());
      case QVariant::UInt: return PyLong_FromUnsignedLong(var.toUInt());
      case QVariant::Bool: return PyBool_FromLong(var.toBool());
      case QVariant::String: return type_caster<QString>::cast(var.toString(), policy, parent);
      // We need to check for StringList here because these are not considered List
      // since List is QList<QVariant> will StringList is QList<QString>:
      case QVariant::StringList: return type_caster<QStringList>::cast(var.toStringList(), policy, parent);
      case QVariant::List: return type_caster<QVariantList>::cast(var.toList(), policy, parent);
      case QVariant::Map: // return type_caster<QVariantMap>::cast(var.toList(), policy, parent);
      default: {
        PyErr_Format(PyExc_TypeError, "type unsupported: %d", var.type());
        throw pybind11::error_already_set();
      }
    }
  }

  // QMessageBox's enums
  //
  template <>
  struct type_caster<QMessageBox::StandardButton> :
    mo2::details::enum_type_caster<QMessageBox::StandardButton> {};
  template <>
  struct type_caster<QMessageBox::Icon> :
    mo2::details::enum_type_caster<QMessageBox::Icon> {};

  // QFlags
  //
  template <class T>
  struct type_caster<QFlags<T>> {
    PYBIND11_TYPE_CASTER(QFlags<T>, const_name("QFlags[") + make_caster<T>::name + const_name("]"));

    /**
     * Conversion part 1 (Python->C++): convert a PyObject into a QString
     * instance or return false upon failure. The second argument
     * indicates whether implicit conversions should be applied.
     */
    bool load(handle src, bool implicit) {
      PyObject* tmp = PyNumber_Long(src.ptr());

      if (!tmp) {
        return false;
      }

      // we do an intermediate extraction to T but this actually
      // can contains multiple values
      T flag_value = static_cast<T>(PyLong_AsLong(tmp));
      Py_DECREF(tmp);

      value = QFlags<T>(flag_value);

      return !PyErr_Occurred();
    }

    /**
     * Conversion part 2 (C++ -> Python): convert an QString instance into
     * a Python object. The second and third arguments are used to
     * indicate the return value policy and parent object (for
     * ``return_value_policy::reference_internal``) and are generally
     * ignored by implicit casters.
     */
    static handle cast(QFlags<T> const& src, return_value_policy /* policy */, handle /* parent */) {
      return PyLong_FromLong(static_cast<int>(src));
    }

  };


} // namespace pybind11::detail

#endif
