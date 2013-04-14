#include "proxypython.h"
#include <utility.h>
#include <versioninfo.h>
#include <QtPlugin>
#include <QDirIterator>
#include <QCoreApplication>

#pragma warning( push )
#pragma warning( disable : 4100 )

#include "pythontoolwrapper.h"
#include <boost/python.hpp>

#pragma warning( pop )


using namespace MOBase;
using namespace boost::python;

/*
long int unwrap(QObject* ptr) {
    return reinterpret_cast<long int>(ptr);
}

template <typename T>
T* wrap(long int ptr) {
    return reinterpret_cast<T*>(ptr);
}
*/

struct QString_to_python_str
{
  static PyObject* convert(QString const& s) {
    return boost::python::incref(
      boost::python::object(
        s.toUtf8().constData()).ptr());
  }
};

struct QString_from_python_str
{
  QString_from_python_str() {
    boost::python::converter::registry::push_back(
      &convertible,
      &construct,
      boost::python::type_id<QString>());
  }

  static void* convertible(PyObject* obj_ptr) {
    if (!PyString_Check(obj_ptr)) return 0;
    return obj_ptr;
  }

  static void construct(PyObject* obj_ptr, boost::python::converter::rvalue_from_python_stage1_data* data) {
    // Extract the character data from the python string
    const char* value = PyString_AsString(obj_ptr);

    // Verify that obj_ptr is a string (should be ensured by convertible())
    assert(value);

    // Grab pointer to memory into which to construct the new QString
    void* storage = (
      (boost::python::converter::rvalue_from_python_storage<QString>*)
      data)->storage.bytes;

    // in-place construct the new QString using the character data
    // extraced from the python object
    new (storage) QString(value);

    // Stash the memory chunk pointer for later use by boost.python
    data->convertible = storage;
  }
};



struct QVariant_to_python_str
{
  static PyObject* convert(QVariant const &var) {
    switch (var.type()) {
      case QVariant::Int: return PyLong_FromLong(var.toInt());
      case QVariant::Bool: return PyBool_FromLong(var.toBool());
      case QVariant::String: return boost::python::incref(boost::python::object(var.toString().toUtf8().constData()).ptr());
      default: {
        qCritical("variant type not supportd in python: %d", var.type());
        throw boost::python::error_already_set();
      } break;
    }
  }
};


struct QVariant_from_python_str
{
  QVariant_from_python_str() {
    boost::python::converter::registry::push_back(
      &convertible,
      &construct,
      boost::python::type_id<QVariant>());
  }

  static void* convertible(PyObject* objPtr) {
    if (!PyString_Check(objPtr) && !PyInt_Check(objPtr) && !PyBool_Check(objPtr)) return 0;
    return objPtr;
  }

  template <typename T>
  static void constructVariant(const T &value, boost::python::converter::rvalue_from_python_stage1_data *data) {
    void* storage = ((boost::python::converter::rvalue_from_python_storage<QVariant>*)data)->storage.bytes;

    new (storage) QVariant(value);

    data->convertible = storage;
  }


  static void construct(PyObject *objPtr, boost::python::converter::rvalue_from_python_stage1_data* data) {
    // PyBools will also return true for PyInt_Check but not the other way around, so the order
    // here is relevant
    if (PyString_Check(objPtr)) {
      const char *value = PyString_AsString(objPtr);
      constructVariant(value, data);
    } else if (PyBool_Check(objPtr)) {
      bool value = (objPtr == Py_True);
      constructVariant(value, data);
    } else if (PyInt_Check(objPtr)) {
      long value = PyInt_AsLong(objPtr);
      constructVariant(value, data);
    } else {
      // cause a type-error message to be printed. I guess this isn't a very clean
      // way of doing that
      PyString_AsString(objPtr);
      throw boost::python::error_already_set();
    }
  }
};





template <class C>
struct QList_ptr_to_python_list
{
  typedef typename C::value_type T;
  typedef typename C::const_iterator iter;
  static PyObject* convert(C const &list)
  {
    // the python list
    boost::python::list pyList;
    // we need to wrap the pointers into PyObjects
    typename boost::python::reference_existing_object::apply<T*>::type converter;
    for (iter i = list.begin(); i != list.end(); ++i) {
      PyObject *obj = converter(*i);
      object real_obj = object(handle<>(obj));
      // append the PyObject
      pyList.append(real_obj);
    }
    return incref(pyList.ptr());
  }
};
template <class C>
struct QList_ptr_from_python_list
{
  typedef typename C::value_type T;
  QList_ptr_from_python_list()
  {
    converter::registry::push_back( &convertible, &construct, type_id<C>() );
  }
  static void* convertible(PyObject *obj_ptr)
  {
    //is this a tuple type?
    if (PyTuple_Check(obj_ptr)) {
      //check the tuple elements... - convert to a boost::tuple object
      boost::python::tuple t( handle<>(borrowed(obj_ptr)) );
      //how many elements are there?
      int n = PyTuple_Size(obj_ptr);
      //can they all be converted to type 'T'?
      for (int i=0; i<n; ++i) {
        if (!boost::python::extract<T>(t[i]).check())
          return 0;
      }
      //the tuple is ok!
      return obj_ptr;
    }
    //is this a list type?
    else if (PyList_Check(obj_ptr)) {
      //check that all of the list elements can be converted to the right type
      boost::python::list l( handle<>(borrowed(obj_ptr)) );
      //how many elements are there?
      int n = PyList_Size(obj_ptr);
      //can all of the elements be converted to type 'T'?
      for (int i=0; i<n; ++i) {
        if (!boost::python::extract<T>(l[i]).check())
          return 0;
      }
      //the list is ok!
      return obj_ptr;
    }
    //could not recognise the type...
    return 0;
  }
  static void construct(PyObject *obj_ptr, converter::rvalue_from_python_stage1_data *data)
  {
    if (PyTuple_Check(obj_ptr)) {
      //convert the PyObject to a boost::python::object
      boost::python::tuple t( handle<>(borrowed(obj_ptr)) );
      //locate the storage space for the result
      void* storage = ((converter::rvalue_from_python_storage<C>*)data)->storage.bytes;
      //create the T container
      new (storage) C();
      C *container = static_cast<C*>(storage);
      //add all of the elements from the tuple - get the number of elements in the tuple
      int n = PyTuple_Size(obj_ptr);
      for (int i=0; i<n; ++i)
        container->append( extract<T>(t[i])() );
      data->convertible = storage;
    }
    else if (PyList_Check(obj_ptr)) {
      //convert the PyObject to a boost::python::object
      boost::python::list l( handle<>(borrowed(obj_ptr)) );
      //locate the storage space for the result
      void* storage = ((converter::rvalue_from_python_storage<C>*)data)->storage.bytes;
      //create the T container
      new (storage) C();
      C *container = static_cast<C*>(storage);
      //add all of the elements from the tuple - get the number of elements in the tuple
      int n = PyList_Size(obj_ptr);
      for (int i=0; i<n; ++i)
        container->append( extract<T>(l[i])() );
      data->convertible = storage;
    }
  }
};

struct IOrganizerWrapper: MOBase::IOrganizer, wrapper<MOBase::IOrganizer>
{
  virtual MOBase::IGameInfo &gameInfo() const {
    MOBase::IGameInfo *result = this->get_override("gameInfo")();
    return *result;
  }
  virtual QString profileName() const { return this->get_override("profileName")(); }
  virtual QString profilePath() const { return this->get_override("profilePath")(); }
  virtual QString downloadsPath() const { return this->get_override("downloadsPath")(); }
  virtual MOBase::VersionInfo appVersion() const { return this->get_override("appVersion")(); }
  virtual MOBase::IModInterface *getMod(const QString &name) { return this->get_override("getMod")(name); }
  virtual MOBase::IModInterface *createMod(const QString &name) { return this->get_override("createMod")(name); }
  virtual bool removeMod(MOBase::IModInterface *mod) { return this->get_override("removeMod")(mod); }
  virtual void modDataChanged(MOBase::IModInterface *mod) { this->get_override("modDataChanged")(mod); }
  virtual QVariant pluginSetting(const QString &pluginName, const QString &key) const { qDebug("xyz"); return this->get_override("pluginSetting")(pluginName, key); }
  virtual QString pluginDataPath() const { return this->get_override("pluginDataPath")(); }
};


struct IGameInfoWrapper: MOBase::IGameInfo, wrapper<MOBase::IGameInfo>
{
  virtual Type type() const { return this->get_override("type")(); }
  virtual QString path() const { return this->get_override("path")(); }
  virtual QString binaryName() const { return this->get_override("binaryName")(); }
};



enum PluginType {
  PLUGIN_TOOL,
  PLUGIN_INSTALLER_SIMPLE,
  PLUGIN_INSTALLER_COMPLEX
};


BOOST_PYTHON_MODULE(mobase)
{
  enum_<MOBase::VersionInfo::ReleaseType>("ReleaseType")
         .value("final", MOBase::VersionInfo::RELEASE_FINAL)
         .value("candidate", MOBase::VersionInfo::RELEASE_CANDIDATE)
         .value("beta", MOBase::VersionInfo::RELEASE_BETA)
         .value("alpha", MOBase::VersionInfo::RELEASE_ALPHA)
         .value("prealpha", MOBase::VersionInfo::RELEASE_PREALPHA);

  enum_<PluginType>("PluginType")
      .value("tool", PLUGIN_TOOL)
      .value("installer_simple", PLUGIN_INSTALLER_SIMPLE)
      .value("installer_complex", PLUGIN_INSTALLER_COMPLEX);

  enum_<MOBase::IGameInfo::Type>("GameType")
      .value("oblivion", MOBase::IGameInfo::TYPE_OBLIVION)
      .value("fallout3", MOBase::IGameInfo::TYPE_FALLOUT3)
      .value("falloutnv", MOBase::IGameInfo::TYPE_FALLOUTNV)
      .value("skyrim", MOBase::IGameInfo::TYPE_SKYRIM);

  class_<MOBase::VersionInfo>("VersionInfo")
      .def(init<int, int, int, MOBase::VersionInfo::ReleaseType>())
      .def("parse", &MOBase::VersionInfo::parse)
      .def("canonicalString", &MOBase::VersionInfo::canonicalString);

  class_<MOBase::PluginSetting>("PluginSetting", init<const QString&, const QString&, const QVariant&>());

  class_<IGameInfoWrapper, boost::noncopyable>("GameInfo")
      .def("type", pure_virtual(&MOBase::IGameInfo::type))
      .def("path", pure_virtual(&MOBase::IGameInfo::path))
      .def("binaryName", pure_virtual(&MOBase::IGameInfo::binaryName));

  class_<IOrganizerWrapper, boost::noncopyable>("IOrganizer")
      .def("gameInfo", pure_virtual(&MOBase::IOrganizer::gameInfo), return_value_policy<reference_existing_object>())
      .def("profileName", pure_virtual(&MOBase::IOrganizer::profileName))
      .def("profilePath", pure_virtual(&MOBase::IOrganizer::profilePath))
      .def("downloadsPath", pure_virtual(&MOBase::IOrganizer::downloadsPath))
      .def("appVersion", pure_virtual(&MOBase::IOrganizer::appVersion))
      .def("getMod", pure_virtual(&MOBase::IOrganizer::getMod), return_value_policy<reference_existing_object>())
      .def("createMod", pure_virtual(&MOBase::IOrganizer::createMod), return_value_policy<reference_existing_object>())
      .def("removeMod", pure_virtual(&MOBase::IOrganizer::removeMod))
      .def("modDataChanged", pure_virtual(&MOBase::IOrganizer::modDataChanged))
      .def("pluginSetting", pure_virtual(&MOBase::IOrganizer::pluginSetting))
      .def("pluginDataPath", pure_virtual(&MOBase::IOrganizer::pluginDataPath));

  boost::python::to_python_converter<
    QString,
    QString_to_python_str>();

  QString_from_python_str();

  boost::python::to_python_converter<
    QVariant,
    QVariant_to_python_str>();

  QVariant_from_python_str();

/*
  boost::python::to_python_converter<
      QList<PluginSetting>,
      QList_ptr_to_python_list<QList<PluginSetting> > >();

  QList_ptr_from_python_list<QList<PluginSetting> >();*/
}


static char* argv0 = "Banana?";

ProxyPython::ProxyPython()
  : m_MOInfo(NULL)
{
  PyImport_AppendInittab("mobase", &initmobase);
  Py_Initialize();

  PySys_SetArgv(0, &argv0);
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


bool handled_exec_file(str filename, object globals = object(), object locals = object())
{
  return handle_exception(boost::bind(exec_file, filename, globals, locals));
}


QObject *ProxyPython::instantiate(const QString &pluginName) const
{
  try {
    object main_module = import("__main__");
    object main_namespace = main_module.attr("__dict__");

    object mobase_module((handle<>(PyImport_ImportModule("mobase"))));
    main_namespace["mobase"] = mobase_module;

    std::string temp = ToString(pluginName);
    if (handled_exec_file(temp.c_str(), main_namespace)) {
      if (PyErr_Occurred()) {
        PyErr_Print();
      } else {
        qCritical("An unexpected C++ exception was thrown in python code");
      }
      return NULL;
    }

    PluginType type = boost::python::extract<PluginType>(main_namespace["type"]());

    switch (type) {
      case PLUGIN_TOOL:  return new PythonToolWrapper(
                                                main_namespace["init"],
                                                main_namespace["name"],
                                                main_namespace["author"],
                                                main_namespace["description"],
                                                main_namespace["version"],
                                                main_namespace["isActive"],
                                                main_namespace["settings"],
                                                main_namespace["displayName"],
                                                main_namespace["tooltip"],
                                                main_namespace["icon"],
                                                main_namespace["display"]
                                              );
      case PLUGIN_INSTALLER_SIMPLE: return NULL;
      case PLUGIN_INSTALLER_COMPLEX: return NULL;
    }

  } catch (const std::exception &e) {
    qDebug("failed to run python script \"%s\": %s", qPrintable(pluginName), e.what());
  } catch (const error_already_set&) {
    qDebug("failed to run python script \"%s\"", qPrintable(pluginName));
    PyErr_Print();
  }
  return NULL;
}

Q_EXPORT_PLUGIN2(proxyPython, ProxyPython)
