#include "gamefeatureswrappers.h"

#include <boost/any.hpp>

#include <typeindex>

#include <ipluginlist.h>
#include <iprofile.h>
#include <isavegame.h>
#include <isavegameinfowidget.h>

#include "gilock.h"
#include "pythonwrapperutilities.h"

/////////////////////////////
/// BSAInvalidation Wrapper


bool BSAInvalidationWrapper::isInvalidationBSA(const QString &bsaName)
{
  return basicWrapperFunctionImplementation<BSAInvalidationWrapper, bool>(this, "isInvalidationBSA", bsaName);
}

void BSAInvalidationWrapper::deactivate(MOBase::IProfile *profile)
{
  return basicWrapperFunctionImplementation<BSAInvalidationWrapper, void>(this, "deactivate", boost::python::ptr(profile));
}

void BSAInvalidationWrapper::activate(MOBase::IProfile *profile)
{
  return basicWrapperFunctionImplementation<BSAInvalidationWrapper, void>(this, "activate", boost::python::ptr(profile));
}

bool BSAInvalidationWrapper::prepareProfile(MOBase::IProfile *profile)
{
  return basicWrapperFunctionImplementation<BSAInvalidationWrapper, bool>(this, "prepareProfile", boost::python::ptr(profile));
}
/// end BSAInvalidation Wrapper
/////////////////////////////
/// DataArchives Wrapper


QStringList DataArchivesWrapper::vanillaArchives() const
{
  return basicWrapperFunctionImplementation<DataArchivesWrapper, QStringList>(this, "vanillaArchives");
}

QStringList DataArchivesWrapper::archives(const MOBase::IProfile *profile) const
{
  return basicWrapperFunctionImplementation<DataArchivesWrapper, QStringList>(this, "archives", boost::python::ptr(profile));
}

void DataArchivesWrapper::addArchive(MOBase::IProfile *profile, int index, const QString &archiveName)
{
  return basicWrapperFunctionImplementation<DataArchivesWrapper, void>(this, "addArchive", boost::python::ptr(profile), index, archiveName);
}

void DataArchivesWrapper::removeArchive(MOBase::IProfile *profile, const QString &archiveName)
{
  return basicWrapperFunctionImplementation<DataArchivesWrapper, void>(this, "removeArchive", boost::python::ptr(profile), archiveName);
}
/// end DataArchives Wrapper
/////////////////////////////
/// GamePlugins Wrapper


void GamePluginsWrapper::writePluginLists(const MOBase::IPluginList * pluginList)
{
  return basicWrapperFunctionImplementation<GamePluginsWrapper, void>(this, "writePluginLists", boost::python::ptr(pluginList));
}

void GamePluginsWrapper::readPluginLists(MOBase::IPluginList * pluginList)
{
  return basicWrapperFunctionImplementation<GamePluginsWrapper, void>(this, "readPluginLists", boost::python::ptr(pluginList));
}

void GamePluginsWrapper::getLoadOrder(QStringList &loadOrder)
{
  return basicWrapperFunctionImplementation<GamePluginsWrapper, void>(this, "getLoadOrder", loadOrder);
}

bool GamePluginsWrapper::lightPluginsAreSupported()
{
    return basicWrapperFunctionImplementation<GamePluginsWrapper, bool>(this, "lightPluginsAreSupported");
}

/// end GamePlugins Wrapper
/////////////////////////////
/// LocalSavegames Wrapper


MappingType LocalSavegamesWrapper::mappings(const QDir & profileSaveDir) const
{
  return basicWrapperFunctionImplementation<LocalSavegamesWrapper, MappingType>(this, "mappings", profileSaveDir);
}

bool LocalSavegamesWrapper::prepareProfile(MOBase::IProfile * profile)
{
  return basicWrapperFunctionImplementation<LocalSavegamesWrapper, bool>(this, "prepareProfile", boost::python::ptr(profile));
}

/// end LocalSavegames Wrapper
/////////////////////////////
/// SaveGameInfo Wrapper


MOBase::ISaveGame const * SaveGameInfoWrapper::getSaveGameInfo(QString const & file) const
{
  return basicWrapperFunctionImplementation<SaveGameInfoWrapper, MOBase::ISaveGame const *>(this, "getSaveGameInfo", file);
}

SaveGameInfoWrapper::MissingAssets SaveGameInfoWrapper::getMissingAssets(QString const & file) const
{
  return basicWrapperFunctionImplementation<SaveGameInfoWrapper, SaveGameInfoWrapper::MissingAssets>(this, "getMissingAssets", file);
}

MOBase::ISaveGameInfoWidget* SaveGameInfoWrapper::getSaveGameWidget(QWidget* parent) const
{
  // This will require a lot of works as ISaveGameInfoWidget inherits QWidget and I currently found no
  // way of exposing a class that inherits QWidget without having to expose manually the whole QWidget, 
  // and even with this, I am not sure how this would fit with PyQt/sip.
  qCritical("Calling method with unimplemented from_python converter.");
  return nullptr;
}

bool SaveGameInfoWrapper::hasScriptExtenderSave(QString const & file) const
{
  return basicWrapperFunctionImplementation<SaveGameInfoWrapper, bool>(this, "hasScriptExtenderSave", file);
}
/// end SaveGameInfo Wrapper
/////////////////////////////
/// ScriptExtender Wrapper

QString ScriptExtenderWrapper::BinaryName() const
{
  return basicWrapperFunctionImplementation<ScriptExtenderWrapper, QString>(this, "BinaryName");
}

QString ScriptExtenderWrapper::PluginPath() const
{
  return basicWrapperFunctionImplementation<ScriptExtenderWrapper, QString>(this, "PluginPath");
}

QString ScriptExtenderWrapper::loaderName() const
{
  return basicWrapperFunctionImplementation<ScriptExtenderWrapper, QString>(this, "loaderName");
}

QString ScriptExtenderWrapper::loaderPath() const
{
  return basicWrapperFunctionImplementation<ScriptExtenderWrapper, QString>(this, "loaderPath");
}

QStringList ScriptExtenderWrapper::saveGameAttachmentExtensions() const
{
  return basicWrapperFunctionImplementation<ScriptExtenderWrapper, QStringList>(this, "saveGameAttachmentExtensions");
}

bool ScriptExtenderWrapper::isInstalled() const
{
  return basicWrapperFunctionImplementation<ScriptExtenderWrapper, bool>(this, "isInstalled");
}

QString ScriptExtenderWrapper::getExtenderVersion() const
{
  return basicWrapperFunctionImplementation<ScriptExtenderWrapper, QString>(this, "getExtenderVersion");
}

WORD ScriptExtenderWrapper::getArch() const
{
  return basicWrapperFunctionImplementation<ScriptExtenderWrapper, WORD>(this, "getArch");
}

/// end ScriptExtender Wrapper
/////////////////////////////
/// UnmanagedMods Wrapper


QStringList UnmanagedModsWrapper::mods(bool onlyOfficial) const
{
  return basicWrapperFunctionImplementation<UnmanagedModsWrapper, QStringList>(this, "mods", onlyOfficial);
}

QString UnmanagedModsWrapper::displayName(const QString & modName) const
{
  return basicWrapperFunctionImplementation<UnmanagedModsWrapper, QString>(this, "displayName", modName);
}

QFileInfo UnmanagedModsWrapper::referenceFile(const QString & modName) const
{
  return basicWrapperFunctionImplementation<UnmanagedModsWrapper, QFileInfo>(this, "referenceFile", modName);
}

QStringList UnmanagedModsWrapper::secondaryFiles(const QString & modName) const
{
  return basicWrapperFunctionImplementation<UnmanagedModsWrapper, QStringList>(this, "secondaryFiles", modName);
}
/// end UnmanagedMods Wrapper
/////////////////////////////

template<typename T>
void insertGameFeature(std::map<std::type_index, boost::any> &map, const boost::python::object &pyObject)
{
  map[std::type_index(typeid(T))] = boost::python::extract<T*>(pyObject)();
}

game_features_map_from_python::game_features_map_from_python()
{
  boost::python::converter::registry::push_back(&convertible, &construct, boost::python::type_id<std::map<std::type_index, boost::any>>());
}

void * game_features_map_from_python::convertible(PyObject * objPtr)
{
  return PyDict_Check(objPtr) ? objPtr : nullptr;
}

void game_features_map_from_python::construct(PyObject * objPtr, boost::python::converter::rvalue_from_python_stage1_data * data)
{
  void *storage = ((boost::python::converter::rvalue_from_python_storage<std::map<std::type_index, boost::any>>*)data)->storage.bytes;
  std::map<std::type_index, boost::any> *result = new (storage) std::map<std::type_index, boost::any>();
  boost::python::dict source(boost::python::handle<>(boost::python::borrowed(objPtr)));
  boost::python::list keys = source.keys();
  int len = boost::python::len(keys);
  for (int i = 0; i < len; ++i)
  {
    boost::python::object pyKey = keys[i];
    // pyKey should be a Boost.Python.class corresponding to a game feature.
    std::string className = boost::python::extract<std::string>(pyKey.attr("__name__"))();
    if (className == "BSAInvalidation")
      insertGameFeature<BSAInvalidation>(*result, source[pyKey]);
    else if (className == "DataArchives")
      insertGameFeature<DataArchives>(*result, source[pyKey]);
    else if (className == "GamePlugins")
      insertGameFeature<GamePlugins>(*result, source[pyKey]);
    else if (className == "LocalSavegames")
      insertGameFeature<LocalSavegames>(*result, source[pyKey]);
    else if (className == "SaveGameInfo")
      insertGameFeature<SaveGameInfo>(*result, source[pyKey]);
    else if (className == "ScriptExtender")
      insertGameFeature<ScriptExtender>(*result, source[pyKey]);
    else if (className == "UnmanagedMods")
      insertGameFeature<UnmanagedMods>(*result, source[pyKey]);
  }

  data->convertible = storage;
}

void registerGameFeaturesPythonConverters()
{
  namespace bpy = boost::python;

  game_features_map_from_python();

  // Features require defs for all methods as Python can access C++ features
  bpy::class_<BSAInvalidationWrapper, boost::noncopyable>("BSAInvalidation")
      .def("isInvalidationBSA", bpy::pure_virtual(&BSAInvalidation::isInvalidationBSA))
      .def("deactivate", bpy::pure_virtual(&BSAInvalidation::deactivate))
      .def("activate", bpy::pure_virtual(&BSAInvalidation::activate))
      ;

  bpy::class_<DataArchivesWrapper, boost::noncopyable>("DataArchives")
      .def("vanillaArchives", bpy::pure_virtual(&DataArchives::vanillaArchives))
      .def("archives", bpy::pure_virtual(&DataArchives::archives))
      .def("addArchive", bpy::pure_virtual(&DataArchives::addArchive))
      .def("removeArchive", bpy::pure_virtual(&DataArchives::removeArchive))
      ;

  bpy::class_<GamePluginsWrapper, boost::noncopyable>("GamePlugins")
      .def("writePluginLists", bpy::pure_virtual(&GamePlugins::writePluginLists))
      .def("readPluginLists", bpy::pure_virtual(&GamePlugins::readPluginLists))
      .def("getLoadOrder", bpy::pure_virtual(&GamePlugins::getLoadOrder))
      .def("lightPluginsAreSupported", bpy::pure_virtual(&GamePlugins::lightPluginsAreSupported))
      ;

  bpy::class_<LocalSavegamesWrapper, boost::noncopyable>("LocalSavegames")
      .def("mappings", bpy::pure_virtual(&LocalSavegames::mappings))
      .def("prepareProfile", bpy::pure_virtual(&LocalSavegames::prepareProfile))
      ;

  bpy::class_<SaveGameInfoWrapper, boost::noncopyable>("SaveGameInfo")
      .def("getSaveGameInfo", bpy::pure_virtual(&SaveGameInfo::getSaveGameInfo), bpy::return_value_policy<bpy::manage_new_object>())
      .def("getMissingAssets", bpy::pure_virtual(&SaveGameInfo::getMissingAssets))
      .def("getSaveGameWidget", bpy::pure_virtual(&SaveGameInfo::getSaveGameWidget), bpy::return_value_policy<bpy::manage_new_object>(), "[optional]")
      .def("hasScriptExtenderSave", bpy::pure_virtual(&SaveGameInfo::hasScriptExtenderSave))
      ;

  bpy::class_<ScriptExtenderWrapper, boost::noncopyable>("ScriptExtender")
      .def("BinaryName", bpy::pure_virtual(&ScriptExtender::BinaryName))
      .def("PluginPath", bpy::pure_virtual(&ScriptExtender::PluginPath))
      .def("loaderName", bpy::pure_virtual(&ScriptExtender::loaderName))
      .def("loaderPath", bpy::pure_virtual(&ScriptExtender::loaderPath))
      .def("saveGameAttachmentExtensions", bpy::pure_virtual(&ScriptExtender::saveGameAttachmentExtensions))
      .def("isInstalled", bpy::pure_virtual(&ScriptExtender::isInstalled))
      .def("getExtenderVersion", bpy::pure_virtual(&ScriptExtender::getExtenderVersion))
      .def("getArch", bpy::pure_virtual(&ScriptExtender::getArch))
      ;

  bpy::class_<UnmanagedModsWrapper, boost::noncopyable>("UnmanagedMods")
      .def("mods", bpy::pure_virtual(&UnmanagedMods::mods))
      .def("displayName", bpy::pure_virtual(&UnmanagedMods::displayName))
      .def("referenceFile", bpy::pure_virtual(&UnmanagedMods::referenceFile))
      .def("secondaryFiles", bpy::pure_virtual(&UnmanagedMods::secondaryFiles))
      ;
}
