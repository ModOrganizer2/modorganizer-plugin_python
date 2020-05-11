#include "pythonrunner.h"

#pragma warning( disable : 4100 )
#pragma warning( disable : 4996 )

#include <iplugin.h>
#include <iplugingame.h>
#include <iplugininstaller.h>
#include <iplugintool.h>
#include "uibasewrappers.h"
#include "proxypluginwrappers.h"
#include "gamefeatureswrappers.h"
#include "sipApiAccess.h"

#include <Windows.h>
#include <utility.h>
#include <QDir>
#include <QFile>
#include <QCoreApplication>
#include <QWidget>

// sip and qt slots seems to conflict
#include <sip.h>

#ifndef Q_MOC_RUN
#include <boost/python.hpp>
#endif


MOBase::IOrganizer *s_Organizer = nullptr;



class PythonRunner : public IPythonRunner
{

public:
  PythonRunner(const MOBase::IOrganizer *moInfo);
  bool initPython(const QString &pythonDir);
  QList<QObject*> instantiate(const QString &pluginName);
  bool isPythonInstalled() const;
  bool isPythonVersionSupported() const;

private:

  void initPath();
  
  /**
   * @brief Ensure that the given folder is in sys.path.
   */
  void ensureFolderInPath(QString folder);

private:
  std::map<QString, boost::python::object> m_PythonObjects;
  const MOBase::IOrganizer *m_MOInfo;
  wchar_t *m_PythonHome;
};


IPythonRunner *CreatePythonRunner(MOBase::IOrganizer *moInfo, const QString &pythonDir)
{
  s_Organizer = moInfo;
  PythonRunner *result = new PythonRunner(moInfo);
  if (result->initPython(pythonDir)) {
    return result;
  } else {
    delete result;
    return nullptr;
  }
}


using namespace MOBase;

namespace bpy = boost::python;

struct QString_to_python_str
{
  static PyObject *convert(const QString &str) {
    // It's safer to explicitly convert to unicode as if we don't, this can return either str or unicode without it being easy to know which to expect
    bpy::object pyStr = bpy::object(qUtf8Printable(str));
    if (SIPBytes_Check(pyStr.ptr()))
      pyStr = pyStr.attr("decode")("utf-8");
    return bpy::incref(pyStr.ptr());
  }
};

template <typename T>
struct QFlags_to_int
{
  static PyObject *convert(const QFlags<T> &flags) {
    return bpy::incref(bpy::object(static_cast<int>(flags)).ptr());
  }
};


struct QString_from_python_str
{
  QString_from_python_str() {
    bpy::converter::registry::push_back(&convertible, &construct, bpy::type_id<QString>());
  }

  static void *convertible(PyObject *objPtr) {
    return SIPBytes_Check(objPtr) || PyUnicode_Check(objPtr) ? objPtr : nullptr;
  }

  static void construct(PyObject *objPtr, bpy::converter::rvalue_from_python_stage1_data *data) {
    // Ensure the string uses 8-bit characters
    PyObject *strPtr = PyUnicode_Check(objPtr) ? PyUnicode_AsUTF8String(objPtr) : objPtr;

    // Extract the character data from the python string
    const char* value = SIPBytes_AsString(strPtr);
    assert(value != nullptr);

    // allocate storage
    void *storage = ((bpy::converter::rvalue_from_python_storage<QString>*)data)->storage.bytes;

    // construct QString in the allocated memory
    new (storage) QString(value);

    data->convertible = storage;

    // Deallocate local copy if one was made
    if (strPtr != objPtr)
      Py_DecRef(strPtr);
  }
};


struct HANDLE_converters
{
  struct HANDLE_to_python
  {
    static PyObject *convert(HANDLE handle) {
      size_t size_t_version = (size_t)handle;
      return bpy::incref(bpy::object(size_t_version).ptr());
    }
  };

  // bpy isn't keen on actually using this.
  // maybe it's detecting that the function receives a pointer, and assumes that it needs to convert to the pointer's target.
  // the issue can be worked around by wrapping the function to take a size_t and converting it there
  struct HANDLE_from_python
  {
    HANDLE_from_python() {
      bpy::converter::registry::push_back(&convertible, &construct, bpy::type_id<HANDLE>());
    }

    static void *convertible(PyObject *objPtr) {
      return PyLong_Check(objPtr) ? objPtr : nullptr;
    }

    static void construct(PyObject *objPtr, bpy::converter::rvalue_from_python_stage1_data *data) {
      void *storage = ((bpy::converter::rvalue_from_python_storage<HANDLE>*)data)->storage.bytes;
      HANDLE *result = new (storage) HANDLE;
      *result = (HANDLE)bpy::extract<size_t>(objPtr)();
    }
  };

  HANDLE_converters()
  {
    HANDLE_from_python();
    bpy::to_python_converter<HANDLE, HANDLE_to_python>();
  }
};


template <typename T>
struct GuessedValue_converters
{
  struct GuessedValue_to_python
  {
    static PyObject *convert(const GuessedValue<T> &var) {
      bpy::list result;
      const std::set<T> &values = var.variants();
      for (auto iter = values.begin(); iter != values.end(); ++iter) {
        result.append(bpy::make_tuple(*iter, GUESS_GOOD));
      }
      return bpy::incref(result.ptr());
    }
  };

  struct GuessedValue_from_python
  {
    GuessedValue_from_python() {
      bpy::converter::registry::push_back(&convertible, &construct, bpy::type_id<GuessedValue<T> >());
    }

    static void *convertible(PyObject *objPtr) {
      if PyList_Check(objPtr) {
        return objPtr;
      } else {
        return nullptr;
      }
    }

    static void construct(PyObject *objPtr, bpy::converter::rvalue_from_python_stage1_data* data) {
      void *storage = ((bpy::converter::rvalue_from_python_storage<GuessedValue<T> >*)data)->storage.bytes;
      GuessedValue<T> *result = new (storage) GuessedValue<T>();

      bpy::list source(bpy::handle<>(bpy::borrowed(objPtr)));
      int length = bpy::len(source);
      for (int i = 0; i < length; ++i) {
        bpy::tuple cell = bpy::extract<bpy::tuple>(source[i]);
        result->update(bpy::extract<T>(cell[0]), bpy::extract<EGuessQuality>(cell[1]));
      }

      data->convertible = storage;
    }
  };

  GuessedValue_converters()
  {
    GuessedValue_from_python();
    bpy::to_python_converter<GuessedValue<T>, GuessedValue_to_python>();
  }
};


template<typename key_type, typename value_type>
struct QMap_converters
{
  struct QMap_to_python
  {
    static PyObject *convert(const QMap<key_type, value_type> &map) {
      bpy::dict result;
      QMapIterator<key_type, value_type> iter(map);
      while (iter.hasNext()) {
        iter.next();
        result[bpy::object(iter.key())] = bpy::object(iter.value());
      }
      return bpy::incref(result.ptr());
    }
  };

  struct QMap_from_python
  {
    QMap_from_python() {
      bpy::converter::registry::push_back(&convertible, &construct, bpy::type_id<QMap<key_type, value_type>>());
    }

    static void *convertible(PyObject *objPtr) {
      return PyDict_Check(objPtr) ? objPtr : nullptr;
    }

    static void construct(PyObject *objPtr, bpy::converter::rvalue_from_python_stage1_data *data) {
      void *storage = ((bpy::converter::rvalue_from_python_storage<QMap<key_type, value_type>>*)data)->storage.bytes;
      QMap<key_type, value_type> *result = new (storage) QMap<key_type, value_type>();
      bpy::dict source(bpy::handle<>(bpy::borrowed(objPtr)));
      bpy::list keys = source.keys();
      int len = bpy::len(keys);
      for (int i = 0; i < len; ++i) {
        bpy::object pyKey = keys[i];
        (*result)[bpy::extract<key_type>(pyKey)] = bpy::extract<value_type>(source[pyKey]);
      }

      data->convertible = storage;
    }
  };

  QMap_converters()
  {
    QMap_from_python();
    bpy::to_python_converter<QMap<key_type, value_type>, QMap_to_python >();
  }
};


struct QVariant_to_python_obj
{
  static PyObject *convert(const QVariant &var) {
    switch (var.type()) {
      case QVariant::Invalid: return bpy::incref(Py_None);
      case QVariant::Int: return SIPLong_FromLong(var.toInt());
      case QVariant::UInt: return PyLong_FromUnsignedLong(var.toUInt());
      case QVariant::Bool: return PyBool_FromLong(var.toBool());
      case QVariant::String: return bpy::incref(bpy::object(var.toString()).ptr());
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
  QVariant_from_python_obj() {
    bpy::converter::registry::push_back(&convertible, &construct, bpy::type_id<QVariant>());
  }

  static void *convertible(PyObject *objPtr) {
    if (!SIPBytes_Check(objPtr) && !PyUnicode_Check(objPtr) && !PyLong_Check(objPtr) &&
        !PyBool_Check(objPtr) && !PyList_Check(objPtr) && !PyDict_Check(objPtr) &&
        objPtr != Py_None) {
      return nullptr;
    }
    return objPtr;
  }

  template <typename T>
  static void constructVariant(const T &value, bpy::converter::rvalue_from_python_stage1_data *data) {
    void* storage = ((bpy::converter::rvalue_from_python_storage<QVariant>*)data)->storage.bytes;

    new (storage) QVariant(value);

    data->convertible = storage;
  }

  static void constructVariant(bpy::converter::rvalue_from_python_stage1_data *data) {
    void* storage = ((bpy::converter::rvalue_from_python_storage<QVariant>*)data)->storage.bytes;

    new (storage) QVariant();

    data->convertible = storage;
  }

  static void construct(PyObject *objPtr, bpy::converter::rvalue_from_python_stage1_data *data) {
    // PyBools will also return true for SIPLong_Check but not the other way around, so the order
    // here is relevant
    if (PyList_Check(objPtr)) {
      constructVariant(bpy::extract<QVariantList>(objPtr)(), data);
    } else if (objPtr == Py_None) {
      constructVariant(data);
    } else if (PyDict_Check(objPtr)) {
      constructVariant(bpy::extract<QVariantMap>(objPtr)(), data);
    } else if (SIPBytes_Check(objPtr) || PyUnicode_Check(objPtr)) {
      constructVariant(bpy::extract<QString>(objPtr)(), data);
    } else if (PyBool_Check(objPtr)) {
      constructVariant(bpy::extract<bool>(objPtr)(), data);
    } else if (SIPLong_Check(objPtr)) {
      //QVariant doesn't have long. It has int or long long. Given that on m/s,
      //long is 32 bits for 32- and 64- bit code...
      constructVariant(bpy::extract<int>(objPtr)(), data);
    } else {
      PyErr_SetString(PyExc_TypeError, "type unsupported");
      throw bpy::error_already_set();
    }
  }
};


template <typename T>
struct QList_to_python_list
{
  static PyObject *convert(const QList<T> &list)
  {
    bpy::list pyList;

    try {
      for (const T &item : list) {
        pyList.append(item);
      }
    } catch (const bpy::error_already_set&) {
      reportPythonError();
    }
    PyObject *res = bpy::incref(pyList.ptr());
    return res;
  }
};


template <typename T>
struct QList_from_python_obj
{
  QList_from_python_obj() {
    bpy::converter::registry::push_back(
      &convertible,
      &construct,
      bpy::type_id<QList<T> >());
  }

  static void* convertible(PyObject *objPtr) {
    if (PyList_Check(objPtr)) return objPtr;
    return nullptr;
  }

  static void construct(PyObject *objPtr, bpy::converter::rvalue_from_python_stage1_data *data) {
    void *storage = ((bpy::converter::rvalue_from_python_storage<QList<T> >*)data)->storage.bytes;
    QList<T> *result = new (storage) QList<T>();
    bpy::list source(bpy::handle<>(bpy::borrowed(objPtr)));
    int length = bpy::len(source);
    for (int i = 0; i < length; ++i) {
      result->append(bpy::extract<T>(source[i]));
    }

    data->convertible = storage;
  }
};


template <typename T>
struct std_vector_to_python_list
{
  static PyObject *convert(const std::vector<T> &vector)
  {
    bpy::list pyList;

    try {
      for (const T &item : vector)
        pyList.append(item);
    }
    catch (const bpy::error_already_set&) {
      reportPythonError();
    }

    return bpy::incref(pyList.ptr());
  }
};


template <typename T>
struct std_vector_from_python_obj
{
  std_vector_from_python_obj() {
    bpy::converter::registry::push_back(
      &convertible,
      &construct,
      bpy::type_id<std::vector<T> >());
  }

  static void* convertible(PyObject *objPtr) {
    if (PyList_Check(objPtr)) return objPtr;
    return nullptr;
  }

  static void construct(PyObject *objPtr, bpy::converter::rvalue_from_python_stage1_data *data) {
    void *storage = ((bpy::converter::rvalue_from_python_storage<std::vector<T> >*)data)->storage.bytes;
    std::vector<T> *result = new (storage) std::vector<T>();
    bpy::list source(bpy::handle<>(bpy::borrowed(objPtr)));
    int length = bpy::len(source);
    for (int i = 0; i < length; ++i) {
      result->push_back(bpy::extract<T>(source[i]));
    }

    data->convertible = storage;
  }
};


template <typename T>
struct QFlags_from_python_obj
{
  QFlags_from_python_obj() {
    bpy::converter::registry::push_back(
      &convertible,
      &construct,
      bpy::type_id<QFlags<T>>());
  }

  static void* convertible(PyObject *objPtr) {
    return SIPLong_Check(objPtr) ? objPtr : nullptr;
  }

  static void construct(PyObject *objPtr, bpy::converter::rvalue_from_python_stage1_data *data) {
    int intVersion = (int)SIPLong_AsLong(objPtr);
    T tVersion = (T)intVersion;
    void *storage = ((bpy::converter::rvalue_from_python_storage<QFlags<T>> *)data)->storage.bytes;
    new (storage) QFlags<T>(tVersion);

    data->convertible = storage;
  }
};


template <typename T>
struct stdset_from_python_list
{
  stdset_from_python_list() {
    bpy::converter::registry::push_back(
      &convertible,
      &construct,
      bpy::type_id<std::set<T> >());
  }

  static void* convertible(PyObject *objPtr) {
    if (PyList_Check(objPtr)) return objPtr;
    return nullptr;
  }

  static void construct(PyObject *objPtr, bpy::converter::rvalue_from_python_stage1_data *data) {
    void *storage = ((bpy::converter::rvalue_from_python_storage<std::set<T> >*)data)->storage.bytes;
    std::set<T> *result = new (storage) std::set<T>();

    bpy::list source(bpy::handle<>(bpy::borrowed(objPtr)));
    int length = bpy::len(source);
    for (int i = 0; i < length; ++i) {
      result->insert(bpy::extract<T>(source[i]));
    }

    data->convertible = storage;
  }
};


struct IModRepositoryBridge_to_python
{
  static PyObject *convert(IModRepositoryBridge *bridge)
  {
    ModRepositoryBridgeWrapper wrapper(bridge);

    return bpy::incref(bpy::object(wrapper).ptr());
  }
};



template <typename T> struct MetaData;

template <> struct MetaData<IModRepositoryBridge> { static const char *className() { return "QObject"; } };
template <> struct MetaData<IDownloadManager> { static const char *className() { return "QObject"; } };
template <> struct MetaData<QObject> { static const char *className() { return "QObject"; } };
template <> struct MetaData<QWidget> { static const char *className() { return "QWidget"; } };
template <> struct MetaData<QDateTime> { static const char *className() { return "QDateTime"; } };
template <> struct MetaData<QDir> { static const char *className() { return "QDir"; } };
template <> struct MetaData<QFileInfo> { static const char *className() { return "QFileInfo"; } };
template <> struct MetaData<QIcon> { static const char *className() { return "QIcon"; } };
template <> struct MetaData<QSize> { static const char *className() { return "QSize"; } };
template <> struct MetaData<QStringList> { static const char *className() { return "QStringList"; } };
template <> struct MetaData<QUrl> { static const char *className() { return "QUrl"; } };
template <> struct MetaData<QVariant> { static const char *className() { return "QVariant"; } };


template <typename T>
PyObject *toPyQt(T *objPtr)
{
  if (objPtr == nullptr) {
    qDebug("no input object");
    return bpy::incref(Py_None);
  }
  const sipTypeDef *type = sipAPIAccess::sipAPI()->api_find_type(MetaData<T>::className());

  if (type == nullptr) {
    qDebug("failed to determine type: %s", MetaData<T>::className());
    return bpy::incref(Py_None);
  }

  PyObject *sipObj = sipAPIAccess::sipAPI()->api_convert_from_type(objPtr, type, 0);
  if (sipObj == nullptr) {
    qDebug("failed to convert");
    return bpy::incref(Py_None);
  }
  return bpy::incref(sipObj);
}


template <typename T>
struct QClass_converters
{
  struct QClass_to_PyQt
  {
    template <typename Q>
    static typename std::enable_if_t<std::is_copy_constructible_v<Q>, T*> getSafeCopy(T *qClass)
    {
      return new T(*qClass);
    }

    template <typename Q>
    static typename std::enable_if_t<!std::is_copy_constructible_v<Q>, T*> getSafeCopy(T *qClass)
    {
      return qClass;
    }

    static PyObject *convert(const T &object) {
      const sipTypeDef *type = sipAPIAccess::sipAPI()->api_find_type(MetaData<T>::className());
      if (type == nullptr) {
        return bpy::incref(Py_None);
      }

      PyObject *sipObj = sipAPIAccess::sipAPI()->api_convert_from_type((void*)getSafeCopy<T>((T*)&object), type, 0);
      if (sipObj == nullptr) {
        return bpy::incref(Py_None);
      }

      if (std::is_copy_constructible_v<T>)
        // Ensure Python deletes the C++ component
        sipAPIAccess::sipAPI()->api_transfer_back(sipObj);

      return bpy::incref(sipObj);
    }

    static PyObject *convert(T *object) {
      if (object == nullptr) {
        return bpy::incref(Py_None);
      }

      const sipTypeDef *type = sipAPIAccess::sipAPI()->api_find_type(MetaData<T>::className());
      if (type == nullptr) {
        return bpy::incref(Py_None);
      }

      PyObject *sipObj = sipAPIAccess::sipAPI()->api_convert_from_type(getSafeCopy<T>(object), type, 0);
      if (sipObj == nullptr) {
        return bpy::incref(Py_None);
      }

      if (std::is_copy_constructible_v<T>)
        // Ensure Python deletes the C++ component
        sipAPIAccess::sipAPI()->api_transfer_back(sipObj);

      return bpy::incref(sipObj);
    }

    static PyObject *convert(const T *object) {
      return convert((T*)object);
    }
  };

  static void *QClass_from_PyQt(PyObject *objPtr)
  {
    // This would transfer responsibility for deconstructing the object to C++, but Boost assumes l-value converters (such as this) don't do that
    // Instead, this should be called within the wrappers for functions which return deletable pointers.
    //sipAPI()->api_transfer_to(objPtr, Py_None);
    if (PyObject_TypeCheck(objPtr, sipAPIAccess::sipAPI()->api_simplewrapper_type)) {
      sipSimpleWrapper *wrapper;
      wrapper = reinterpret_cast<sipSimpleWrapper*>(objPtr);
      return wrapper->data;
    } else if (PyObject_TypeCheck(objPtr, sipAPIAccess::sipAPI()->api_wrapper_type)) {
      sipWrapper *wrapper;
      wrapper = reinterpret_cast<sipWrapper*>(objPtr);
      return wrapper->super.data;
    } else {
      if (std::is_same_v<T, QStringList>)
      {
        // QStringLists aren't wrapped by PyQt - regular Python string/unicode lists are used instead
        bpy::extract<QList<QString>> extractor(objPtr);
        if (extractor.check())
          return new QStringList(extractor());
      }
      PyErr_SetString(PyExc_TypeError, "type not wrapped");
      bpy::throw_error_already_set();
    }
    return new void*;
  }

  QClass_converters()
  {
    bpy::converter::registry::insert(&QClass_from_PyQt, bpy::type_id<T>());
    bpy::to_python_converter<const T*, QClass_to_PyQt>();
    bpy::to_python_converter<T*, QClass_to_PyQt>();
    bpy::to_python_converter<T, QClass_to_PyQt>();
  }
};


template <typename T>
struct QInterface_converters
{
  struct QInterface_to_PyQt
  {
    static PyObject *convert(const T &object) {
      const sipTypeDef *type = sipAPIAccess::sipAPI()->api_find_type(MetaData<T>::className());
      if (type == nullptr) {
        return bpy::incref(Py_None);
      }

      PyObject *sipObj = sipAPIAccess::sipAPI()->api_convert_from_type((void*)(&object), type, 0);
      if (sipObj == nullptr) {
        return bpy::incref(Py_None);
      }

      return bpy::incref(sipObj);
    }

    static PyObject *convert(T *object) {
      if (object == nullptr) {
        return bpy::incref(Py_None);
      }

      const sipTypeDef *type = sipAPIAccess::sipAPI()->api_find_type(MetaData<T>::className());
      if (type == nullptr) {
        return bpy::incref(Py_None);
      }

      PyObject *sipObj = sipAPIAccess::sipAPI()->api_convert_from_type(object, type, 0);
      if (sipObj == nullptr) {
        return bpy::incref(Py_None);
      }

      return bpy::incref(sipObj);
    }

    static PyObject *convert(const T *object) {
      return convert((T*)object);
    }
  };

  static void *QInterface_from_PyQt(PyObject *objPtr)
  {
    if (!PyObject_TypeCheck(objPtr, sipAPIAccess::sipAPI()->api_wrapper_type)) {
      bpy::throw_error_already_set();
    }

    // This would transfer responsibility for deconstructing the object to C++, but Boost assumes l-value converters (such as this) don't do that
    // Instead, this should be called within the wrappers for functions which return deletable pointers.
    //sipAPI()->api_transfer_to(objPtr, Py_None);

    sipSimpleWrapper *wrapper = reinterpret_cast<sipSimpleWrapper*>(objPtr);
    return wrapper->data;
  }

  QInterface_converters()
  {
    bpy::converter::registry::insert(&QInterface_from_PyQt, bpy::type_id<T>());
    bpy::to_python_converter<const T*, QInterface_to_PyQt>();
    bpy::to_python_converter<T*, QInterface_to_PyQt>();
  }
};


int getArgCount(PyObject *object) {
  int result = 0;
  PyObject *funcCode = PyObject_GetAttrString(object, "__code__");
  if (funcCode) {
    PyObject *argCount = PyObject_GetAttrString(funcCode, "co_argcount");
    if(argCount) {
      result = SIPLong_AsLong(argCount);
      Py_DECREF(argCount);
    }
    Py_DECREF(funcCode);
  }
  return result;
}


template <typename RET, typename... PARAMS>
struct Functor_converter
{

  struct FunctorWrapper
  {
    FunctorWrapper(boost::python::object callable) : m_Callable(callable) {
    }

    ~FunctorWrapper() {
      GILock lock;
      m_Callable = bpy::object();
    }

    RET operator()(const PARAMS &...params) {
      GILock lock;
      return (RET) m_Callable(params...);
    }

    boost::python::object m_Callable;
  };

  Functor_converter()
  {
    bpy::converter::registry::push_back(&convertible, &construct, bpy::type_id<std::function<RET(PARAMS...)>>());
  }

  static void *convertible(PyObject *object)
  {
    if (!PyCallable_Check(object)
        || (getArgCount(object) != sizeof...(PARAMS))) {
      return nullptr;
    }
    return object;
  }

  static void construct(PyObject *object, bpy::converter::rvalue_from_python_stage1_data *data)
  {
    bpy::object callable(bpy::handle<>(bpy::borrowed(object)));
    void *storage = ((bpy::converter::rvalue_from_python_storage<std::function<RET(PARAMS...)>>*)data)->storage.bytes;
    new (storage) std::function<RET(PARAMS...)>(FunctorWrapper(callable));
    data->convertible = storage;
  }
};


// We must wrap IOrganizer::waitForApplication to convert the out parameter to a return value and also because bpy doesn't like coverting to void* (HANDLE) even if a converter exists.
static PyObject *waitForApplication(const bpy::object &self, size_t handle)
{
  IOrganizer& organizer = bpy::extract<IOrganizer&>(self)();
  DWORD returnCode;
  bool result = organizer.waitForApplication((HANDLE)handle, &returnCode);
  return bpy::incref(bpy::make_tuple(result, returnCode).ptr());
}


BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(updateWithQuality, MOBase::GuessedValue<QString>::update, 2, 2)


BOOST_PYTHON_MODULE(mobase)
{
  PyEval_InitThreads();
  bpy::to_python_converter<QVariant, QVariant_to_python_obj>();
  QVariant_from_python_obj();

  bpy::to_python_converter<QString, QString_to_python_str>();
  QString_from_python_str();

  //QClass_converters<QObject>();
  QClass_converters<QDateTime>();
  QClass_converters<QDir>();
  QClass_converters<QFileInfo>();
  QClass_converters<QWidget>();
  QClass_converters<QIcon>();
  QClass_converters<QSize>();
  QClass_converters<QStringList>();
  QClass_converters<QUrl>();
  QInterface_converters<IDownloadManager>();


  bpy::def("toPyQt", &toPyQt<IModRepositoryBridge>);
  bpy::def("toPyQt", &toPyQt<IDownloadManager>);

  bpy::enum_<MOBase::VersionInfo::ReleaseType>("ReleaseType")
      .value("final", MOBase::VersionInfo::RELEASE_FINAL)
      .value("candidate", MOBase::VersionInfo::RELEASE_CANDIDATE)
      .value("beta", MOBase::VersionInfo::RELEASE_BETA)
      .value("alpha", MOBase::VersionInfo::RELEASE_ALPHA)
      .value("prealpha", MOBase::VersionInfo::RELEASE_PREALPHA)
      ;

  bpy::enum_<MOBase::VersionInfo::VersionScheme>("VersionScheme")
      .value("discover", MOBase::VersionInfo::SCHEME_DISCOVER)
      .value("regular", MOBase::VersionInfo::SCHEME_REGULAR)
      .value("decimalmark", MOBase::VersionInfo::SCHEME_DECIMALMARK)
      .value("numbersandletters", MOBase::VersionInfo::SCHEME_NUMBERSANDLETTERS)
      .value("date", MOBase::VersionInfo::SCHEME_DATE)
      .value("literal", MOBase::VersionInfo::SCHEME_LITERAL)
      ;

  bpy::class_<VersionInfo>("VersionInfo")
      .def(bpy::init<QString>())
      .def(bpy::init<QString, VersionInfo::VersionScheme>())
      .def(bpy::init<int, int, int>())
      .def(bpy::init<int, int, int, VersionInfo::ReleaseType>())
      .def(bpy::init<int, int, int, int>())
      .def(bpy::init<int, int, int, int, VersionInfo::ReleaseType>())
      .def("clear", &VersionInfo::clear)
      .def("parse", &VersionInfo::parse)
      .def("canonicalString", &VersionInfo::canonicalString)
      .def("displayString", &VersionInfo::displayString)
      .def("isValid", &VersionInfo::isValid)
      .def("scheme", &VersionInfo::scheme)
      .def(bpy::self < bpy::self)
      .def(bpy::self > bpy::self)
      .def(bpy::self <= bpy::self)
      .def(bpy::self >= bpy::self)
      .def(bpy::self != bpy::self)
      .def(bpy::self == bpy::self)
      ;

  bpy::class_<PluginSetting>("PluginSetting", bpy::init<const QString&, const QString&, const QVariant&>());

  bpy::class_<ExecutableInfo>("ExecutableInfo", bpy::init<const QString&, const QFileInfo&>())
      .def("withArgument", &ExecutableInfo::withArgument, bpy::return_value_policy<bpy::reference_existing_object>())
      .def("withWorkingDirectory", &ExecutableInfo::withWorkingDirectory, bpy::return_value_policy<bpy::reference_existing_object>())
      .def("withSteamAppId", &ExecutableInfo::withSteamAppId, bpy::return_value_policy<bpy::reference_existing_object>())
      .def("asCustom", &ExecutableInfo::asCustom, bpy::return_value_policy<bpy::reference_existing_object>())
      .def("isValid", &ExecutableInfo::isValid)
      .def("title", &ExecutableInfo::title)
      .def("binary", &ExecutableInfo::binary)
      .def("arguments", &ExecutableInfo::arguments)
      .def("workingDirectory", &ExecutableInfo::workingDirectory)
      .def("steamAppID", &ExecutableInfo::steamAppID)
      .def("isCustom", &ExecutableInfo::isCustom)
      ;

  bpy::class_<ISaveGameWrapper, boost::noncopyable>("ISaveGame")
      .def("getFilename", bpy::pure_virtual(&ISaveGame::getFilename))
      .def("getCreationTime", bpy::pure_virtual(&ISaveGame::getCreationTime))
      .def("getSaveGroupIdentifier", bpy::pure_virtual(&ISaveGame::getSaveGroupIdentifier))
      .def("allFiles", bpy::pure_virtual(&ISaveGame::allFiles))
      .def("hasScriptExtenderFile", bpy::pure_virtual(&ISaveGame::hasScriptExtenderFile))
      ;

  // TODO: ISaveGameInfoWidget bindings

  Functor_converter<bool, const IOrganizer::FileInfo&>();
  Functor_converter<void, const QString&>();
  Functor_converter<bool, const QString&>();
  Functor_converter<void, const QString&, unsigned int>();

  bpy::class_<IOrganizerWrapper, boost::noncopyable>("IOrganizer")
      .def("createNexusBridge", bpy::pure_virtual(&IOrganizer::createNexusBridge), bpy::return_value_policy<bpy::reference_existing_object>())
      .def("profileName", bpy::pure_virtual(&IOrganizer::profileName))
      .def("profilePath", bpy::pure_virtual(&IOrganizer::profilePath))
      .def("downloadsPath", bpy::pure_virtual(&IOrganizer::downloadsPath))
      .def("overwritePath", bpy::pure_virtual(&IOrganizer::overwritePath))
      .def("basePath", bpy::pure_virtual(&IOrganizer::basePath))
      .def("modsPath", bpy::pure_virtual(&IOrganizer::modsPath))
      .def("appVersion", bpy::pure_virtual(&IOrganizer::appVersion))
      .def("getMod", bpy::pure_virtual(&IOrganizer::getMod), bpy::return_value_policy<bpy::reference_existing_object>())
      .def("createMod", bpy::pure_virtual(&IOrganizer::createMod), bpy::return_value_policy<bpy::reference_existing_object>())
      .def("getGame", bpy::pure_virtual(&IOrganizer::getGame), bpy::return_value_policy<bpy::reference_existing_object>())
      .def("removeMod", bpy::pure_virtual(&IOrganizer::removeMod))
      .def("modDataChanged", bpy::pure_virtual(&IOrganizer::modDataChanged))
      .def("pluginSetting", bpy::pure_virtual(&IOrganizer::pluginSetting))
      .def("setPluginSetting", bpy::pure_virtual(&IOrganizer::setPluginSetting))
      .def("persistent", bpy::pure_virtual(&IOrganizer::persistent))
      .def("setPersistent", bpy::pure_virtual(&IOrganizer::setPersistent))
      .def("pluginDataPath", bpy::pure_virtual(&IOrganizer::pluginDataPath))
      .def("installMod", bpy::pure_virtual(&IOrganizer::installMod),(bpy::arg("nameSuggestion")=""), bpy::return_value_policy<bpy::reference_existing_object>())
      .def("resolvePath", bpy::pure_virtual(&IOrganizer::resolvePath))
      .def("listDirectories", bpy::pure_virtual(&IOrganizer::listDirectories))
      .def("findFiles", bpy::pure_virtual(&IOrganizer::findFiles))
      .def("getFileOrigins", bpy::pure_virtual(&IOrganizer::getFileOrigins))
      .def("findFileInfos", bpy::pure_virtual(&IOrganizer::findFileInfos))
      .def("downloadManager", bpy::pure_virtual(&IOrganizer::downloadManager), bpy::return_value_policy<bpy::reference_existing_object>())
      .def("pluginList", bpy::pure_virtual(&IOrganizer::pluginList), bpy::return_value_policy<bpy::reference_existing_object>())
      .def("modList", bpy::pure_virtual(&IOrganizer::modList), bpy::return_value_policy<bpy::reference_existing_object>())
      .def("profile", bpy::pure_virtual(&IOrganizer::profile), bpy::return_value_policy<bpy::reference_existing_object>())
      .def("startApplication", bpy::pure_virtual(&IOrganizer::startApplication), ((bpy::arg("args")=QStringList()), (bpy::arg("cwd")=""), (bpy::arg("profile")=""), (bpy::arg("forcedCustomOverwrite")=""), (bpy::arg("ignoreCustomOverwrite")=false)), bpy::return_value_policy<bpy::return_by_value>())
      //.def("waitForApplication", bpy::pure_virtual(&IOrganizer::waitForApplication), (bpy::arg("exitCode")=nullptr), bpy::return_value_policy<bpy::return_by_value>())
      // Use wrapped version
      .def("waitForApplication", waitForApplication)
      .def("onModInstalled", bpy::pure_virtual(&IOrganizer::onModInstalled))
      .def("onAboutToRun", bpy::pure_virtual(&IOrganizer::onAboutToRun))
      .def("onFinishedRun", bpy::pure_virtual(&IOrganizer::onFinishedRun))
      .def("refreshModList", bpy::pure_virtual(&IOrganizer::refreshModList), (bpy::arg("saveChanges")=true))
      .def("managedGame", bpy::pure_virtual(&IOrganizer::managedGame), bpy::return_value_policy<bpy::reference_existing_object>())
      .def("modsSortedByProfilePriority", bpy::pure_virtual(&IOrganizer::modsSortedByProfilePriority))
      ;

  bpy::class_<IProfileWrapper, boost::noncopyable>("IProfile")
      .def("name", bpy::pure_virtual(&IProfile::name))
      .def("absolutePath", bpy::pure_virtual(&IProfile::absolutePath))
      .def("localSavesEnabled", bpy::pure_virtual(&IProfile::localSavesEnabled))
      .def("localSettingsEnabled", bpy::pure_virtual(&IProfile::localSettingsEnabled))
      .def("invalidationActive", bpy::pure_virtual(&IProfile::invalidationActive))
      ;

  bpy::class_<ModRepositoryBridgeWrapper, boost::noncopyable>("ModRepositoryBridge")
      .def(bpy::init<IModRepositoryBridge*>())
      .def("requestDescription", &ModRepositoryBridgeWrapper::requestDescription)
      .def("requestFiles", &ModRepositoryBridgeWrapper::requestFiles)
      .def("requestFileInfo", &ModRepositoryBridgeWrapper::requestFileInfo)
      .def("requestToggleEndorsement", &ModRepositoryBridgeWrapper::requestToggleEndorsement)
      .def("onFilesAvailable", &ModRepositoryBridgeWrapper::onFilesAvailable)
      .def("onFileInfoAvailable", &ModRepositoryBridgeWrapper::onFileInfoAvailable)
      .def("onDescriptionAvailable", &ModRepositoryBridgeWrapper::onDescriptionAvailable)
      .def("onEndorsementToggled", &ModRepositoryBridgeWrapper::onEndorsementToggled)
      .def("onRequestFailed", &ModRepositoryBridgeWrapper::onRequestFailed)
      ;

  bpy::class_<IModRepositoryBridgeWrapper, boost::noncopyable>("IModRepositoryBridge")
      .def("requestDescription", bpy::pure_virtual(&IModRepositoryBridge::requestDescription))
      .def("requestFiles", bpy::pure_virtual(&IModRepositoryBridge::requestFiles))
      .def("requestFileInfo", bpy::pure_virtual(&IModRepositoryBridge::requestFileInfo))
      .def("requestDownloadURL", bpy::pure_virtual(&IModRepositoryBridge::requestDownloadURL))
      .def("requestToggleEndorsement", bpy::pure_virtual(&IModRepositoryBridge::requestToggleEndorsement))
      ;

  bpy::class_<ModRepositoryFileInfo>("ModRepositoryFileInfo")
      .def(bpy::init<const ModRepositoryFileInfo &>())
      .def(bpy::init<bpy::optional<QString, int, int>>())
      .def("toString", &ModRepositoryFileInfo::toString)
      .def("createFromJson", &ModRepositoryFileInfo::createFromJson).staticmethod("createFromJson")
      .def_readwrite("name", &ModRepositoryFileInfo::name)
      .def_readwrite("uri", &ModRepositoryFileInfo::uri)
      .def_readwrite("description", &ModRepositoryFileInfo::description)
      .def_readwrite("version", &ModRepositoryFileInfo::version)
      .def_readwrite("newestVersion", &ModRepositoryFileInfo::newestVersion)
      .def_readwrite("categoryID", &ModRepositoryFileInfo::categoryID)
      .def_readwrite("modName", &ModRepositoryFileInfo::modName)
      .def_readwrite("gameName", &ModRepositoryFileInfo::gameName)
      .def_readwrite("modID", &ModRepositoryFileInfo::modID)
      .def_readwrite("fileID", &ModRepositoryFileInfo::fileID)
      .def_readwrite("fileSize", &ModRepositoryFileInfo::fileSize)
      .def_readwrite("fileName", &ModRepositoryFileInfo::fileName)
      .def_readwrite("fileCategory", &ModRepositoryFileInfo::fileCategory)
      .def_readwrite("fileTime", &ModRepositoryFileInfo::fileTime)
      .def_readwrite("repository", &ModRepositoryFileInfo::repository)
      .def_readwrite("userData", &ModRepositoryFileInfo::userData)
      ;

  bpy::class_<IDownloadManagerWrapper, boost::noncopyable>("IDownloadManager")
      .def("startDownloadURLs", bpy::pure_virtual(&IDownloadManager::startDownloadURLs))
      .def("startDownloadNexusFile", bpy::pure_virtual(&IDownloadManager::startDownloadNexusFile))
      .def("downloadPath", bpy::pure_virtual(&IDownloadManager::downloadPath))
      ;

  bpy::class_<IInstallationManagerWrapper, boost::noncopyable>("IInstallationManager")
      .def("extractFile", bpy::pure_virtual(&IInstallationManager::extractFile))
      .def("extractFiles", bpy::pure_virtual(&IInstallationManager::extractFiles))
      .def("installArchive", bpy::pure_virtual(&IInstallationManager::installArchive))
      .def("setURL", bpy::pure_virtual(&IInstallationManager::setURL))
      ;

  bpy::class_<IModInterfaceWrapper, boost::noncopyable>("IModInterface")
      .def("name", bpy::pure_virtual(&IModInterface::name))
      .def("absolutePath", bpy::pure_virtual(&IModInterface::absolutePath))
      .def("setVersion", bpy::pure_virtual(&IModInterface::setVersion))
      .def("setNewestVersion", bpy::pure_virtual(&IModInterface::setNewestVersion))
      .def("setIsEndorsed", bpy::pure_virtual(&IModInterface::setIsEndorsed))
      .def("setNexusID", bpy::pure_virtual(&IModInterface::setNexusID))
      .def("addNexusCategory", bpy::pure_virtual(&IModInterface::addNexusCategory))
      .def("addCategory", bpy::pure_virtual(&IModInterface::addCategory))
      .def("removeCategory", bpy::pure_virtual(&IModInterface::removeCategory))
      .def("categories", bpy::pure_virtual(&IModInterface::categories))
      .def("setGameName", bpy::pure_virtual(&IModInterface::setGameName))
      .def("setName", bpy::pure_virtual(&IModInterface::setName))
      .def("remove", bpy::pure_virtual(&IModInterface::remove))
      ;

  bpy::enum_<MOBase::EGuessQuality>("GuessQuality")
      .value("invalid", MOBase::GUESS_INVALID)
      .value("fallback", MOBase::GUESS_FALLBACK)
      .value("good", MOBase::GUESS_GOOD)
      .value("meta", MOBase::GUESS_META)
      .value("preset", MOBase::GUESS_PRESET)
      .value("user", MOBase::GUESS_USER)
      ;

  bpy::class_<MOBase::GuessedValue<QString>, boost::noncopyable>("GuessedString")
      .def("update",
           static_cast<GuessedValue<QString> &(GuessedValue<QString>::*)(const QString&, EGuessQuality)>(&GuessedValue<QString>::update),
           bpy::return_value_policy<bpy::reference_existing_object>(), updateWithQuality())
      .def("variants", &MOBase::GuessedValue<QString>::variants, bpy::return_value_policy<bpy::copy_const_reference>())
      ;

  bpy::to_python_converter<IPluginList::PluginStates, QFlags_to_int<IPluginList::PluginState>>();
  QFlags_from_python_obj<IPluginList::PluginState>();
  Functor_converter<void>(); // converter for the onRefreshed-callback

  bpy::enum_<IPluginList::PluginState>("PluginState")
      .value("missing", IPluginList::STATE_MISSING)
      .value("inactive", IPluginList::STATE_INACTIVE)
      .value("active", IPluginList::STATE_ACTIVE)
      ;

  bpy::class_<IPluginListWrapper, boost::noncopyable>("IPluginList")
      .def("state", bpy::pure_virtual(&MOBase::IPluginList::state))
      .def("priority", bpy::pure_virtual(&MOBase::IPluginList::priority))
      .def("loadOrder", bpy::pure_virtual(&MOBase::IPluginList::loadOrder))
      .def("isMaster", bpy::pure_virtual(&MOBase::IPluginList::isMaster))
      .def("masters", bpy::pure_virtual(&MOBase::IPluginList::masters))
      .def("origin", bpy::pure_virtual(&MOBase::IPluginList::origin))
      .def("onRefreshed", bpy::pure_virtual(&MOBase::IPluginList::onRefreshed))
      .def("onPluginMoved", bpy::pure_virtual(&MOBase::IPluginList::onPluginMoved))
      .def("pluginNames", bpy::pure_virtual(&MOBase::IPluginList::pluginNames))
      .def("setState", bpy::pure_virtual(&MOBase::IPluginList::setState))
      .def("setLoadOrder", bpy::pure_virtual(&MOBase::IPluginList::setLoadOrder))
      ;

  bpy::to_python_converter<IModList::ModStates, QFlags_to_int<IModList::ModState>>();
  QFlags_from_python_obj<IModList::ModState>();
  Functor_converter<void, const QString&, IModList::ModStates>(); // converter for the onModStateChanged-callback

  bpy::enum_<IModList::ModState>("ModState")
      .value("exists", IModList::STATE_EXISTS)
      .value("active", IModList::STATE_ACTIVE)
      .value("essential", IModList::STATE_ESSENTIAL)
      .value("empty", IModList::STATE_EMPTY)
      .value("endorsed", IModList::STATE_ENDORSED)
      .value("valid", IModList::STATE_VALID)
      .value("alternate", IModList::STATE_ALTERNATE)
      ;

  bpy::class_<IModListWrapper, boost::noncopyable>("IModList")
      .def("displayName", bpy::pure_virtual(&MOBase::IModList::displayName))
      .def("allMods", bpy::pure_virtual(&MOBase::IModList::allMods))
      .def("state", bpy::pure_virtual(&MOBase::IModList::state))
      .def("setActive", bpy::pure_virtual(&MOBase::IModList::setActive))
      .def("priority", bpy::pure_virtual(&MOBase::IModList::priority))
      .def("setPriority", bpy::pure_virtual(&MOBase::IModList::setPriority))
      .def("onModStateChanged", bpy::pure_virtual(&MOBase::IModList::onModStateChanged))
      .def("onModMoved", bpy::pure_virtual(&MOBase::IModList::onModMoved))
      ;

  bpy::class_<IPluginWrapper, boost::noncopyable>("IPlugin");

  bpy::class_<IPluginDiagnoseWrapper, boost::noncopyable>("IPluginDiagnose")
      .def("activeProblems", bpy::pure_virtual(&MOBase::IPluginDiagnose::activeProblems))
      .def("shortDescription", bpy::pure_virtual(&MOBase::IPluginDiagnose::shortDescription))
      .def("fullDescription", bpy::pure_virtual(&MOBase::IPluginDiagnose::fullDescription))
      .def("hasGuidedFix", bpy::pure_virtual(&MOBase::IPluginDiagnose::hasGuidedFix))
      .def("startGuidedFix", bpy::pure_virtual(&MOBase::IPluginDiagnose::startGuidedFix))
      .def("_invalidate", &IPluginDiagnoseWrapper::invalidate)
      ;

  bpy::class_<Mapping>("Mapping")
      .def_readwrite("source", &Mapping::source)
      .def_readwrite("destination", &Mapping::destination)
      .def_readwrite("isDirectory", &Mapping::isDirectory)
      .def_readwrite("createTarget", &Mapping::createTarget)
      ;

  bpy::class_<IPluginFileMapperWrapper, boost::noncopyable>("IPluginFileMapper")
      .def("mappings", bpy::pure_virtual(&MOBase::IPluginFileMapper::mappings))
      ;

  bpy::enum_<MOBase::IPluginGame::LoadOrderMechanism>("LoadOrderMechanism")
      .value("FileTime", MOBase::IPluginGame::LoadOrderMechanism::FileTime)
      .value("PluginsTxt", MOBase::IPluginGame::LoadOrderMechanism::PluginsTxt)
      ;

  bpy::enum_<MOBase::IPluginGame::SortMechanism>("SortMechanism")
      .value("NONE", MOBase::IPluginGame::SortMechanism::NONE)
      .value("MLOX", MOBase::IPluginGame::SortMechanism::MLOX)
      .value("BOSS", MOBase::IPluginGame::SortMechanism::BOSS)
      .value("LOOT", MOBase::IPluginGame::SortMechanism::LOOT)
      ;

  // This doesn't actually do the conversion, but might be convenient for accessing the names for enum bits
  bpy::enum_<MOBase::IPluginGame::ProfileSetting>("ProfileSetting")
      .value("mods", MOBase::IPluginGame::MODS)
      .value("configuration", MOBase::IPluginGame::CONFIGURATION)
      .value("savegames", MOBase::IPluginGame::SAVEGAMES)
      .value("preferDefaults", MOBase::IPluginGame::PREFER_DEFAULTS)
      ;

  bpy::to_python_converter<IPluginGame::ProfileSettings, QFlags_to_int<IPluginGame::ProfileSetting>>();
  QFlags_from_python_obj<IPluginGame::ProfileSetting>();

  bpy::class_<IPluginGameWrapper, boost::noncopyable>("IPluginGame")
      .def("gameName", bpy::pure_virtual(&MOBase::IPluginGame::gameName))
      .def("initializeProfile", bpy::pure_virtual(&MOBase::IPluginGame::initializeProfile))
      .def("savegameExtension", bpy::pure_virtual(&MOBase::IPluginGame::savegameExtension))
      .def("savegameSEExtension", bpy::pure_virtual(&MOBase::IPluginGame::savegameSEExtension))
      .def("isInstalled", bpy::pure_virtual(&MOBase::IPluginGame::isInstalled))
      .def("gameIcon", bpy::pure_virtual(&MOBase::IPluginGame::gameIcon))
      .def("gameDirectory", bpy::pure_virtual(&MOBase::IPluginGame::gameDirectory))
      .def("dataDirectory", bpy::pure_virtual(&MOBase::IPluginGame::dataDirectory))
      .def("setGamePath", bpy::pure_virtual(&MOBase::IPluginGame::setGamePath))
      .def("documentsDirectory", bpy::pure_virtual(&MOBase::IPluginGame::documentsDirectory))
      .def("savesDirectory", bpy::pure_virtual(&MOBase::IPluginGame::savesDirectory))
      .def("executables", bpy::pure_virtual(&MOBase::IPluginGame::executables))
      .def("steamAPPId", bpy::pure_virtual(&MOBase::IPluginGame::steamAPPId))
      .def("primaryPlugins", bpy::pure_virtual(&MOBase::IPluginGame::primaryPlugins))
      .def("gameVariants", bpy::pure_virtual(&MOBase::IPluginGame::gameVariants))
      .def("setGameVariant", bpy::pure_virtual(&MOBase::IPluginGame::setGameVariant))
      .def("binaryName", bpy::pure_virtual(&MOBase::IPluginGame::binaryName))
      .def("gameShortName", bpy::pure_virtual(&MOBase::IPluginGame::gameShortName))
      .def("primarySources", bpy::pure_virtual(&MOBase::IPluginGame::primarySources))
      .def("validShortNames", bpy::pure_virtual(&MOBase::IPluginGame::validShortNames))
      .def("gameNexusName", bpy::pure_virtual(&MOBase::IPluginGame::gameNexusName))
      .def("iniFiles", bpy::pure_virtual(&MOBase::IPluginGame::iniFiles))
      .def("DLCPlugins", bpy::pure_virtual(&MOBase::IPluginGame::DLCPlugins))
      .def("CCPlugins", bpy::pure_virtual(&MOBase::IPluginGame::CCPlugins))
      .def("loadOrderMechanism", bpy::pure_virtual(&MOBase::IPluginGame::loadOrderMechanism))
      .def("sortMechanism", bpy::pure_virtual(&MOBase::IPluginGame::sortMechanism))
      .def("nexusModOrganizerID", bpy::pure_virtual(&MOBase::IPluginGame::nexusModOrganizerID))
      .def("nexusGameID", bpy::pure_virtual(&MOBase::IPluginGame::nexusGameID))
      .def("looksValid", bpy::pure_virtual(&MOBase::IPluginGame::looksValid))
      .def("gameVersion", bpy::pure_virtual(&MOBase::IPluginGame::gameVersion))
      .def("getLauncherName", bpy::pure_virtual(&MOBase::IPluginGame::getLauncherName))

      //Plugin interface.
      .def("init", bpy::pure_virtual(&MOBase::IPluginGame::init))
      .def("name", bpy::pure_virtual(&MOBase::IPluginGame::name))
      .def("author", bpy::pure_virtual(&MOBase::IPluginGame::author))
      .def("description", bpy::pure_virtual(&MOBase::IPluginGame::description))
      .def("version", bpy::pure_virtual(&MOBase::IPluginGame::version))
      .def("isActive", bpy::pure_virtual(&MOBase::IPluginGame::isActive))
      .def("settings", bpy::pure_virtual(&MOBase::IPluginGame::settings))

      // The syntax has to differ slightly from C++ because these are templated
      .def("featureBSAInvalidation", &MOBase::IPluginGame::feature<BSAInvalidation>, bpy::return_value_policy<bpy::reference_existing_object>())
      .def("featureDataArchives", &MOBase::IPluginGame::feature<DataArchives>, bpy::return_value_policy<bpy::reference_existing_object>())
      .def("featureGamePlugins", &MOBase::IPluginGame::feature<GamePlugins>, bpy::return_value_policy<bpy::reference_existing_object>())
      .def("featureLocalSavegames", &MOBase::IPluginGame::feature<LocalSavegames>, bpy::return_value_policy<bpy::reference_existing_object>())
      .def("featureSaveGameInfo", &MOBase::IPluginGame::feature<SaveGameInfo>, bpy::return_value_policy<bpy::reference_existing_object>())
      .def("featureScriptExtender", &MOBase::IPluginGame::feature<ScriptExtender>, bpy::return_value_policy<bpy::reference_existing_object>())
      .def("featureUnmanagedMods", &MOBase::IPluginGame::feature<UnmanagedMods>, bpy::return_value_policy<bpy::reference_existing_object>())
      ;

  bpy::enum_<MOBase::IPluginInstaller::EInstallResult>("InstallResult")
      .value("success", MOBase::IPluginInstaller::RESULT_SUCCESS)
      .value("failed", MOBase::IPluginInstaller::RESULT_FAILED)
      .value("canceled", MOBase::IPluginInstaller::RESULT_CANCELED)
      .value("manualRequested", MOBase::IPluginInstaller::RESULT_MANUALREQUESTED)
      .value("notAttempted", MOBase::IPluginInstaller::RESULT_NOTATTEMPTED)
      ;

  bpy::class_<IPluginInstallerCustomWrapper, boost::noncopyable>("IPluginInstallerCustom")
      .def("setParentWidget", bpy::pure_virtual(&MOBase::IPluginInstallerCustom::setParentWidget))
      ;

  bpy::class_<IPluginModPageWrapper, boost::noncopyable>("IPluginModPage")
      .def("setParentWidget", bpy::pure_virtual(&MOBase::IPluginModPage::setParentWidget))
      ;

  bpy::class_<IPluginPreviewWrapper, boost::noncopyable>("IPluginPreview")
      ;

  bpy::class_<IPluginToolWrapper, bpy::bases<IPlugin>, boost::noncopyable>("IPluginTool")
      .def("setParentWidget", bpy::pure_virtual(&MOBase::IPluginTool::setParentWidget))
      ;

  GuessedValue_converters<QString>();

  HANDLE_converters();

  //bpy::to_python_converter<ModRepositoryFileInfo, ModRepositoryFileInfo_to_python_dict>();

  QList_from_python_obj<ExecutableInfo>();
  QList_from_python_obj<PluginSetting>();
  bpy::to_python_converter<QList<ModRepositoryFileInfo>,
      QList_to_python_list<ModRepositoryFileInfo> >();
  QList_from_python_obj<QString>();
  bpy::to_python_converter<QList<QFileInfo>,
      QList_to_python_list<QFileInfo>>();
  QList_from_python_obj<QVariant>();
  bpy::to_python_converter<QList<QVariant>,
      QList_to_python_list<QVariant>>();

  QMap_converters<QString, QVariant>();
  QMap_converters<QString, QStringList>();

  std_vector_from_python_obj<unsigned int>();
  std_vector_from_python_obj<Mapping>();
  bpy::to_python_converter<std::vector<Mapping>,
      std_vector_to_python_list<Mapping>>();

  stdset_from_python_list<QString>();

  registerGameFeaturesPythonConverters();
}


PythonRunner::PythonRunner(const MOBase::IOrganizer *moInfo)
  : m_MOInfo(moInfo)
{
  m_PythonHome = new wchar_t[MAX_PATH + 1];
}

static const char *argv0 = "ModOrganizer.exe";

struct PrintWrapper
{
  void write(const char * message)
  {
    buffer << message;
    if (buffer.tellp() != 0 && buffer.str().back() == '\n')
    {
      // actually put the string in a variable so it doesn't get destroyed as soon as we get a pointer to its data
      std::string string = buffer.str().substr(0, buffer.str().length() - 1);
      qDebug().nospace().noquote() << string.c_str();
      buffer = std::stringstream();
    }
  }

  std::stringstream buffer;
};

// ErrWrapper is in error.h

BOOST_PYTHON_MODULE(moprivate)
{
  bpy::class_<PrintWrapper, boost::noncopyable>("PrintWrapper", bpy::init<>())
    .def("write", &PrintWrapper::write);
  bpy::class_<ErrWrapper, boost::noncopyable>("ErrWrapper", bpy::init<>())
    .def("instance", &ErrWrapper::instance, bpy::return_value_policy<bpy::reference_existing_object>()).staticmethod("instance")
    .def("write", &ErrWrapper::write)
    .def("startRecordingExceptionMessage", &ErrWrapper::startRecordingExceptionMessage)
    .def("stopRecordingExceptionMessage", &ErrWrapper::stopRecordingExceptionMessage)
    .def("getLastExceptionMessage", &ErrWrapper::getLastExceptionMessage);
}

bool PythonRunner::initPython(const QString &pythonPath)
{
  if (Py_IsInitialized())
    return true;
  try {
    if (!pythonPath.isEmpty() && !QFile::exists(pythonPath + "/python.exe")) {
      return false;
    }
    pythonPath.toWCharArray(m_PythonHome);
    if (!pythonPath.isEmpty()) {
      Py_SetPythonHome(m_PythonHome);
    }

    wchar_t argBuffer[MAX_PATH];
    const size_t cSize = strlen(argv0) + 1;
    mbstowcs(argBuffer, argv0, MAX_PATH);

    Py_SetProgramName(argBuffer);
    PyImport_AppendInittab("mobase", &PyInit_mobase);
    PyImport_AppendInittab("moprivate", &PyInit_moprivate);
    Py_OptimizeFlag = 2;
    Py_NoSiteFlag = 1;
    initPath();
    Py_InitializeEx(0);

    if (!Py_IsInitialized()) {
      if (PyGILState_Check())
        PyEval_SaveThread();
      return false;
    }

    PySys_SetArgv(0, (wchar_t**)&argBuffer);

    bpy::object mainModule = bpy::import("__main__");
    bpy::object mainNamespace = mainModule.attr("__dict__");
    mainNamespace["sys"] = bpy::import("sys");
    mainNamespace["moprivate"] = bpy::import("moprivate");
    bpy::import("site");
    bpy::exec("sys.stdout = moprivate.PrintWrapper()\n"
              "sys.stderr = moprivate.ErrWrapper.instance()\n"
              "sys.excepthook = lambda x, y, z: sys.__excepthook__(x, y, z)\n",
                        mainNamespace);

    PyEval_SaveThread();
    return true;
  } catch (const bpy::error_already_set&) {
    qDebug("failed to init python");
    PyErr_Print();
    if (PyErr_Occurred()) {
      PyErr_Print();
    } else {
      qCritical("An unexpected C++ exception was thrown in python code");
    }
    if (PyGILState_Check())
      PyEval_SaveThread();
    return false;
  }
}


bool handled_exec_file(bpy::str filename, bpy::object globals = bpy::object(), bpy::object locals = bpy::object())
{
  return bpy::handle_exception(std::bind<bpy::object(&)(bpy::str, bpy::object, bpy::object)>(bpy::exec_file, filename, globals, locals));
}


#define TRY_PLUGIN_TYPE(type, var) do { \
    bpy::extract<type *> extr(var); \
    if (extr.check()) { \
      QObject *res = extr; \
      interfaceList.append(res); \
    }\
  } while (false)


void PythonRunner::initPath()
{
  static QStringList paths = {
    QCoreApplication::applicationDirPath() + "/pythoncore.zip",
    QCoreApplication::applicationDirPath() + "/pythoncore",
    m_MOInfo->pluginDataPath()
  };

  Py_SetPath(paths.join(';').toStdWString().c_str());
}

void PythonRunner::ensureFolderInPath(QString folder) {
  bpy::object sys = bpy::import("sys");
  bpy::list sysPath = bpy::extract<bpy::list>(sys.attr("path"));

  // Converting to QStringList for Qt::CaseInsensitive and because .index()
  // raise an exception:
  QStringList currentPath = bpy::extract<QStringList>(sysPath);
  if (!currentPath.contains(folder, Qt::CaseInsensitive)) {
    sysPath.insert(0, folder);
  }
}

QList<QObject*> PythonRunner::instantiate(const QString &pluginName)
{
  try {
    GILock lock;
    bpy::object mainModule = bpy::import("__main__");
    bpy::object moduleNamespace = mainModule.attr("__dict__");

    bpy::object sys = bpy::import("sys");
    moduleNamespace["sys"] = sys;
    moduleNamespace["mobase"] = bpy::import("mobase");

    if (pluginName.endsWith(".py")) {
      std::string temp = ToString(pluginName);
      if (handled_exec_file(temp.c_str(), moduleNamespace)) {
        reportPythonError();
        return QList<QObject*>();
      }
      m_PythonObjects[pluginName] = moduleNamespace["createPlugin"]();
    }
    else {
      // Retrieve the module name:
      QStringList parts = pluginName.split("/");
      std::string moduleName = ToString(parts.takeLast());
      ensureFolderInPath(parts.join("/"));
      bpy::object createPlugin = bpy::import(moduleName.c_str()).attr("createPlugin");
      m_PythonObjects[pluginName] = createPlugin();
    }

    bpy::object pluginObj = m_PythonObjects[pluginName];
    QList<QObject *> interfaceList;
    TRY_PLUGIN_TYPE(IPluginGame, pluginObj);
    // Must try the wrapper because it's only a plugin extension interface in C++, so doesn't extend QObject
    TRY_PLUGIN_TYPE(IPluginDiagnoseWrapper, pluginObj);
    // Must try the wrapper because it's only a plugin extension interface in C++, so doesn't extend QObject
    TRY_PLUGIN_TYPE(IPluginFileMapperWrapper, pluginObj);
    TRY_PLUGIN_TYPE(IPluginInstallerCustom, pluginObj);
    TRY_PLUGIN_TYPE(IPluginModPage, pluginObj);
    TRY_PLUGIN_TYPE(IPluginPreview, pluginObj);
    TRY_PLUGIN_TYPE(IPluginTool, pluginObj);
    if (interfaceList.isEmpty())
      TRY_PLUGIN_TYPE(IPluginWrapper, pluginObj);

    return interfaceList;
  } catch (const bpy::error_already_set&) {
    qWarning("failed to run python script \"%s\"", qUtf8Printable(pluginName));
    reportPythonError();
  }
  return QList<QObject*>();
}

bool PythonRunner::isPythonInstalled() const
{
  return Py_IsInitialized() != 0;
}


bool PythonRunner::isPythonVersionSupported() const
{
  const char *version = Py_GetVersion();
  return strstr(version, "3.7") == version;
}

