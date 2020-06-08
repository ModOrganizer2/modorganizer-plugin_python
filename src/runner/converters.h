#ifndef PYTHON_CONVERTERS_HPP
#define PYTHON_CONVERTERS_HPP

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QString>
#include <QVariant>
#include <QWidget>

// sip and qt slots seems to conflict
#include <sip.h>

// Include the container converters from utils:
#include "pythonutils.h"

namespace utils {

  namespace bpy = boost::python;

  namespace QString_converter {

    /**
     * We need this since sip does not expose QString but uses standard python str.
     */
    struct QString_to_python_str
    {
      static PyObject* convert(const QString& str) {
        // It's safer to explicitly convert to unicode as if we don't, this can return 
        // either str or unicode without it being easy to know which to expect
        bpy::object pyStr = bpy::object(qUtf8Printable(str));
        if (SIPBytes_Check(pyStr.ptr()))
          pyStr = pyStr.attr("decode")("utf-8");
        return bpy::incref(pyStr.ptr());
      }
    };

    struct QString_from_python_str
    {

      static void* convertible(PyObject* objPtr) {
        return SIPBytes_Check(objPtr) || PyUnicode_Check(objPtr) ? objPtr : nullptr;
      }

      static void construct(PyObject* objPtr, bpy::converter::rvalue_from_python_stage1_data* data) {
        // Ensure the string uses 8-bit characters
        PyObject* strPtr = PyUnicode_Check(objPtr) ? PyUnicode_AsUTF8String(objPtr) : objPtr;

        // Extract the character data from the python string
        const char* value = SIPBytes_AsString(strPtr);
        assert(value != nullptr);

        // allocate storage
        void* storage = ((bpy::converter::rvalue_from_python_storage<QString>*)data)->storage.bytes;

        // construct QString in the allocated memory
        new (storage) QString(value);

        data->convertible = storage;

        // Deallocate local copy if one was made
        if (strPtr != objPtr)
          Py_DecRef(strPtr);
      }
    };

  }

  namespace QFlags_converter {

    /**
     *
     */
    template <typename T>
    struct QFlags_to_int
    {
      static PyObject* convert(const QFlags<T>& flags) {
        return bpy::incref(bpy::object(static_cast<int>(flags)).ptr());
      }
    };

    template <typename T>
    struct QFlags_from_python_obj
    {

      static void* convertible(PyObject* objPtr) {
        return SIPLong_Check(objPtr) ? objPtr : nullptr;
      }

      static void construct(PyObject* objPtr, bpy::converter::rvalue_from_python_stage1_data* data) {
        int intVersion = (int)SIPLong_AsLong(objPtr);
        T tVersion = (T)intVersion;
        void* storage = ((bpy::converter::rvalue_from_python_storage<QFlags<T>>*)data)->storage.bytes;
        new (storage) QFlags<T>(tVersion);

        data->convertible = storage;
      }
    };

  }

  namespace QVariant_converter {

    struct QVariant_to_python_obj
    {
      static PyObject* convert(const QVariant& var) {
        switch (var.type()) {
        case QVariant::Invalid: return bpy::incref(Py_None);
        case QVariant::Int: return SIPLong_FromLong(var.toInt());
        case QVariant::UInt: return PyLong_FromUnsignedLong(var.toUInt());
        case QVariant::Bool: return PyBool_FromLong(var.toBool());
        case QVariant::String: return bpy::incref(bpy::object(var.toString()).ptr());
        // We need to check for StringList here because these are not considered List
        // since List is QList<QVariant> will StringList is QList<QString>:
        case QVariant::StringList: return bpy::incref(bpy::object(var.toStringList()).ptr());
        case QVariant::List: {
          return bpy::incref(bpy::object(var.toList()).ptr());
        } break;
        case QVariant::Map: {
          return bpy::incref(bpy::object(var.toMap()).ptr());
        } break;
        default: {
          PyErr_Format(PyExc_TypeError, "type unsupported: %d", var.type());
          throw bpy::error_already_set();
        } break;
        }
      }
    };

    struct QVariant_from_python_obj
    {

      static void* convertible(PyObject* objPtr) {
        if (!SIPBytes_Check(objPtr) && !PyUnicode_Check(objPtr) && !PyLong_Check(objPtr) &&
          !PyBool_Check(objPtr) && !PyList_Check(objPtr) && !PyDict_Check(objPtr) &&
          objPtr != Py_None) {
          return nullptr;
        }
        return objPtr;
      }

      template <typename T>
      static void constructVariant(const T& value, bpy::converter::rvalue_from_python_stage1_data* data) {
        void* storage = ((bpy::converter::rvalue_from_python_storage<QVariant>*)data)->storage.bytes;

        new (storage) QVariant(value);

        data->convertible = storage;
      }

      static void constructVariant(bpy::converter::rvalue_from_python_stage1_data* data) {
        void* storage = ((bpy::converter::rvalue_from_python_storage<QVariant>*)data)->storage.bytes;

        new (storage) QVariant();

        data->convertible = storage;
      }

      static void construct(PyObject* objPtr, bpy::converter::rvalue_from_python_stage1_data* data) {
        if (PyList_Check(objPtr)) {
          // We could check if all the elements can be converted to QString and store a QStringList
          // in the QVariant but I am not sure that is really useful.
          constructVariant(bpy::extract<QVariantList>(objPtr)(), data);
        }
        else if (objPtr == Py_None) {
          constructVariant(data);
        }
        else if (PyDict_Check(objPtr)) {
          constructVariant(bpy::extract<QVariantMap>(objPtr)(), data);
        }
        else if (SIPBytes_Check(objPtr) || PyUnicode_Check(objPtr)) {
          constructVariant(bpy::extract<QString>(objPtr)(), data);
        }
        // PyBools will also return true for SIPLong_Check but not the other way around, so the order
        // here is relevant.
        else if (PyBool_Check(objPtr)) {
          constructVariant(bpy::extract<bool>(objPtr)(), data);
        }
        else if (SIPLong_Check(objPtr)) {
          // QVariant doesn't have long. It has int or long long. Given that on m/s,
          // long is 32 bits for 32- and 64- bit code...
          constructVariant(bpy::extract<int>(objPtr)(), data);
        }
        else {
          PyErr_SetString(PyExc_TypeError, "type unsupported");
          throw bpy::error_already_set();
        }
      }
    };

  }

  namespace QClass_converter {

    template <typename T> struct MetaData;

    template <> struct MetaData<QObject> { static const char* className() { return "QObject"; } };
    template <> struct MetaData<QWidget> { static const char* className() { return "QWidget"; } };
    template <> struct MetaData<QDateTime> { static const char* className() { return "QDateTime"; } };
    template <> struct MetaData<QDir> { static const char* className() { return "QDir"; } };
    template <> struct MetaData<QFileInfo> { static const char* className() { return "QFileInfo"; } };
    template <> struct MetaData<QIcon> { static const char* className() { return "QIcon"; } };
    template <> struct MetaData<QSize> { static const char* className() { return "QSize"; } };
    template <> struct MetaData<QUrl> { static const char* className() { return "QUrl"; } };
    template <> struct MetaData<QVariant> { static const char* className() { return "QVariant"; } };

    template <typename T>
    struct QClass_converters
    {
      struct QClass_to_PyQt
      {
        template <typename Q>
        static typename std::enable_if_t<std::is_copy_constructible_v<Q>, T*> getSafeCopy(T* qClass)
        {
          return new T(*qClass);
        }

        template <typename Q>
        static typename std::enable_if_t<!std::is_copy_constructible_v<Q>, T*> getSafeCopy(T* qClass)
        {
          return qClass;
        }

        static PyObject* convert(const T& object) {
          const sipTypeDef* type = sipAPIAccess::sipAPI()->api_find_type(MetaData<T>::className());
          if (type == nullptr) {
            return bpy::incref(Py_None);
          }

          PyObject* sipObj = sipAPIAccess::sipAPI()->api_convert_from_type((void*)getSafeCopy<T>((T*)&object), type, 0);
          if (sipObj == nullptr) {
            return bpy::incref(Py_None);
          }

          if (std::is_copy_constructible_v<T>)
            // Ensure Python deletes the C++ component
            sipAPIAccess::sipAPI()->api_transfer_back(sipObj);

          return bpy::incref(sipObj);
        }

        static PyObject* convert(T* object) {
          if (object == nullptr) {
            return bpy::incref(Py_None);
          }

          const sipTypeDef* type = sipAPIAccess::sipAPI()->api_find_type(MetaData<T>::className());
          if (type == nullptr) {
            return bpy::incref(Py_None);
          }

          PyObject* sipObj = sipAPIAccess::sipAPI()->api_convert_from_type(getSafeCopy<T>(object), type, 0);
          if (sipObj == nullptr) {
            return bpy::incref(Py_None);
          }

          if (std::is_copy_constructible_v<T>)
            // Ensure Python deletes the C++ component
            sipAPIAccess::sipAPI()->api_transfer_back(sipObj);

          return bpy::incref(sipObj);
        }

        static PyObject* convert(const T* object) {
          return convert((T*)object);
        }

        static PyTypeObject const* get_pytype() {
          const sipTypeDef* type = sipAPIAccess::sipAPI()->api_find_type(MetaData<T>::className());
          if (type == nullptr) {
            return bpy::incref(Py_None);
          }
          return bpy::incref(type->td_py_type);
        }
      };

      static void* QClass_from_PyQt(PyObject* objPtr)
      {
        // This would transfer responsibility for deconstructing the object to C++, but Boost assumes l-value converters (such as this) don't do that
        // Instead, this should be called within the wrappers for functions which return deletable pointers.
        //sipAPI()->api_transfer_to(objPtr, Py_None);
        if (PyObject_TypeCheck(objPtr, sipAPIAccess::sipAPI()->api_simplewrapper_type)) {
          sipSimpleWrapper* wrapper;
          wrapper = reinterpret_cast<sipSimpleWrapper*>(objPtr);
          return wrapper->data;
        }
        else if (PyObject_TypeCheck(objPtr, sipAPIAccess::sipAPI()->api_wrapper_type)) {
          sipWrapper* wrapper;
          wrapper = reinterpret_cast<sipWrapper*>(objPtr);
          return wrapper->super.data;
        }
        return nullptr;
      }
    };

  }

  namespace {
    bool has_arity(PyObject* object, std::size_t arity) {
      // Mostly from https://stackoverflow.com/a/36143796/2666289
      bpy::object fn(bpy::handle<>(bpy::borrowed(object)));

      auto inspect = bpy::import("inspect");
      auto arg_spec = inspect.attr("getfullargspec")(fn);
      bpy::object args = arg_spec.attr("args"),
        varargs = arg_spec.attr("varargs"),
        defaults = arg_spec.attr("defaults");

      auto args_count = args ? bpy::len(args) : 0;
      auto defaults_count = defaults ? bpy::len(defaults) : 0;

      if (static_cast<bool>(inspect.attr("ismethod")(fn)) && fn.attr("__self__")) {
        --args_count;
      }

      auto required_count = args_count - defaults_count;

      return required_count <= arity          // Cannot require more parameters than given,
        && (args_count >= arity || varargs);  // Must accept enough parameters.
    }
  }

  /**
   * @brief Convert a python callable to a valid C++ Callable object. Also works
   *     for None.
   */
  template <typename>
  struct Functor_converter;

  template <typename RET, typename... PARAMS>
  struct Functor_converter<RET(PARAMS...)>
  {

    struct FunctorWrapper
    {
      FunctorWrapper(boost::python::object callable) : m_Callable(callable) {
      }

      ~FunctorWrapper() {
        GILock lock;
        m_Callable = bpy::object();
      }

      RET operator()(const PARAMS&...params) {
        GILock lock;
        if constexpr (std::is_same_v<RET, void>) {
          m_Callable(params...);
        }
        else {
          return bpy::extract<RET>(m_Callable(params...));
        }
      }

      boost::python::object m_Callable;
    };

    static void* convertible(PyObject* object)
    {
      // We allow None here, we will just default-construct a std::function:
      if (object == Py_None) {
        return object;
      }

      // Otherwize we check that we have a callable object:
      if (!PyCallable_Check(object) || !has_arity(object, sizeof...(PARAMS))) {
        return nullptr;
      }
      return object;
    }

    static void construct(PyObject* object, bpy::converter::rvalue_from_python_stage1_data* data)
    {
      bpy::object callable(bpy::handle<>(bpy::borrowed(object)));
      void* storage =((bpy::converter::rvalue_from_python_storage<std::function<RET(PARAMS...)>>*)data)->storage.bytes;
      if (callable.is_none()) {
        new (storage) std::function<RET(PARAMS...)>{};
      }
      else {
        new (storage) std::function<RET(PARAMS...)>(FunctorWrapper(callable));
      }
      data->convertible = storage;
    }
  };



  /**
   * @brief Call policy that automatically downcast shared pointer of type FromType
   * to shared pointer of type ToType.
   */
  template <class FromType, class ToType>
  struct DowncastConverter {

    bool convertible() const { return true; }

    inline PyObject* operator()(std::shared_ptr<FromType> p) const {
      if (p == nullptr) {
        return bpy::detail::none();
      }
      else {
        auto downcast_p = std::dynamic_pointer_cast<ToType>(p);
        bpy::object p_value = downcast_p == nullptr ? bpy::object{ p } : bpy::object{ downcast_p };
        return bpy::incref(p_value.ptr());
      }
    }

    inline PyTypeObject const* get_pytype() const {
      return bpy::converter::registered_pytype<FromType>::get_pytype();
    }

  };

  template <class FromType, class ToType>
  struct downcast_return {

    template <class T>
    struct apply_;

    template <class T>
    struct apply_<std::shared_ptr<T>> {
      static_assert(std::is_convertible_v<std::shared_ptr<T>, std::shared_ptr<FromType>>);
      using type = DowncastConverter<FromType, ToType>;
    };

    template <class T>
    using apply = apply_<std::decay_t<T>>;

  };

  // Functions:
  inline void register_qstring_converter() {
    using namespace QString_converter;
    bpy::to_python_converter<QString, QString_to_python_str>();
    bpy::converter::registry::push_back(
      &QString_from_python_str::convertible, 
      &QString_from_python_str::construct, 
      bpy::type_id<QString>());
  }

  inline void register_qvariant_converter() {
    using namespace QVariant_converter;
    bpy::to_python_converter<QVariant, QVariant_to_python_obj>();
    bpy::converter::registry::push_back(
      &QVariant_from_python_obj::convertible, 
      &QVariant_from_python_obj::construct, 
      bpy::type_id<QVariant>());
  }

  template <class Flags>
  inline void register_qflags_converter() {
    using T = typename Flags::enum_type;
    using namespace QFlags_converter;
    bpy::to_python_converter<Flags, QFlags_to_int<T>>();
    bpy::converter::registry::push_back(
      &QFlags_from_python_obj<T>::convertible,
      &QFlags_from_python_obj<T>::construct,
      bpy::type_id<Flags>());
  }

  template <class QClass>
  inline void register_qclass_converter() {
    using Converter = QClass_converter::QClass_converters<QClass>;
    bpy::converter::registry::insert(&Converter::QClass_from_PyQt, bpy::type_id<QClass>());
    bpy::to_python_converter<const QClass*, typename Converter::QClass_to_PyQt>();
    bpy::to_python_converter<QClass*, typename Converter::QClass_to_PyQt>();
    bpy::to_python_converter<QClass, typename Converter::QClass_to_PyQt>();
  }

  template <class Fn>
  inline void register_functor_converter() {
    using Converter = Functor_converter<Fn>;
    bpy::converter::registry::push_back(
      &Converter::convertible, 
      &Converter::construct, 
      bpy::type_id<std::function<Fn>>());
  }



}

#endif