#include "gamefeatureswrappers.h"

#include <boost/any.hpp>

#include <typeindex>

#include <ipluginlist.h>
#include <iprofile.h>
#include <isavegame.h>
#include <isavegameinfowidget.h>

#include "ifiletree.h"
#include "pythonwrapperutilities.h"

/////////////////////////////
/// BSAInvalidation Wrapper


bool BSAInvalidationWrapper::isInvalidationBSA(const QString &bsaName)
{
  return basicWrapperFunctionImplementation<bool>(this, "isInvalidationBSA", bsaName);
}

void BSAInvalidationWrapper::deactivate(MOBase::IProfile *profile)
{
  return basicWrapperFunctionImplementation<void>(this, "deactivate", boost::python::ptr(profile));
}

void BSAInvalidationWrapper::activate(MOBase::IProfile *profile)
{
  return basicWrapperFunctionImplementation<void>(this, "activate", boost::python::ptr(profile));
}

bool BSAInvalidationWrapper::prepareProfile(MOBase::IProfile *profile)
{
  return basicWrapperFunctionImplementation<bool>(this, "prepareProfile", boost::python::ptr(profile));
}
/// end BSAInvalidation Wrapper
/////////////////////////////
/// DataArchives Wrapper


QStringList DataArchivesWrapper::vanillaArchives() const
{
  return basicWrapperFunctionImplementation<QStringList>(this, "vanillaArchives");
}

QStringList DataArchivesWrapper::archives(const MOBase::IProfile *profile) const
{
  return basicWrapperFunctionImplementation<QStringList>(this, "archives", boost::python::ptr(profile));
}

void DataArchivesWrapper::addArchive(MOBase::IProfile *profile, int index, const QString &archiveName)
{
  return basicWrapperFunctionImplementation<void>(this, "addArchive", boost::python::ptr(profile), index, archiveName);
}

void DataArchivesWrapper::removeArchive(MOBase::IProfile *profile, const QString &archiveName)
{
  return basicWrapperFunctionImplementation<void>(this, "removeArchive", boost::python::ptr(profile), archiveName);
}
/// end DataArchives Wrapper
/////////////////////////////
/// GamePlugins Wrapper


void GamePluginsWrapper::writePluginLists(const MOBase::IPluginList * pluginList)
{
  return basicWrapperFunctionImplementation<void>(this, "writePluginLists", boost::python::ptr(pluginList));
}

void GamePluginsWrapper::readPluginLists(MOBase::IPluginList * pluginList)
{
  return basicWrapperFunctionImplementation<void>(this, "readPluginLists", boost::python::ptr(pluginList));
}

QStringList GamePluginsWrapper::getLoadOrder()
{
  return basicWrapperFunctionImplementation<QStringList>(this, "getLoadOrder");
}

bool GamePluginsWrapper::lightPluginsAreSupported()
{
    return basicWrapperFunctionImplementation<bool>(this, "lightPluginsAreSupported");
}

/// end GamePlugins Wrapper
/////////////////////////////
/// LocalSavegames Wrapper


MappingType LocalSavegamesWrapper::mappings(const QDir & profileSaveDir) const
{
  return basicWrapperFunctionImplementation<MappingType>(this, "mappings", profileSaveDir);
}

bool LocalSavegamesWrapper::prepareProfile(MOBase::IProfile * profile)
{
  return basicWrapperFunctionImplementation<bool>(this, "prepareProfile", boost::python::ptr(profile));
}

/// end LocalSavegames Wrapper
/////////////////////////////
/// ModDataChecker Wrapper

ModDataChecker::CheckReturn ModDataCheckerWrapper::dataLooksValid(std::shared_ptr<const MOBase::IFileTree> fileTree) const {
  return basicWrapperFunctionImplementation<CheckReturn>(this, "dataLooksValid", fileTree);
}

std::shared_ptr<MOBase::IFileTree> ModDataCheckerWrapper::fix(std::shared_ptr<MOBase::IFileTree> fileTree) const {
  return basicWrapperFunctionImplementationWithDefault<std::shared_ptr<MOBase::IFileTree>>(this, [](auto&&... args) { return nullptr; }, "fix", fileTree);
}

/// end ModDataChecker Wrapper
/////////////////////////////
/// ModDataContent Wrapper

std::vector<ModDataContent::Content> ModDataContentWrapper::getAllContents() const {
  return basicWrapperFunctionImplementation<std::vector<Content>>(this, "getAllContents");
}
std::vector<int> ModDataContentWrapper::getContentsFor(std::shared_ptr<const MOBase::IFileTree> fileTree) const {
  return basicWrapperFunctionImplementation<std::vector<int>>(this, "getContentsFor", fileTree);
}

/// end ModDataContent Wrapper
/////////////////////////////
/// SaveGameInfo Wrapper


MOBase::ISaveGame const * SaveGameInfoWrapper::getSaveGameInfo(QString const & file) const
{
  return basicWrapperFunctionImplementation<MOBase::ISaveGame*>(this, m_SaveGames[file], "getSaveGameInfo", file);
}

SaveGameInfoWrapper::MissingAssets SaveGameInfoWrapper::getMissingAssets(QString const & file) const
{
  return basicWrapperFunctionImplementation<SaveGameInfoWrapper::MissingAssets>(this, "getMissingAssets", file);
}

MOBase::ISaveGameInfoWidget* SaveGameInfoWrapper::getSaveGameWidget(QWidget* parent) const
{
  return basicWrapperFunctionImplementation<MOBase::ISaveGameInfoWidget*>(this, m_SaveGameWidget, "getSaveGameWidget", parent);
}

bool SaveGameInfoWrapper::hasScriptExtenderSave(QString const & file) const
{
  return basicWrapperFunctionImplementation<bool>(this, "hasScriptExtenderSave", file);
}
/// end SaveGameInfo Wrapper
/////////////////////////////
/// ScriptExtender Wrapper

QString ScriptExtenderWrapper::BinaryName() const
{
  return basicWrapperFunctionImplementation<QString>(this, "BinaryName");
}

QString ScriptExtenderWrapper::PluginPath() const
{
  return basicWrapperFunctionImplementation<QString>(this, "PluginPath");
}

QString ScriptExtenderWrapper::loaderName() const
{
  return basicWrapperFunctionImplementation<QString>(this, "loaderName");
}

QString ScriptExtenderWrapper::loaderPath() const
{
  return basicWrapperFunctionImplementation<QString>(this, "loaderPath");
}

QStringList ScriptExtenderWrapper::saveGameAttachmentExtensions() const
{
  return basicWrapperFunctionImplementation<QStringList>(this, "saveGameAttachmentExtensions");
}

bool ScriptExtenderWrapper::isInstalled() const
{
  return basicWrapperFunctionImplementation<bool>(this, "isInstalled");
}

QString ScriptExtenderWrapper::getExtenderVersion() const
{
  return basicWrapperFunctionImplementation<QString>(this, "getExtenderVersion");
}

WORD ScriptExtenderWrapper::getArch() const
{
  return basicWrapperFunctionImplementation<WORD>(this, "getArch");
}

/// end ScriptExtender Wrapper
/////////////////////////////
/// UnmanagedMods Wrapper


QStringList UnmanagedModsWrapper::mods(bool onlyOfficial) const
{
  return basicWrapperFunctionImplementation<QStringList>(this, "mods", onlyOfficial);
}

QString UnmanagedModsWrapper::displayName(const QString & modName) const
{
  return basicWrapperFunctionImplementation<QString>(this, "displayName", modName);
}

QFileInfo UnmanagedModsWrapper::referenceFile(const QString & modName) const
{
  return basicWrapperFunctionImplementation<QFileInfo>(this, "referenceFile", modName);
}

QStringList UnmanagedModsWrapper::secondaryFiles(const QString & modName) const
{
  return basicWrapperFunctionImplementation<QStringList>(this, "secondaryFiles", modName);
}
/// end UnmanagedMods Wrapper
/////////////////////////////


game_features_map_from_python::game_features_map_from_python()
{
  boost::python::converter::registry::push_back(&convertible, &construct, boost::python::type_id<std::map<std::type_index, boost::any>>());
}

void * game_features_map_from_python::convertible(PyObject * objPtr)
{
  return PyDict_Check(objPtr) ? objPtr : nullptr;
}

template<typename T>
void insertGameFeature(std::map<std::type_index, boost::any>& map, const boost::python::object& pyObject)
{
  map[std::type_index(typeid(T))] = boost::python::extract<T*>(pyObject)();
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
    boost::python::object pyValue = source[pyKey];

    boost::mp11::mp_for_each<
      // Must user pointers because mp_for_each construct object:
      boost::mp11::mp_transform<std::add_pointer_t, MpGameFeaturesList>
    >([&](auto* pt) {
      using T = std::remove_pointer_t<decltype(pt)>;
      boost::python::extract<T*> extract(pyValue);
      if (extract.check()) {
        (*result)[std::type_index(typeid(T))] = extract();
      }
    });
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

  auto modDataCheckerClass = bpy::class_<ModDataCheckerWrapper, boost::noncopyable>("ModDataChecker");
  {
    bpy::scope scope = modDataCheckerClass;

    bpy::enum_<ModDataChecker::CheckReturn>("CheckReturn")
      .value("INVALID", ModDataChecker::CheckReturn::INVALID)
      .value("FIXABLE", ModDataChecker::CheckReturn::FIXABLE)
      .value("VALID", ModDataChecker::CheckReturn::VALID)
      .export_values()
      ;

    modDataCheckerClass
      .def("dataLooksValid", bpy::pure_virtual(&ModDataChecker::dataLooksValid))
      .def("fix", bpy::pure_virtual(&ModDataChecker::fix))
      ;
  }

  {
    bpy::scope scope = bpy::class_<ModDataContentWrapper, boost::noncopyable>("ModDataContent")
      .def("getAllContents", bpy::pure_virtual(&ModDataContent::getAllContents))
      .def("getContentsFor", bpy::pure_virtual(&ModDataContent::getContentsFor))
      ;

    bpy::class_<ModDataContent::Content>("Content", bpy::init<int, QString, QString, bpy::optional<bool>>())
      .add_property("id", &ModDataContent::Content::id)
      .add_property("name", &ModDataContent::Content::name)
      .add_property("icon", &ModDataContent::Content::icon)
      .def("isOnlyForFilter", &ModDataContent::Content::isOnlyForFilter)
      ;

  }

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
