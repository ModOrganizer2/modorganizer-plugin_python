#include "pythonrunner.h"

#pragma warning( disable : 4100 )
#pragma warning( disable : 4996 )

#include "iplugingame.h"
#include <iplugininstaller.h>
#include <iplugintool.h>
#include <iplugingame.h>
#include <iplugin.h>
#include "uibasewrappers.h"
#include "proxypluginwrappers.h"

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
  QObject *instantiate(const QString &pluginName);
  bool isPythonInstalled() const;
  bool isPythonVersionSupported() const;

private:

  void initPath(boost::python::object &moduleNamespace);

private:
  std::map<QString, boost::python::object> m_PythonObjects;
  const MOBase::IOrganizer *m_MOInfo;
  char *m_PythonHome;
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

struct ModRepositoryFileInfo_to_python_dict
{
  static PyObject *convert(const ModRepositoryFileInfo &info) {
    PyObject *res = PyDict_New();
    PyDict_SetItemString(res, "uri", bpy::incref(bpy::object(info.uri).ptr()));
    PyDict_SetItemString(res, "name", bpy::incref(bpy::object(info.name).ptr()));
    PyDict_SetItemString(res, "description", bpy::incref(bpy::object(info.description.toUtf8().constData()).ptr()));
    PyDict_SetItemString(res, "categoryID", PyLong_FromLong(info.categoryID));
    PyDict_SetItemString(res, "fileID", PyLong_FromLong(info.fileID));
    PyDict_SetItemString(res, "fileSize", PyLong_FromLong(static_cast<long>(info.fileSize)));
    PyDict_SetItemString(res, "version", bpy::incref(bpy::object(info.version).ptr()));
    return bpy::incref(res);
  }
};


struct QString_to_python_str
{
  static PyObject *convert(const QString &str) {
    return bpy::incref(bpy::object(str.toUtf8().constData()).ptr());
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
    return PyString_Check(objPtr) || PyUnicode_Check(objPtr) ? objPtr : nullptr;
  }

  static void construct(PyObject *objPtr, bpy::converter::rvalue_from_python_stage1_data *data) {
    // Ensure the string uses 8-bit characters
    PyObject *strPtr = PyUnicode_Check(objPtr) ? PyUnicode_AsUTF8String(objPtr) : objPtr;
    // Extract the character data from the python string
    const char* value = PyString_AsString(strPtr);
    // Deallocate local copy if one was made
    if (strPtr != objPtr)
      Py_DecRef(strPtr);
    assert(value != nullptr);

    // allocate storage
    void *storage = ((bpy::converter::rvalue_from_python_storage<QString>*)data)->storage.bytes;
    // construct QString in the allocated mr
    new (storage) QString(value);

    data->convertible = storage;
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


struct QVariant_to_python_obj
{
  static PyObject *convert(const QVariant &var) {
    switch (var.type()) {
      case QVariant::Int: return PyLong_FromLong(var.toInt());
      case QVariant::UInt: return PyLong_FromUnsignedLong(var.toUInt());
      case QVariant::Bool: return PyBool_FromLong(var.toBool());
      case QVariant::String: return bpy::incref(bpy::object(var.toString().toUtf8().constData()).ptr());
      case QVariant::List: {
        QVariantList list = var.toList();
        PyObject *result = PyList_New(list.count());
        for (QVariant var : list) {
          PyList_Append(result, convert(var));
        }
        return result;
      } break;
      case QVariant::Map: {
        QVariantMap map = var.toMap();
        PyObject *result = PyDict_New();
        QMapIterator<QString, QVariant> iter(map);
        while (iter.hasNext()) {
          iter.next();
          PyDict_SetItem(result, convert(iter.key()), convert(iter.value()));
        }
        return result;
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
    if (!PyString_Check(objPtr) && !PyInt_Check(objPtr) &&
        !PyBool_Check(objPtr) && !PyList_Check(objPtr)) {
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

  static QVariant variantFromPyObject(PyObject *objPtr) {
    QVariant result;
    if (PyList_Check(objPtr)) {
      QVariantList resultList;
      for (int i = 0; i < PyList_Size(objPtr); ++i) {
        resultList.append(variantFromPyObject(PyList_GetItem(objPtr, i)));
      }
      result = resultList;
    } else if (PyString_Check(objPtr)) {
      result = PyString_AsString(objPtr);
    } else if (PyBool_Check(objPtr)) {
      result = (objPtr == Py_True);
    } else if (PyInt_Check(objPtr)) {
      //QVariant doesn't have long. It has int or long long. Given that on m/s,
      //long is 32 bits for 32- and 64- bit code...
      result = static_cast<int>(PyInt_AsLong(objPtr));
    } else {
      PyErr_SetString(PyExc_TypeError, "type unsupported");
      throw bpy::error_already_set();
    }
    return result;
  }

  static void construct(PyObject *objPtr, bpy::converter::rvalue_from_python_stage1_data *data) {
    // PyBools will also return true for PyInt_Check but not the other way around, so the order
    // here is relevant
    if (PyList_Check(objPtr)) {
      QVariantList result;
      for (int i = 0; i < PyList_Size(objPtr); ++i) {
        result.append(variantFromPyObject(PyList_GetItem(objPtr, i)));
      }
      constructVariant(result, data);
    } else if (PyString_Check(objPtr)) {
      const char *value = PyString_AsString(objPtr);
      constructVariant(value, data);
    } else if (PyBool_Check(objPtr)) {
      bool value = (objPtr == Py_True);
      constructVariant(value, data);
    } else if (PyInt_Check(objPtr)) {
      //QVariant doesn't have long. It has int or long long. Given that on m/s,
      //long is 32 bits for 32- and 64- bit code...
      int value = static_cast<int>(PyInt_AsLong(objPtr));
      constructVariant(value, data);
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
struct QFlags_from_python_obj
{
  QFlags_from_python_obj() {
    bpy::converter::registry::push_back(
      &convertible,
      &construct,
      bpy::type_id<QFlags<T>>());
  }

  static void* convertible(PyObject *objPtr) {
    return PyInt_Check(objPtr) ? objPtr : nullptr;
  }

  static void construct(PyObject *objPtr, bpy::converter::rvalue_from_python_stage1_data *data) {
    int intVersion = (int)PyInt_AsLong(objPtr);
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


static const sipAPIDef *sipAPI()
{
  static const sipAPIDef *sipApi = nullptr;
  if (sipApi == nullptr) {
    sipApi = (const sipAPIDef *)PyCapsule_Import("sip._C_API", 0);
  }

  return sipApi;
}


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
template <> struct MetaData<QIcon> { static const char *className() { return "QIcon"; } };
template <> struct MetaData<QStringList> { static const char *className() { return "QStringList"; } };
template <> struct MetaData<QVariant> { static const char *className() { return "QVariant"; } };


template <typename T>
PyObject *toPyQt(T *objPtr)
{
  if (objPtr == nullptr) {
    qDebug("no input object");
    return bpy::incref(Py_None);
  }
  const sipTypeDef *type = sipAPI()->api_find_type(MetaData<T>::className());

  if (type == nullptr) {
    qDebug("failed to determine type: %s", MetaData<T>::className());
    return bpy::incref(Py_None);
  }

  PyObject *sipObj = sipAPI()->api_convert_from_type(objPtr, type, 0);
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
    static PyObject *convert(const T &object) {
      const sipTypeDef *type = sipAPI()->api_find_type(MetaData<T>::className());
      if (type == nullptr) {
        return bpy::incref(Py_None);
      }

      PyObject *sipObj = sipAPI()->api_convert_from_type((void*)(&object), type, 0);
      if (sipObj == nullptr) {
        return bpy::incref(Py_None);
      }

      return bpy::incref(sipObj);
    }

    static PyObject *convert(T *object) {
      if (object == nullptr) {
        return bpy::incref(Py_None);
      }

      const sipTypeDef *type = sipAPI()->api_find_type(MetaData<T>::className());
      if (type == nullptr) {
        return bpy::incref(Py_None);
      }

      PyObject *sipObj = sipAPI()->api_convert_from_type(object, type, 0);
      if (sipObj == nullptr) {
        return bpy::incref(Py_None);
      }

      return bpy::incref(sipObj);
    }

    static PyObject *convert(const T *object) {
      return convert((T*)object);
    }
  };

  static void *QClass_from_PyQt(PyObject *objPtr)
  {
    if (!PyObject_TypeCheck(objPtr, sipAPI()->api_wrapper_type)) {
      PyErr_SetString(PyExc_TypeError, "type not wrapped");
      bpy::throw_error_already_set();
    }

    sipAPI()->api_transfer_to(objPtr, 0);

    sipSimpleWrapper *wrapper = reinterpret_cast<sipSimpleWrapper*>(objPtr);
    return wrapper->data;
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
      const sipTypeDef *type = sipAPI()->api_find_type(MetaData<T>::className());
      if (type == nullptr) {
        return bpy::incref(Py_None);
      }

      PyObject *sipObj = sipAPI()->api_convert_from_type((void*)(&object), type, 0);
      if (sipObj == nullptr) {
        return bpy::incref(Py_None);
      }

      return bpy::incref(sipObj);
    }

    static PyObject *convert(T *object) {
      if (object == nullptr) {
        return bpy::incref(Py_None);
      }

      const sipTypeDef *type = sipAPI()->api_find_type(MetaData<T>::className());
      if (type == nullptr) {
        return bpy::incref(Py_None);
      }

      PyObject *sipObj = sipAPI()->api_convert_from_type(object, type, 0);
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
    if (!PyObject_TypeCheck(objPtr, sipAPI()->api_wrapper_type)) {
      bpy::throw_error_already_set();
    }

    sipAPI()->api_transfer_to(objPtr, 0);

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
  PyObject *funcCode = PyObject_GetAttrString(object, "func_code");
  if (funcCode) {
    PyObject *argCount = PyObject_GetAttrString(funcCode, "co_argcount");
    if(argCount) {
      result = PyInt_AsLong(argCount);
      Py_DECREF(argCount);
    }
    Py_DECREF(funcCode);
  }
  return result;
}

struct Functor0_converter
{

  struct FunctorWrapper
  {
    FunctorWrapper(boost::python::object callable) : m_Callable(callable) {
    }

    void operator()() {
      GILock lock;
      m_Callable();
    }

    boost::python::object m_Callable;
  };

  Functor0_converter()
  {
    bpy::converter::registry::push_back(&convertible, &construct, bpy::type_id<std::function<void()>>());
  }

  static void *convertible(PyObject *object)
  {
    if (!PyCallable_Check(object)
        || (getArgCount(object) != 0)) {
      return nullptr;
    }
    return object;
  }

  static void construct(PyObject *object, bpy::converter::rvalue_from_python_stage1_data *data)
  {
    bpy::object callable(bpy::handle<>(bpy::borrowed(object)));
    void *storage = ((bpy::converter::rvalue_from_python_storage<std::function<void()>>*)data)->storage.bytes;
    new (storage) std::function<void()>(FunctorWrapper(callable));
    data->convertible = storage;
  }
};


template <typename PAR1, typename PAR2>
struct Functor2_converter
{

  struct FunctorWrapper
  {
    FunctorWrapper(boost::python::object callable) : m_Callable(callable) {
    }

    void operator()(const PAR1 &param1, const PAR2 &param2) {
      GILock lock;
      m_Callable(param1, param2);
    }

    boost::python::object m_Callable;
  };

  Functor2_converter()
  {
    bpy::converter::registry::push_back(&convertible, &construct, bpy::type_id<std::function<void(PAR1, PAR2)>>());
  }

  static void *convertible(PyObject *object)
  {
    if (!PyCallable_Check(object)
        || (getArgCount(object) != 2)) {
      return nullptr;
    }
    return object;
  }

  static void construct(PyObject *object, bpy::converter::rvalue_from_python_stage1_data *data)
  {
    bpy::object callable(bpy::handle<>(bpy::borrowed(object)));
    void *storage = ((bpy::converter::rvalue_from_python_storage<std::function<void(PAR1, PAR2)>>*)data)->storage.bytes;
    new (storage) std::function<void(PAR1, PAR2)>(FunctorWrapper(callable));
    data->convertible = storage;
  }
};


BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(updateWithQuality, MOBase::GuessedValue<QString>::update, 2, 2)


BOOST_PYTHON_MODULE(mobase)
{
  PyEval_InitThreads();
  bpy::to_python_converter<QVariant, QVariant_to_python_obj>();
  QVariant_from_python_obj();

  bpy::to_python_converter<QString, QString_to_python_str>();
  QString_from_python_str();

  //QClass_converters<QObject>();
  QClass_converters<QWidget>();
  QClass_converters<QIcon>();
  QClass_converters<QStringList>();
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

  bpy::enum_<MOBase::IPluginInstaller::EInstallResult>("InstallResult")
      .value("success", MOBase::IPluginInstaller::RESULT_SUCCESS)
      .value("failed", MOBase::IPluginInstaller::RESULT_FAILED)
      .value("canceled", MOBase::IPluginInstaller::RESULT_CANCELED)
      .value("manualRequested", MOBase::IPluginInstaller::RESULT_MANUALREQUESTED)
      .value("notAttempted", MOBase::IPluginInstaller::RESULT_NOTATTEMPTED)
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


  bpy::class_<IOrganizerWrapper, boost::noncopyable>("IOrganizer")
      .def("createNexusBridge", bpy::pure_virtual(&IOrganizer::createNexusBridge), bpy::return_value_policy<bpy::reference_existing_object>())
      .def("profileName", bpy::pure_virtual(&IOrganizer::profileName))
      .def("profilePath", bpy::pure_virtual(&IOrganizer::profilePath))
      .def("downloadsPath", bpy::pure_virtual(&IOrganizer::downloadsPath))
      .def("appVersion", bpy::pure_virtual(&IOrganizer::appVersion))
      .def("getMod", bpy::pure_virtual(&IOrganizer::getMod), bpy::return_value_policy<bpy::reference_existing_object>())
      .def("createMod", bpy::pure_virtual(&IOrganizer::createMod), bpy::return_value_policy<bpy::reference_existing_object>())
      .def("removeMod", bpy::pure_virtual(&IOrganizer::removeMod))
      .def("modDataChanged", bpy::pure_virtual(&IOrganizer::modDataChanged))
      .def("pluginSetting", bpy::pure_virtual(&IOrganizer::pluginSetting))
      .def("setPluginSetting", bpy::pure_virtual(&IOrganizer::pluginSetting))
      .def("persistent", bpy::pure_virtual(&IOrganizer::persistent))
      .def("setPersistent", bpy::pure_virtual(&IOrganizer::setPersistent))
      .def("pluginDataPath", bpy::pure_virtual(&IOrganizer::pluginDataPath))
      .def("installMod", bpy::pure_virtual(&IOrganizer::installMod),(bpy::arg("nameSuggestion")=""), bpy::return_value_policy<bpy::reference_existing_object>())
      .def("downloadManager", bpy::pure_virtual(&IOrganizer::downloadManager), bpy::return_value_policy<bpy::reference_existing_object>())
      .def("pluginList", bpy::pure_virtual(&IOrganizer::pluginList), bpy::return_value_policy<bpy::reference_existing_object>())
      .def("modList", bpy::pure_virtual(&IOrganizer::modList), bpy::return_value_policy<bpy::reference_existing_object>())
      .def("startApplication", bpy::pure_virtual(&IOrganizer::startApplication), bpy::return_value_policy<bpy::return_by_value>())
      //.def("waitForApplication", bpy::pure_virtual(&IOrganizer::waitForApplication), (bpy::arg("exitCode")=nullptr), bpy::return_value_policy<bpy::return_by_value>())
      .def("onAboutToRun", bpy::pure_virtual(&IOrganizer::onAboutToRun))
      .def("onFinishedRun", bpy::pure_virtual(&IOrganizer::onFinishedRun))
      .def("onModInstalled", bpy::pure_virtual(&IOrganizer::onModInstalled))
      .def("refreshModList", bpy::pure_virtual(&IOrganizer::refreshModList))
      .def("profile", bpy::pure_virtual(&IOrganizer::profile), bpy::return_value_policy<bpy::reference_existing_object>())
      .def("managedGame", bpy::pure_virtual(&IOrganizer::managedGame), bpy::return_value_policy<bpy::reference_existing_object>())
      .def("modsSortedByProfilePriority", bpy::pure_virtual(&IOrganizer::modsSortedByProfilePriority))
      ;

  bpy::class_<IProfileWrapper, boost::noncopyable>("IProfile")
      .def("name", bpy::pure_virtual(&IProfile::name))
      .def("absolutePath", bpy::pure_virtual(&IProfile::absolutePath))
      .def("localSavesEnabled", bpy::pure_virtual(&IProfile::localSavesEnabled))
      .def("localSettingsEnabled", bpy::pure_virtual(&IProfile::localSettingsEnabled))
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

  bpy::class_<IPluginWrapper, boost::noncopyable>("IPlugin");

  bpy::class_<IPluginToolWrapper, bpy::bases<IPlugin>, boost::noncopyable>("IPluginTool")
      .def("setParentWidget", bpy::pure_virtual(&MOBase::IPluginTool::setParentWidget))
      ;

  bpy::class_<IPluginInstallerCustomWrapper, boost::noncopyable>("IPluginInstallerCustom")
      .def("setParentWidget", bpy::pure_virtual(&MOBase::IPluginInstallerCustom::setParentWidget))
      ;

  bpy::to_python_converter<IPluginList::PluginStates, QFlags_to_int<IPluginList::PluginState>>();
  QFlags_from_python_obj<IPluginList::PluginState>();
  Functor0_converter(); // converter for the onRefreshed-callback

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
  Functor2_converter<const QString&, IModList::ModStates>(); // converter for the onModStateChanged-callback

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

  bpy::enum_<MOBase::IPluginGame::LoadOrderMechanism>("LoadOrderMechanism")
      .value("FileTime", MOBase::IPluginGame::LoadOrderMechanism::FileTime)
      .value("PluginsTxt", MOBase::IPluginGame::LoadOrderMechanism::PluginsTxt)
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
      .def("gameNexusName", bpy::pure_virtual(&MOBase::IPluginGame::gameNexusName))
      .def("iniFiles", bpy::pure_virtual(&MOBase::IPluginGame::iniFiles))
      .def("DLCPlugins", bpy::pure_virtual(&MOBase::IPluginGame::DLCPlugins))
      .def("CCPlugins", bpy::pure_virtual(&MOBase::IPluginGame::CCPlugins))
      .def("loadOrderMechanism", bpy::pure_virtual(&MOBase::IPluginGame::loadOrderMechanism))
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

      ;

    bpy::class_<QDir>("QDir")
      .def("absolutePath", &QDir::absolutePath)
    ;

  GuessedValue_converters<QString>();

  bpy::to_python_converter<ModRepositoryFileInfo, ModRepositoryFileInfo_to_python_dict>();

  QList_from_python_obj<PluginSetting>();
  bpy::to_python_converter<QList<ModRepositoryFileInfo>,
      QList_to_python_list<ModRepositoryFileInfo> >();

  stdset_from_python_list<QString>();
}


PythonRunner::PythonRunner(const MOBase::IOrganizer *moInfo)
  : m_MOInfo(moInfo)
{
  m_PythonHome = new char[MAX_PATH + 1];
}

static const char *argv0 = "ModOrganizer.exe";


bool PythonRunner::initPython(const QString &pythonPath)
{
  try {
    if (!pythonPath.isEmpty() && !QFile::exists(pythonPath + "/python.exe")) {
      return false;
    }
    strncpy(m_PythonHome, pythonPath.toUtf8().constData(), MAX_PATH);
    if (!pythonPath.isEmpty()) {
      Py_SetPythonHome(m_PythonHome);
    }

    char argBuffer[MAX_PATH];
    strcpy(argBuffer, argv0);

    Py_SetProgramName(argBuffer);
    PyImport_AppendInittab("mobase", &initmobase);
    Py_OptimizeFlag = 2;
    Py_NoSiteFlag = 1;
    Py_InitializeEx(0);

    if (!Py_IsInitialized()) {
      return false;
    }

    PySys_SetArgv(0, (char**)&argBuffer);

    bpy::object mainModule = bpy::import("__main__");
    bpy::object mainNamespace = mainModule.attr("__dict__");
    mainNamespace["sys"] = bpy::import("sys");
    initPath(mainNamespace);
    bpy::import("site");
    mainNamespace["cStringIO"] = bpy::import("cStringIO");
    bpy::exec("s_ErrIO = cStringIO.StringIO()\n"
                        "sys.stderr = s_ErrIO",
                        mainNamespace);
    return true;
  } catch (const bpy::error_already_set&) {
    qDebug("failed to init python");
    PyErr_Print();
    if (PyErr_Occurred()) {
      PyErr_Print();
    } else {
      qCritical("An unexpected C++ exception was thrown in python code");
    }
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
      return res; \
    }\
  } while (false)


void PythonRunner::initPath(bpy::object &moduleNamespace)
{
  static QString paths[] = {
      m_MOInfo->pluginDataPath(),
      QCoreApplication::applicationDirPath(),
      QCoreApplication::applicationDirPath() + "/python27.zip",
      QCoreApplication::applicationDirPath() + "/python27.zip/Lib",
      QCoreApplication::applicationDirPath() + "/python27.zip/DLLs",
      QCoreApplication::applicationDirPath() + "/python27.zip/Lib/site-packages",
    };

  for (int i = 0; i < sizeof(paths) / sizeof(QString); ++i) {
    QString expr = QString("sys.path.insert(%1, \"%2\")").arg(i).arg(paths[i]);
    bpy::eval(expr.toUtf8().constData(), moduleNamespace);
  }
}


QObject *PythonRunner::instantiate(const QString &pluginName)
{
  try {
    GILock lock;
    bpy::object mainModule = bpy::import("__main__");
    bpy::object moduleNamespace = mainModule.attr("__dict__");

    bpy::object sys = bpy::import("sys");
    moduleNamespace["sys"] = sys;
    moduleNamespace["mobase"] = bpy::import("mobase");

    std::string temp = ToString(pluginName);
    if (handled_exec_file(temp.c_str(), moduleNamespace)) {
      reportPythonError();
      return nullptr;
    }
    m_PythonObjects[pluginName] = moduleNamespace["createPlugin"]();

    bpy::object pluginObj = m_PythonObjects[pluginName];
    TRY_PLUGIN_TYPE(IPluginInstallerCustom, pluginObj);
    TRY_PLUGIN_TYPE(IPluginTool, pluginObj);
    TRY_PLUGIN_TYPE(IPluginGame, pluginObj);
  } catch (const bpy::error_already_set&) {
    qWarning("failed to run python script \"%s\"", qPrintable(pluginName));
    reportPythonError();
  }
  return nullptr;
}

bool PythonRunner::isPythonInstalled() const
{
  return Py_IsInitialized() != 0;
}


bool PythonRunner::isPythonVersionSupported() const
{
  const char *version = Py_GetVersion();
  return strstr(version, "2.7") == version;
}

