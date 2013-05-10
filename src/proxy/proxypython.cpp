#pragma warning( push )
#pragma warning( disable : 4100 ) // a lot of unreferenced formal parameters from boost libraries

#include "proxypython.h"

#include <boost/python.hpp>

#pragma warning( pop )

#include <utility.h>
#include <versioninfo.h>
#include <QtPlugin>
#include <QDirIterator>
#include <QWidget>
#include "proxypluginwrappers.h"
#include "uibasewrappers.h"

// sip and qt slots seems to conflict
#include <sip.h>


using namespace MOBase;
namespace bpy = boost::python;


static void reportPythonError()
{
  if (PyErr_Occurred()) {
    // prints to s_ErrIO buffer
    PyErr_Print();
    // extract data from python buffer
    bpy::object mainModule = bpy::import("__main__");
    bpy::object mainNamespace = mainModule.attr("__dict__");
    bpy::object errMsgObj = bpy::eval("s_ErrIO.getvalue()", mainNamespace);
    QString errMsg = bpy::extract<QString>(errMsgObj.ptr());
    bpy::eval("s_ErrIO.truncate(0)", mainNamespace);
    throw MyException(errMsg);
  } else {
    throw MyException("An unexpected C++ exception was thrown in python code");
  }
}


struct QString_to_python_str
{
  static PyObject *convert(QString const& str) {
    return bpy::incref(bpy::object(str.toUtf8().constData()).ptr());
  }
};

struct QString_from_python_str
{
  QString_from_python_str() {
    bpy::converter::registry::push_back(&convertible, &construct, bpy::type_id<QString>());
  }

  static void *convertible(PyObject *objPtr) {
    return PyString_Check(objPtr) ? objPtr : NULL;
  }

  static void construct(PyObject *objPtr, bpy::converter::rvalue_from_python_stage1_data *data) {
    // Extract the character data from the python string
    const char* value = PyString_AsString(objPtr);
    assert(value != NULL);

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
        return NULL;
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
      case QVariant::Bool: return PyBool_FromLong(var.toBool());
      case QVariant::String: return bpy::incref(bpy::object(var.toString().toUtf8().constData()).ptr());
      case QVariant::List: {
        qDebug("to list convert");
        QVariantList list = var.toList();
        PyObject *result = PyList_New(list.count());
        foreach (QVariant var, list) {
          PyList_Append(result, convert(var));
        }
        return result;
      } break;
      default: {
        PyErr_SetString(PyExc_TypeError, "type unsupported");
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
      return NULL;
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
      result = PyInt_AsLong(objPtr);
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
      long value = PyInt_AsLong(objPtr);
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
    qDebug("convert list");

    boost::python::list pyList;
    foreach (const T &item, list) {
      pyList.append(item);
    }
    return bpy::incref(pyList.ptr());
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
    return NULL;
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
    return NULL;
  }

  static void construct(PyObject *objPtr, bpy::converter::rvalue_from_python_stage1_data *data) {
    void *storage = ((bpy::converter::rvalue_from_python_storage<std::set<T> >*)data)->storage.bytes;
    std::set<T> *result = new (storage) std::set<T>();

    bpy::list source(bpy::handle<>(bpy::borrowed(objPtr)));
    int length = bpy::len(source);
    try {
      for (int i = 0; i < length; ++i) {
        result->insert(bpy::extract<T>(source[i]));
      }
    } catch (const bpy::error_already_set&) {
      qDebug("bla");
    }

    data->convertible = storage;
  }
};


static const sipAPIDef *sipAPI()
{
  static const sipAPIDef *sipApi = NULL;
  if (sipApi == NULL) {
    sipApi = (const sipAPIDef *)PyCapsule_Import("sip._C_API", 0);
  }

  return sipApi;
}


template <typename T> struct MetaData;

template <> struct MetaData<IModRepositoryBridge> { static const char *className() { return "MOBase::INexusBridge"; } };
template <> struct MetaData<QObject> { static const char *className() { return "QObject"; } };
template <> struct MetaData<QWidget> { static const char *className() { return "QWidget"; } };
template <> struct MetaData<QIcon> { static const char *className() { return "QIcon"; } };
template <> struct MetaData<QVariant> { static const char *className() { return "QVariant"; } };
//template <> struct MetaData<QList<MOBase::NexusFileInfo> > { static const char *className() { return "QList<NexusFileInfo>"; } };


template <typename T>
PyObject *toPyQt(T *objPtr)
{
  if (objPtr == NULL) {
    qDebug("no input object");
    return bpy::incref(Py_None);
  }

  const sipTypeDef *type = sipAPI()->api_find_type(MetaData<T>::className());

  if (type == NULL) {
    qDebug("failed to determine type");
    return bpy::incref(Py_None);
  }

  PyObject *sipObj = sipAPI()->api_convert_from_type(objPtr, type, 0);
  if (sipObj == NULL) {
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
      if (type == NULL) {
        return bpy::incref(Py_None);
      }

      PyObject *sipObj = sipAPI()->api_convert_from_type((void*)(&object), type, 0);
      if (sipObj == NULL) {
        return bpy::incref(Py_None);
      }

      return bpy::incref(sipObj);
    }

    static PyObject *convert(T *object) {
      if (object == NULL) {
        return bpy::incref(Py_None);
      }

      const sipTypeDef *type = sipAPI()->api_find_type(MetaData<T>::className());
      if (type == NULL) {
        return bpy::incref(Py_None);
      }

      PyObject *sipObj = sipAPI()->api_convert_from_type(object, type, 0);
      if (sipObj == NULL) {
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
      if (type == NULL) {
        return bpy::incref(Py_None);
      }

      PyObject *sipObj = sipAPI()->api_convert_from_type((void*)(&object), type, 0);
      if (sipObj == NULL) {
        return bpy::incref(Py_None);
      }

      return bpy::incref(sipObj);
    }

    static PyObject *convert(T *object) {
      if (object == NULL) {
        return bpy::incref(Py_None);
      }

      const sipTypeDef *type = sipAPI()->api_find_type(MetaData<T>::className());
      if (type == NULL) {
        return bpy::incref(Py_None);
      }

      PyObject *sipObj = sipAPI()->api_convert_from_type(object, type, 0);
      if (sipObj == NULL) {
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


BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(updateWithQuality, MOBase::GuessedValue<QString>::update, 2, 2)


BOOST_PYTHON_MODULE(mobase)
{
  bpy::to_python_converter<QVariant, QVariant_to_python_obj>();
  QVariant_from_python_obj();

  bpy::to_python_converter<QString, QString_to_python_str>();
  QString_from_python_str();

  QClass_converters<QObject>();
  QClass_converters<QWidget>();
  //QClass_converters<QVariant>();
  QClass_converters<QIcon>();
  QInterface_converters<IModRepositoryBridge>();

  bpy::def("toPyQt", &toPyQt<IModRepositoryBridge>);

  bpy::enum_<MOBase::VersionInfo::ReleaseType>("ReleaseType")
      .value("final", MOBase::VersionInfo::RELEASE_FINAL)
      .value("candidate", MOBase::VersionInfo::RELEASE_CANDIDATE)
      .value("beta", MOBase::VersionInfo::RELEASE_BETA)
      .value("alpha", MOBase::VersionInfo::RELEASE_ALPHA)
      .value("prealpha", MOBase::VersionInfo::RELEASE_PREALPHA);

  bpy::enum_<MOBase::IPluginInstaller::EInstallResult>("InstallResult")
      .value("success", MOBase::IPluginInstaller::RESULT_SUCCESS)
      .value("failed", MOBase::IPluginInstaller::RESULT_FAILED)
      .value("canceled", MOBase::IPluginInstaller::RESULT_CANCELED)
      .value("manualRequested", MOBase::IPluginInstaller::RESULT_MANUALREQUESTED)
      .value("notAttempted", MOBase::IPluginInstaller::RESULT_NOTATTEMPTED);

  bpy::enum_<MOBase::IGameInfo::Type>("GameType")
      .value("oblivion", MOBase::IGameInfo::TYPE_OBLIVION)
      .value("fallout3", MOBase::IGameInfo::TYPE_FALLOUT3)
      .value("falloutnv", MOBase::IGameInfo::TYPE_FALLOUTNV)
      .value("skyrim", MOBase::IGameInfo::TYPE_SKYRIM);

  bpy::class_<MOBase::VersionInfo>("VersionInfo")
      .def(bpy::init<int, int, int, MOBase::VersionInfo::ReleaseType>())
      .def("parse", &MOBase::VersionInfo::parse)
      .def("canonicalString", &MOBase::VersionInfo::canonicalString);

  bpy::class_<MOBase::PluginSetting>("PluginSetting", bpy::init<const QString&, const QString&, const QVariant&>());

  bpy::class_<IGameInfoWrapper, boost::noncopyable>("GameInfo")
      .def("type", bpy::pure_virtual(&MOBase::IGameInfo::type))
      .def("path", bpy::pure_virtual(&MOBase::IGameInfo::path))
      .def("binaryName", bpy::pure_virtual(&MOBase::IGameInfo::binaryName));

  bpy::class_<IOrganizerWrapper, boost::noncopyable>("IOrganizer")
      .def("gameInfo", bpy::pure_virtual(&MOBase::IOrganizer::gameInfo), bpy::return_value_policy<bpy::reference_existing_object>())
      .def("createNexusBridge", bpy::pure_virtual(&MOBase::IOrganizer::createNexusBridge), bpy::return_value_policy<bpy::reference_existing_object>())
      .def("profileName", bpy::pure_virtual(&MOBase::IOrganizer::profileName))
      .def("profilePath", bpy::pure_virtual(&MOBase::IOrganizer::profilePath))
      .def("downloadsPath", bpy::pure_virtual(&MOBase::IOrganizer::downloadsPath))
      .def("appVersion", bpy::pure_virtual(&MOBase::IOrganizer::appVersion))
      .def("getMod", bpy::pure_virtual(&MOBase::IOrganizer::getMod), bpy::return_value_policy<bpy::reference_existing_object>())
      .def("createMod", bpy::pure_virtual(&MOBase::IOrganizer::createMod), bpy::return_value_policy<bpy::reference_existing_object>())
      .def("removeMod", bpy::pure_virtual(&MOBase::IOrganizer::removeMod))
      .def("modDataChanged", bpy::pure_virtual(&MOBase::IOrganizer::modDataChanged))
      .def("pluginSetting", bpy::pure_virtual(&IOrganizer::pluginSetting))
      .def("pluginDataPath", bpy::pure_virtual(&IOrganizer::pluginDataPath));

  bpy::class_<INexusBridgeWrapper, boost::noncopyable>("INexusBridge")
      .def("requestDescription", bpy::pure_virtual(&MOBase::IModRepositoryBridge::requestDescription))
      .def("requestFiles", bpy::pure_virtual(&MOBase::IModRepositoryBridge::requestFiles))
      .def("requestFileInfo", bpy::pure_virtual(&MOBase::IModRepositoryBridge::requestFileInfo))
      .def("requestDownloadURL", bpy::pure_virtual(&MOBase::IModRepositoryBridge::requestDownloadURL))
      .def("requestToggleEndorsement", bpy::pure_virtual(&MOBase::IModRepositoryBridge::requestToggleEndorsement))
      ;

  bpy::enum_<MOBase::EGuessQuality>("GuessQuality")
      .value("invalid", MOBase::GUESS_INVALID)
      .value("fallback", MOBase::GUESS_FALLBACK)
      .value("good", MOBase::GUESS_GOOD)
      .value("meta", MOBase::GUESS_META)
      .value("preset", MOBase::GUESS_PRESET)
      .value("user", MOBase::GUESS_USER);

  bpy::class_<MOBase::GuessedValue<QString>, boost::noncopyable>("GuessedString")
      .def("update",
           static_cast<GuessedValue<QString> &(GuessedValue<QString>::*)(const QString&, EGuessQuality)>(&GuessedValue<QString>::update),
           bpy::return_value_policy<bpy::reference_existing_object>(), updateWithQuality())
      .def("variants", &MOBase::GuessedValue<QString>::variants, bpy::return_value_policy<bpy::copy_const_reference>());

  bpy::class_<IPluginToolWrapper, boost::noncopyable>("IPluginTool")
      .def("setParentWidget", bpy::pure_virtual(&MOBase::IPluginTool::setParentWidget));
  bpy::class_<IPluginInstallerCustomWrapper, boost::noncopyable>("IPluginInstallerCustom")
      .def("setParentWidget", bpy::pure_virtual(&MOBase::IPluginInstallerCustom::setParentWidget));

  bpy::class_<ModRepositoryFileInfo, boost::noncopyable>("NexusFileInfo")
      .def_readonly("name", &ModRepositoryFileInfo::name)
      .def_readonly("uri", &ModRepositoryFileInfo::uri);
/*      QString name;
      QString uri;
      VersionInfo version;
      int categoryID;
      int fileID;*/

  GuessedValue_converters<QString>();

  QList_from_python_obj<PluginSetting>();
  bpy::to_python_converter<QList<ModRepositoryFileInfo*>,
      QList_to_python_list<ModRepositoryFileInfo*> >();

  stdset_from_python_list<QString>();
}


ProxyPython::ProxyPython()
  : m_MOInfo(NULL)
{
  try {
    PyImport_AppendInittab("mobase", &initmobase);
    Py_Initialize();
    qDebug("Python: %s", Py_GetVersion());

    static char* argv0 = "Banana?";
    PySys_SetArgv(0, &argv0);

    bpy::object main_module = bpy::import("__main__");
    bpy::object main_namespace = main_module.attr("__dict__");
    main_namespace["cStringIO"] = bpy::import("cStringIO");
    main_namespace["sys"] = bpy::import("sys");
    bpy::exec("s_ErrIO = cStringIO.StringIO()\n"
                        "sys.stderr = s_ErrIO",
                        main_namespace);
  } catch (const bpy::error_already_set&) {
    qDebug("failed to init python");
    PyErr_Print();
    if (PyErr_Occurred()) {
      PyErr_Print();
    } else {
      qCritical("An unexpected C++ exception was thrown in python code");
    }
  }
}

bool ProxyPython::init(IOrganizer *moInfo)
{
  m_MOInfo = moInfo;
  return true;
}

QString ProxyPython::name() const
{
  return tr("Python Proxy");
}

QString ProxyPython::author() const
{
  return "Tannin";
}

QString ProxyPython::description() const
{
  return tr("Proxy Plugin to allow plugins written in python to be loaded");
}

VersionInfo ProxyPython::version() const
{
  return VersionInfo(1, 0, 0, VersionInfo::RELEASE_FINAL);
}

bool ProxyPython::isActive() const
{
  return true;
}

QList<PluginSetting> ProxyPython::settings() const
{
  return QList<PluginSetting>();
}

QStringList ProxyPython::pluginList(const QString &pluginPath) const
{
  QDirIterator iter(pluginPath, QStringList("*.py"));

  QStringList result;
  while (iter.hasNext()) {
    result.append(iter.next());
  }

  return result;
}


bool handled_exec_file(bpy::str filename, bpy::object globals = bpy::object(), bpy::object locals = bpy::object())
{
  return bpy::handle_exception(boost::bind(bpy::exec_file, filename, globals, locals));
}


#define TRY_PLUGIN_TYPE(type, var) do { \
    bpy::extract<type ## *> extr(var); \
    if (extr.check()) { \
      QObject *res = extr; \
      return res; \
    }\
  } while (false)


QObject *ProxyPython::instantiate(const QString &pluginName)
{
  try {
    bpy::object main_module = bpy::import("__main__");
    bpy::object main_namespace = main_module.attr("__dict__");

    bpy::object mobase_module((bpy::handle<>(PyImport_ImportModule("mobase"))));
    main_namespace["mobase"] = mobase_module;

    QString appendDataPath = QString("sys.path.append(\"%1\")").arg(m_MOInfo->pluginDataPath());

    bpy::eval(appendDataPath.toUtf8().constData(), main_namespace);

    main_namespace["interfaces"] = bpy::import("interfaces");

    std::string temp = ToString(pluginName);
    if (handled_exec_file(temp.c_str(), main_namespace)) {
      reportPythonError();
      return NULL;
    }
    m_PythonObjects[pluginName] = main_namespace["createPlugin"]();

    bpy::object pluginObj = m_PythonObjects[pluginName];
    TRY_PLUGIN_TYPE(IPluginInstallerCustom, pluginObj);
    TRY_PLUGIN_TYPE(IPluginTool, pluginObj);
  } catch (const std::exception &e) {
    qWarning("failed to run python script \"%s\": %s", qPrintable(pluginName), e.what());
  } catch (const bpy::error_already_set&) {
    qWarning("failed to run python script \"%s\"", qPrintable(pluginName));
    reportPythonError();
  }
  return NULL;
}


Q_EXPORT_PLUGIN2(proxyPython, ProxyPython)
