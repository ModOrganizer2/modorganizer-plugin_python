#include "pythonrunner.h"

#pragma warning( disable : 4100 )
#pragma warning( disable : 4996 )

#include <ifiletree.h>
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

#include <variant>
#include <tuple>

#ifndef Q_MOC_RUN
#include <boost/python.hpp>
#include <boost/mp11.hpp>

#include "tuple_helper.h"
#include "variant_helper.h"
#include "pythonutils.h"
#endif

MOBase::IOrganizer *s_Organizer = nullptr;



using namespace MOBase;

namespace bpy = boost::python;
namespace mp11 = boost::mp11;


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
   * @brief Append the underlying object of the given python object to the
   *     interface list if it is an instance (pointer) of the given type.
   *
   * @param obj The object to check.
   * @param interfaces The list to append the object to.
   *
   */
  template <class T>
  void appendIfInstance(bpy::object const& obj, QList<QObject*> &interfaces);

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
      if constexpr (std::is_same_v<T, QStringList>)
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

template <typename>
struct Functor_converter;


template <typename RET, typename... PARAMS>
struct Functor_converter<RET(PARAMS... )>
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
      if constexpr (std::is_same_v<RET, void>) {
        m_Callable(params...);
      }
      else {
        return bpy::extract<RET>(m_Callable(params...));
      }
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
struct DowncastReturn {

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


BOOST_PYTHON_MODULE(mobase)
{
  PyEval_InitThreads();

  bpy::import("PyQt5.QtCore");

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

  // Pointers:
  bpy::register_ptr_to_python<std::shared_ptr<FileTreeEntry>>();
  bpy::register_ptr_to_python<std::shared_ptr<const FileTreeEntry>>();
  bpy::register_ptr_to_python<std::shared_ptr<IFileTree>>();
  bpy::register_ptr_to_python<std::shared_ptr<const IFileTree>>();

  // Containers:
  utils::register_sequence_container<QList<ExecutableInfo>>();
  utils::register_sequence_container<QList<PluginSetting>>();
  utils::register_sequence_container<QList<ModRepositoryFileInfo>>();
  utils::register_sequence_container<QList<QString>>();
  utils::register_sequence_container<QList<QFileInfo>>();
  utils::register_sequence_container<QList<QVariant>>();

  utils::register_sequence_container<std::vector<unsigned int>>();
  utils::register_sequence_container<std::vector<Mapping>>();

  utils::register_set_container<std::set<QString>>();
  
  utils::register_associative_container<QMap<QString, QVariant>>();
  utils::register_associative_container<QMap<QString, QStringList>>();
  
  utils::register_associative_container<IFileTree::OverwritesType>();

  // Tuple:
  bpy::register_tuple<std::tuple<bool, DWORD>>(); // IOrganizer::waitForApplication
  bpy::register_tuple<std::tuple<IPluginInstaller::EInstallResult, std::shared_ptr<IFileTree>, QString, int>>();

  // Variants:
  bpy::register_variant<std::variant<
    IPluginInstaller::EInstallResult, 
    std::shared_ptr<IFileTree>, 
    std::tuple<IPluginInstaller::EInstallResult, std::shared_ptr<IFileTree>, QString, int>>>();
  bpy::register_variant<std::variant<IFileTree::OverwritesType, std::size_t, bool>>();
  bpy::register_variant<std::variant<QString, bool>>();

  // Functions:
  Functor_converter<void()>(); // converter for the onRefreshed-callback
  Functor_converter<void(const QString&)>();
  Functor_converter<void(const QString&, unsigned int)>();
  Functor_converter<void(const QString&, IModList::ModStates)>(); // converter for the onModStateChanged-callback
  Functor_converter<bool(const IOrganizer::FileInfo&)>();
  Functor_converter<bool(const QString&)>();
  Functor_converter<bool(std::shared_ptr<FileTreeEntry> const&)>();
  Functor_converter<std::variant<QString, bool>(QString const&)>();


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
      .def("__str__", &VersionInfo::canonicalString)
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

  bpy::class_<IOrganizer::FileInfo>("FileInfo", bpy::init<>())
    .def_readwrite("filePath", &IOrganizer::FileInfo::filePath)
    .def_readwrite("archive", &IOrganizer::FileInfo::archive)
    .def_readwrite("origins", &IOrganizer::FileInfo::origins)
    ;

  bpy::class_<IOrganizer, boost::noncopyable>("IOrganizer", bpy::no_init)
      .def("createNexusBridge", &IOrganizer::createNexusBridge, bpy::return_value_policy<bpy::reference_existing_object>())
      .def("profileName", &IOrganizer::profileName)
      .def("profilePath", &IOrganizer::profilePath)
      .def("downloadsPath", &IOrganizer::downloadsPath)
      .def("overwritePath", &IOrganizer::overwritePath)
      .def("basePath", &IOrganizer::basePath)
      .def("modsPath", &IOrganizer::modsPath)
      .def("appVersion", &IOrganizer::appVersion)
      .def("getMod", &IOrganizer::getMod, bpy::return_value_policy<bpy::reference_existing_object>())
      .def("createMod", &IOrganizer::createMod, bpy::return_value_policy<bpy::reference_existing_object>())
      .def("getGame", &IOrganizer::getGame, bpy::return_value_policy<bpy::reference_existing_object>())
      .def("removeMod", &IOrganizer::removeMod)
      .def("modDataChanged", &IOrganizer::modDataChanged)
      .def("pluginSetting", &IOrganizer::pluginSetting)
      .def("setPluginSetting", &IOrganizer::setPluginSetting)
      .def("persistent", &IOrganizer::persistent, bpy::arg("persistent") = QVariant())
      .def("setPersistent", &IOrganizer::setPersistent, bpy::arg("sync") = true)
      .def("pluginDataPath", &IOrganizer::pluginDataPath)
      .def("installMod", &IOrganizer::installMod, (bpy::arg("name_suggestion") = ""), bpy::return_value_policy<bpy::reference_existing_object>())
      .def("resolvePath", &IOrganizer::resolvePath)
      .def("listDirectories", &IOrganizer::listDirectories)
      .def("findFiles", &IOrganizer::findFiles)
      .def("getFileOrigins", &IOrganizer::getFileOrigins)
      .def("findFileInfos", &IOrganizer::findFileInfos)
      .def("downloadManager", &IOrganizer::downloadManager, bpy::return_value_policy<bpy::reference_existing_object>())
      .def("pluginList", &IOrganizer::pluginList, bpy::return_value_policy<bpy::reference_existing_object>())
      .def("modList", &IOrganizer::modList, bpy::return_value_policy<bpy::reference_existing_object>())
      .def("profile", &IOrganizer::profile, bpy::return_value_policy<bpy::reference_existing_object>())
      .def("startApplication", 
        +[](IOrganizer* o, const QString& executable, const QStringList& args, const QString& cwd, const QString& profile, 
          const QString& forcedCustomOverwrite, bool ignoreCustomOverwrite) {
            return (std::uintptr_t) o->startApplication(executable, args, cwd, profile, forcedCustomOverwrite, ignoreCustomOverwrite);
        }, 
      ((bpy::arg("args") = QStringList()), (bpy::arg("cwd") = ""), (bpy::arg("profile") = ""), (bpy::arg("forcedCustomOverwrite") = ""), (bpy::arg("ignoreCustomOverwrite") = false)), bpy::return_value_policy<bpy::return_by_value>())
      //.def("waitForApplication", bpy::pure_virtual(&IOrganizer::waitForApplication), (bpy::arg("exitCode")=nullptr), bpy::return_value_policy<bpy::return_by_value>())
      // Use wrapped version
      .def("waitForApplication", +[](IOrganizer *o, std::uintptr_t handle) {
          DWORD returnCode;
          bool result = o->waitForApplication((HANDLE)handle, &returnCode);
          return std::make_tuple(result, returnCode);
        }
        )
      .def("onModInstalled", &IOrganizer::onModInstalled)
      .def("onAboutToRun", &IOrganizer::onAboutToRun)
      .def("onFinishedRun", &IOrganizer::onFinishedRun)
      .def("refreshModList", &IOrganizer::refreshModList, (bpy::arg("save_changes")=true))
      .def("managedGame", &IOrganizer::managedGame, bpy::return_value_policy<bpy::reference_existing_object>())
      .def("modsSortedByProfilePriority", &IOrganizer::modsSortedByProfilePriority)
      ;

  // FileTreeEntry Scope:
  auto fileTreeEntryClass = bpy::class_<FileTreeEntry, boost::noncopyable>("FileTreeEntry", bpy::no_init);
  {
    
    bpy::scope scope = fileTreeEntryClass;
    
    bpy::enum_<FileTreeEntry::FileTypes>("FileTypes")
    .value("FILE_OR_DIRECTORY", FileTreeEntry::FILE_OR_DIRECTORY)
    .value("FILE", FileTreeEntry::FILE)
    .value("DIRECTORY", FileTreeEntry::DIRECTORY)
    .export_values()
    ;

    fileTreeEntryClass

      .def("isFile", &FileTreeEntry::isFile)
      .def("isDir", &FileTreeEntry::isDir)
      // Forcing the conversion to FileTypeS to avoid having to expose FileType in python:
      .def("fileType", +[](FileTreeEntry* p) { return FileTreeEntry::FileTypes{ p->fileType() }; })
      // This should probably not be exposed in python since we provide automatic downcast:
      // .def("getTree", static_cast<std::shared_ptr<IFileTree>(FileTreeEntry::*)()>(&FileTreeEntry::astree))
      .def("name", &FileTreeEntry::name)
      .def("suffix", &FileTreeEntry::suffix)
      .def("time", &FileTreeEntry::time)
      .def("parent", static_cast<std::shared_ptr<IFileTree>(FileTreeEntry::*)()>(&FileTreeEntry::parent), "[optional]")
      .def("path", &FileTreeEntry::path, bpy::arg("sep") = "\\")
      .def("pathFrom", &FileTreeEntry::pathFrom, bpy::arg("sep") = "\\")

      // Mutable operation:
      .def("setTime", &FileTreeEntry::setTime)
      .def("detach", &FileTreeEntry::detach)
      .def("moveTo", &FileTreeEntry::moveTo)

      // Special methods:
      .def("__eq__", +[](const FileTreeEntry* entry, QString other) {
        return entry->compare(other) == 0;
      })
      .def("__eq__", +[](const FileTreeEntry* entry, std::shared_ptr<FileTreeEntry> other) {
        return entry == other.get();
      })

      // Special methods for debug:
      .def("__repr__", +[](const FileTreeEntry* entry) { return "FileTreeEntry(\"" + entry->name() + "\")"; })
      ;
  }

  // IFileTree scope:
  auto iFileTreeClass = bpy::class_<IFileTree, bpy::bases<FileTreeEntry>, boost::noncopyable>("IFileTree", bpy::no_init);
  {
    
    bpy::scope scope = iFileTreeClass;

    bpy::enum_<IFileTree::InsertPolicy>("InsertPolicy")
      .value("FAIL_IF_EXISTS", IFileTree::InsertPolicy::FAIL_IF_EXISTS)
      .value("REPLACE", IFileTree::InsertPolicy::REPLACE)
      .value("MERGE", IFileTree::InsertPolicy::MERGE)
      .export_values()
      ;

    iFileTreeClass

      // Non-mutable operations (note: iterator and some methods are at the end with 
      // special python methods):
      .def("exists", static_cast<bool(IFileTree::*)(QString, IFileTree::FileTypes) const>(&IFileTree::exists), (bpy::arg("type") = IFileTree::FILE_OR_DIRECTORY))
      .def("find", static_cast<std::shared_ptr<FileTreeEntry>(IFileTree::*)(QString, IFileTree::FileTypes)>(&IFileTree::find), 
        bpy::arg("type") = IFileTree::FILE_OR_DIRECTORY, bpy::return_value_policy<DowncastReturn<FileTreeEntry, IFileTree>>(), "[optional]")
      .def("pathTo", &IFileTree::pathTo, bpy::arg("sep") = "\\")

      // Kind-of-static operations:
      .def("createOrphanTree", &IFileTree::createOrphanTree, bpy::arg("name") = "")

      // Mutable operations:
      .def("addFile", &IFileTree::addFile, bpy::arg("time") = QDateTime(), "[optional]")
      .def("addDirectory", &IFileTree::addDirectory, "[optional]")
      .def("insert", +[](
        IFileTree* p, std::shared_ptr<FileTreeEntry> entry, IFileTree::InsertPolicy insertPolicy) {
          return p->insert(entry, insertPolicy) != p->end(); }, bpy::arg("policy") = IFileTree::InsertPolicy::FAIL_IF_EXISTS)

      .def("merge", +[](IFileTree* p, std::shared_ptr<IFileTree> other, bool returnOverwrites) -> std::variant<IFileTree::OverwritesType, std::size_t, bool> {
            IFileTree::OverwritesType overwrites;
            auto result = p->merge(other, returnOverwrites ? &overwrites : nullptr);
            if (result == IFileTree::MERGE_FAILED) {
              return { false };
            }
            if (returnOverwrites) {
              return { overwrites };
            }
            return { result };
        }, bpy::arg("overwrites") = false)

      .def("move", &IFileTree::move, bpy::arg("policy") = IFileTree::InsertPolicy::FAIL_IF_EXISTS)

      .def("remove", +[](IFileTree* p, QString name) { return p->erase(name).first != p->end(); })
      .def("remove", +[](IFileTree* p, std::shared_ptr<FileTreeEntry> entry) { return p->erase(entry) != p->end(); })

      .def("clear", &IFileTree::clear)
      .def("removeAll", &IFileTree::removeAll)
      .def("removeIf", &IFileTree::removeIf)

      // Special methods:
      .def("__getitem__", static_cast<std::shared_ptr<FileTreeEntry>(IFileTree::*)(std::size_t)>(&IFileTree::at),
        bpy::return_value_policy<DowncastReturn<FileTreeEntry, IFileTree>>())
      .def("__iter__", bpy::range<bpy::return_value_policy<DowncastReturn<FileTreeEntry, IFileTree>>>(
        static_cast<IFileTree::iterator(IFileTree::*)()>(&IFileTree::begin),
        static_cast<IFileTree::iterator(IFileTree::*)()>(&IFileTree::end)))
      .def("__len__", &IFileTree::size)
      .def("__bool__", +[](const IFileTree* tree) { return !tree->empty(); })
      .def("__repr__", +[](const IFileTree* entry) { return "IFileTree(\"" + entry->name() + "\")"; })
      ;
  }
  

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
      .def("__str__", &ModRepositoryFileInfo::toString)
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

  utils::register_sequence_container<std::vector<std::shared_ptr<const MOBase::FileTreeEntry>>>();
  bpy::class_<IInstallationManager, boost::noncopyable>("IInstallationManager", bpy::no_init)
    .def("extractFile", &IInstallationManager::extractFile)
    .def("extractFiles", &IInstallationManager::extractFiles)
    .def("installArchive", &IInstallationManager::installArchive)
    .def("setURL", &IInstallationManager::setURL)
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
      .def("setGamePlugin", bpy::pure_virtual(&IModInterface::setGamePlugin))
      .def("setName", bpy::pure_virtual(&IModInterface::setName))
      .def("remove", bpy::pure_virtual(&IModInterface::remove))
      ;

  bpy::enum_<MOBase::EGuessQuality>("GuessQuality")
      .value("INVALID", MOBase::GUESS_INVALID)
      .value("FALLBACK", MOBase::GUESS_FALLBACK)
      .value("GOOD", MOBase::GUESS_GOOD)
      .value("META", MOBase::GUESS_META)
      .value("PRESET", MOBase::GUESS_PRESET)
      .value("USER", MOBase::GUESS_USER)
      ;

  bpy::class_<MOBase::GuessedValue<QString>, boost::noncopyable>("GuessedString")
      .def(bpy::init<>())
      .def(bpy::init<QString const&, EGuessQuality>())
      .def("update",
           static_cast<GuessedValue<QString>& (GuessedValue<QString>::*)(const QString&)>(&GuessedValue<QString>::update),
           bpy::return_self<>())
      .def("update",
           static_cast<GuessedValue<QString>& (GuessedValue<QString>::*)(const QString&, EGuessQuality)>(&GuessedValue<QString>::update),
           bpy::return_self<>())

      // Methods to simulate the assignment operator:
      .def("reset", +[](GuessedValue<QString>* gv) { *gv = GuessedValue<QString>(); }, bpy::return_self<>())
      .def("reset", +[](GuessedValue<QString>* gv, const QString& value, EGuessQuality eq) { *gv = GuessedValue<QString>(value, eq); }, bpy::return_self<>())
      .def("reset", +[](GuessedValue<QString>* gv, const GuessedValue<QString>& other) { *gv = other; }, bpy::return_self<>())

      // Use an intermediate lambda to avoid having to register the std::function conversion:
      .def("setFilter", +[](GuessedValue<QString>* gv, std::function<std::variant<QString, bool>(QString const&)> fn) {
        gv->setFilter([fn](QString& s) {
          auto ret = fn(s);
          return std::visit([&s](auto v) {
            if constexpr (std::is_same_v<decltype(v), QString>) {
              s = v;
              return true;
            }
            else if constexpr (std::is_same_v<decltype(v), bool>) {
              return v;
            }
          }, ret);
        });
      })

      // This makes a copy in python but it more practical than exposing an iterator:
      .def("variants", &GuessedValue<QString>::variants, bpy::return_value_policy<bpy::copy_const_reference>())
      .def("__str__", &MOBase::GuessedValue<QString>::operator const QString&, bpy::return_value_policy<bpy::copy_const_reference>())
      ;

  bpy::to_python_converter<IPluginList::PluginStates, QFlags_to_int<IPluginList::PluginState>>();
  QFlags_from_python_obj<IPluginList::PluginState>();

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

  bpy::class_<IPluginWrapper, boost::noncopyable>("IPlugin")
    .def("init", bpy::pure_virtual(&MOBase::IPlugin::init))
    .def("name", bpy::pure_virtual(&MOBase::IPlugin::name))
    .def("author", bpy::pure_virtual(&MOBase::IPlugin::author))
    .def("description", bpy::pure_virtual(&MOBase::IPlugin::description))
    .def("version", bpy::pure_virtual(&MOBase::IPlugin::version))
    .def("isActive", bpy::pure_virtual(&MOBase::IPlugin::isActive))
    .def("settings", bpy::pure_virtual(&MOBase::IPlugin::settings))
    ;

  bpy::class_<IPluginDiagnoseWrapper, bpy::bases<IPlugin>, boost::noncopyable>("IPluginDiagnose")
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

  bpy::class_<IPluginFileMapperWrapper, bpy::bases<IPlugin>, boost::noncopyable>("IPluginFileMapper")
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

  bpy::class_<IPluginGameWrapper, bpy::bases<IPlugin>, boost::noncopyable>("IPluginGame")
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

      .def("featureList", +[](MOBase::IPluginGame* p) {
        // Constructing a dict from class name to actual object:
        bpy::dict dict;
        mp11::mp_for_each<
          mp11::mp_transform<
            // Must user pointers because mp_for_each construct object:
            std::add_pointer_t,
            mp11::mp_list<
              BSAInvalidation,
              DataArchives, 
              GamePlugins, 
              LocalSavegames, 
              SaveGameInfo, 
              ScriptExtender, 
              UnmanagedMods
            >
          >
        >([&](auto* pt) {
          using T = std::remove_pointer_t<decltype(pt)>;
          typename bpy::reference_existing_object::apply<T*>::type converter;

          // Retrieve the python class object:
          const bpy::converter::registration* registration = bpy::converter::registry::query(bpy::type_id<T>());
          bpy::object key = bpy::object(bpy::handle<>(bpy::borrowed(registration->get_class_object())));

          // Set the object:
          dict[key] = bpy::handle<>(converter(p->feature<T>()));
        });
        return dict;
      })
      ;

  bpy::enum_<MOBase::IPluginInstaller::EInstallResult>("InstallResult")
      .value("SUCCESS", MOBase::IPluginInstaller::RESULT_SUCCESS)
      .value("FAILED", MOBase::IPluginInstaller::RESULT_FAILED)
      .value("CANCELED", MOBase::IPluginInstaller::RESULT_CANCELED)
      .value("MANUAL_REQUESTED", MOBase::IPluginInstaller::RESULT_MANUALREQUESTED)
      .value("NOT_ATTEMPTED", MOBase::IPluginInstaller::RESULT_NOTATTEMPTED)
      ;

  bpy::class_<IPluginInstaller, bpy::bases<IPlugin>, boost::noncopyable>("IPluginInstaller", bpy::no_init)
    .def("isArchiveSupported", &IPluginInstaller::isArchiveSupported)
    .def("priority", &IPluginInstaller::priority)
    .def("isManualInstaller", &IPluginInstaller::isManualInstaller)
    .def("setParentWidget", &IPluginInstaller::setParentWidget)
    .def("setInstallationManager", &IPluginInstaller::setInstallationManager)
    ;

  bpy::class_<IPluginInstallerSimpleWrapper, bpy::bases<IPluginInstaller>, boost::noncopyable>("IPluginInstallerSimple")
    // Note: Keeping the variant here even if we always return a tuple to be consistent with the wrapper and
    // have proper stubs generation.
    .def("install", +[](IPluginInstallerSimple* p, GuessedValue<QString>& modName, std::shared_ptr<IFileTree>& tree, QString& version, int& nexusID)
      -> std::variant<IPluginInstaller::EInstallResult, std::shared_ptr<IFileTree>, std::tuple<IPluginInstaller::EInstallResult, std::shared_ptr<IFileTree>, QString, int>> {
        auto result = p->install(modName, tree, version, nexusID);
        return std::make_tuple(result, tree, version, nexusID);
      })
    .def("_parentWidget", &IPluginInstallerSimpleWrapper::parentWidget, bpy::return_value_policy<bpy::return_by_value>())
    .def("_manager", &IPluginInstallerSimpleWrapper::manager, bpy::return_value_policy<bpy::reference_existing_object>())
    ;

  bpy::class_<IPluginInstallerCustomWrapper, boost::noncopyable>("IPluginInstallerCustom")
    // Needs to add both otherwize boost does not understanda:    
    .def("isArchiveSupported", &IPluginInstaller::isArchiveSupported)
    .def("isArchiveSupported", &IPluginInstallerCustom::isArchiveSupported)
    .def("supportedExtensions", &IPluginInstallerCustom::supportedExtensions)
    .def("install", &IPluginInstallerCustom::install)
    .def("_parentWidget", &IPluginInstallerSimpleWrapper::parentWidget, bpy::return_value_policy<bpy::return_by_value>())
    .def("_manager", &IPluginInstallerCustomWrapper::manager, bpy::return_value_policy<bpy::reference_existing_object>())
    ;

  bpy::class_<IPluginModPageWrapper, boost::noncopyable>("IPluginModPage")
    .def("displayName", bpy::pure_virtual(&IPluginModPage::displayName))
    .def("icon", bpy::pure_virtual(&IPluginModPage::icon))
    .def("pageURL", bpy::pure_virtual(&IPluginModPage::pageURL))
    .def("useIntegratedBrowser", bpy::pure_virtual(&IPluginModPage::useIntegratedBrowser))
    .def("handlesDownload", bpy::pure_virtual(&IPluginModPage::handlesDownload))
    .def("setParentWidget", &IPluginModPage::setParentWidget, &IPluginModPageWrapper::setParentWidget_Default)
    .def("_parentWidget", &IPluginModPageWrapper::parentWidget, bpy::return_value_policy<bpy::return_by_value>())
    ;

  bpy::class_<IPluginPreviewWrapper, bpy::bases<IPlugin>, boost::noncopyable>("IPluginPreview")
    .def("supportedExtensions", bpy::pure_virtual(&IPluginPreview::supportedExtensions))
    .def("genFilePreview", bpy::pure_virtual(&IPluginPreview::genFilePreview), bpy::return_value_policy<bpy::return_by_value>())
    ;

  bpy::class_<IPluginToolWrapper, bpy::bases<IPlugin>, boost::noncopyable>("IPluginTool")
    .def("displayName", bpy::pure_virtual(&IPluginTool::displayName))
    .def("tooltip", bpy::pure_virtual(&IPluginTool::tooltip))
    .def("icon", bpy::pure_virtual(&IPluginTool::icon))
    .def("setParentWidget", &IPluginTool::setParentWidget, &IPluginToolWrapper::setParentWidget_Default)
    .def("_parentWidget", &IPluginToolWrapper::parentWidget, bpy::return_value_policy<bpy::return_by_value>())
    ;

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


void PythonRunner::initPath()
{
  static QStringList paths = {
    QCoreApplication::applicationDirPath() + "/pythoncore.zip",
    QCoreApplication::applicationDirPath() + "/pythoncore",
    m_MOInfo->pluginDataPath()
  };

  Py_SetPath(paths.join(';').toStdWString().c_str());
}




template <class T>
void PythonRunner::appendIfInstance(bpy::object const& obj, QList<QObject*> &interfaces) {
  bpy::extract<T*> extr{ obj };
  if (extr.check()) {
    interfaces.append(extr);
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

    std::string temp = ToString(pluginName);
    if (handled_exec_file(temp.c_str(), moduleNamespace)) {
      throw pyexcept::PythonError();
    }
    m_PythonObjects[pluginName] = moduleNamespace["createPlugin"]();

    bpy::object pluginObj = m_PythonObjects[pluginName];
    QList<QObject *> interfaceList;

    appendIfInstance<IPluginGame>(pluginObj, interfaceList);
    // Must try the wrapper because it's only a plugin extension interface in C++, so doesn't extend QObject
    appendIfInstance<IPluginDiagnoseWrapper>(pluginObj, interfaceList);
    // Must try the wrapper because it's only a plugin extension interface in C++, so doesn't extend QObject
    appendIfInstance<IPluginFileMapperWrapper>(pluginObj, interfaceList); 
    appendIfInstance<IPluginInstallerCustom>(pluginObj, interfaceList);
    appendIfInstance<IPluginInstallerSimple>(pluginObj, interfaceList);
    appendIfInstance<IPluginModPage>(pluginObj, interfaceList);
    appendIfInstance<IPluginPreview>(pluginObj, interfaceList);
    appendIfInstance<IPluginTool>(pluginObj, interfaceList);

    if (interfaceList.isEmpty())
      appendIfInstance<IPluginWrapper>(pluginObj, interfaceList);

    return interfaceList;
  } catch (const bpy::error_already_set&) {
    qWarning("failed to run python script \"%s\"", qUtf8Printable(pluginName));
    throw pyexcept::PythonError();
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

