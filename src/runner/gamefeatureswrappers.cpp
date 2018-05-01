#include "gamefeatureswrappers.h"

#include <boost/any.hpp>

#include <typeindex>

#include <ipluginlist.h>
#include <iprofile.h>
#include <isavegame.h>
#include <isavegameinfowidget.h>

#include "gilock.h"
#include "pycatch.h"

/////////////////////////////
/// BSAInvalidation Wrapper


bool BSAInvalidationWrapper::isInvalidationBSA(const QString &bsaName)
{
  GILock lock;

  try {
    return this->get_override("isInvalidationBSA")(bsaName);
  } PYCATCH;
}

void BSAInvalidationWrapper::deactivate(MOBase::IProfile *profile)
{
  GILock lock;

  try {
    this->get_override("deactivate")(boost::python::ptr(profile));
  } PYCATCH;
}

void BSAInvalidationWrapper::activate(MOBase::IProfile *profile)
{
  GILock lock;

  try {
    this->get_override("activate")(boost::python::ptr(profile));
  } PYCATCH;
}
/// end BSAInvalidation Wrapper
/////////////////////////////
/// DataArchives Wrapper


QStringList DataArchivesWrapper::vanillaArchives() const
{
  GILock lock;

  try {
    return this->get_override("vanillaArchives")();
  } PYCATCH;
}

QStringList DataArchivesWrapper::archives(const MOBase::IProfile *profile) const
{
  GILock lock;

  try {
    return this->get_override("archives")(boost::python::ptr(profile));
  } PYCATCH;
}

void DataArchivesWrapper::addArchive(MOBase::IProfile *profile, int index, const QString &archiveName)
{
  GILock lock;

  try {
    this->get_override("addArchive")(boost::python::ptr(profile), index, archiveName);
  } PYCATCH;
}

void DataArchivesWrapper::removeArchive(MOBase::IProfile * profile, const QString & archiveName)
{
  GILock lock;

  try {
    this->get_override("removeArchive")(boost::python::ptr(profile), archiveName);
  } PYCATCH;
}
/// end DataArchives Wrapper
/////////////////////////////
/// GamePlugins Wrapper


void GamePluginsWrapper::writePluginLists(const MOBase::IPluginList * pluginList)
{
  GILock lock;
  
  try {
    this->get_override("writePluginLists")(boost::python::ptr(pluginList));
  } PYCATCH;
}

void GamePluginsWrapper::readPluginLists(MOBase::IPluginList * pluginList)
{
  GILock lock;
  
  try {
    this->get_override("readPluginLists")(boost::python::ptr(pluginList));
  } PYCATCH;
}
/// end GamePlugins Wrapper
/////////////////////////////
/// LocalSavegames Wrapper


MappingType LocalSavegamesWrapper::mappings(const QDir & profileSaveDir) const
{
  GILock lock;

  try {
    return this->get_override("mappings")(profileSaveDir);
  } PYCATCH;
}

void LocalSavegamesWrapper::prepareProfile(MOBase::IProfile * profile)
{
  GILock lock;

  try {
    this->get_override("prepareProfile")(boost::python::ptr(profile));
  } PYCATCH;
}
/// end LocalSavegames Wrapper
/////////////////////////////
/// SaveGameInfo Wrapper


MOBase::ISaveGame const * SaveGameInfoWrapper::getSaveGameInfo(QString const & file) const
{
  GILock lock;

  try {
    return this->get_override("getSaveGameInfo")(file);
  } PYCATCH;
}

SaveGameInfoWrapper::MissingAssets SaveGameInfoWrapper::getMissingAssets(QString const & file) const
{
  GILock lock;

  try {
    return this->get_override("getMissingAssets")(file);
  } PYCATCH;
}

MOBase::ISaveGameInfoWidget * SaveGameInfoWrapper::getSaveGameWidget(QWidget * parent) const
{
  qCritical("Calling method with unimplemented from_python converter.");

  GILock lock;

  try {
    return this->get_override("getSaveGameWidget")(boost::python::ptr(parent));
  } PYCATCH;
}

bool SaveGameInfoWrapper::hasScriptExtenderSave(QString const & file) const
{
  GILock lock;

  try {
    return this->get_override("hasScriptExtenderSave")(file);
  } PYCATCH;
}
/// end SaveGameInfo Wrapper
/////////////////////////////
/// ScriptExtender Wrapper

QString ScriptExtenderWrapper::BinaryName() const
{
  GILock lock;

  try {
    return this->get_override("BinaryName")();
  } PYCATCH;
}

QString ScriptExtenderWrapper::PluginPath() const
{
  GILock lock;

  try {
    return this->get_override("PluginPath")();
  } PYCATCH;
}

QString ScriptExtenderWrapper::loaderName() const
{
  GILock lock;

  try {
    return this->get_override("loaderName")();
  } PYCATCH;
}

QString ScriptExtenderWrapper::loaderPath() const
{
  GILock lock;

  try {
    return this->get_override("loaderPath")();
  } PYCATCH;
}

QStringList ScriptExtenderWrapper::saveGameAttachmentExtensions() const
{
  GILock lock;

  try {
    return this->get_override("saveGameAttachmentExtensions")();
  } PYCATCH;
}

bool ScriptExtenderWrapper::isInstalled() const
{
  GILock lock;

  try {
    return this->get_override("isInstalled")();
  } PYCATCH;
}

QString ScriptExtenderWrapper::getExtenderVersion() const
{
  GILock lock;

  try {
    return this->get_override("getExtenderVersion")();
  } PYCATCH;
}

WORD ScriptExtenderWrapper::getArch() const
{
  GILock lock;

  try {
    return this->get_override("getArch")();
  } PYCATCH;
}

/// end ScriptExtender Wrapper
/////////////////////////////
/// UnmanagedMods Wrapper


QStringList UnmanagedModsWrapper::mods(bool onlyOfficial) const
{
  GILock lock;

  try {
    return this->get_override("mods")(onlyOfficial);
  } PYCATCH;
}

QString UnmanagedModsWrapper::displayName(const QString & modName) const
{
  GILock lock;

  try {
    return this->get_override("displayName")(modName);
  } PYCATCH;
}

QFileInfo UnmanagedModsWrapper::referenceFile(const QString & modName) const
{
  GILock lock;

  try {
    return this->get_override("referenceFile")(modName);
  } PYCATCH;
}

QStringList UnmanagedModsWrapper::secondaryFiles(const QString & modName) const
{
  GILock lock;

  try {
    return this->get_override("secondaryFiles")(modName);
  } PYCATCH;
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
      ;

  bpy::class_<LocalSavegamesWrapper, boost::noncopyable>("LocalSavegames")
      .def("mappings", bpy::pure_virtual(&LocalSavegames::mappings))
      .def("prepareProfile", bpy::pure_virtual(&LocalSavegames::prepareProfile))
      ;

  bpy::class_<SaveGameInfoWrapper, boost::noncopyable>("SaveGameInfo")
      .def("getSaveGameInfo", bpy::pure_virtual(&SaveGameInfo::getSaveGameInfo), bpy::return_value_policy<bpy::manage_new_object>())
      .def("getMissingAssets", bpy::pure_virtual(&SaveGameInfo::getMissingAssets))
      .def("getSaveGameWidget", bpy::pure_virtual(&SaveGameInfo::getSaveGameWidget), bpy::return_value_policy<bpy::manage_new_object>())
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
