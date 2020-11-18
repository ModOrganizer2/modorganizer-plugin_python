#ifndef GAMEFEATURESWRAPPERS_H
#define GAMEFEATURESWRAPPERS_H

#include <map>

#include <bsainvalidation.h>
#include <dataarchives.h>
#include <gameplugins.h>
#include <localsavegames.h>
#include <moddatachecker.h>
#include <moddatacontent.h>
#include <savegameinfo.h>
#include <scriptextender.h>
#include <unmanagedmods.h>

// this might need turning off if Q_MOC_RUN is defined
#include <boost/python.hpp>
#include <boost/mp11.hpp>

// This is a simple MPL list that contains all the game features in one place:
using MpGameFeaturesList = boost::mp11::mp_list<
  BSAInvalidation,
  DataArchives,
  GamePlugins,
  LocalSavegames,
  ModDataChecker,
  ModDataContent,
  SaveGameInfo,
  ScriptExtender,
  UnmanagedMods
>;

/////////////////////////////
/// Wrapper declarations

class BSAInvalidationWrapper : public BSAInvalidation, public boost::python::wrapper<BSAInvalidation>
{
public:
  static constexpr const char* className = "BSAInvalidationWrapper";
  using boost::python::wrapper<BSAInvalidation>::get_override;

  virtual bool isInvalidationBSA(const QString &bsaName) override;
  virtual void deactivate(MOBase::IProfile *profile) override;
  virtual void activate(MOBase::IProfile *profile) override;
  virtual bool prepareProfile(MOBase::IProfile *profile) override;
};

class DataArchivesWrapper : public DataArchives, public boost::python::wrapper<DataArchives>
{
public:
  static constexpr const char* className = "DataArchivesWrapper";
  using boost::python::wrapper<DataArchives>::get_override;

  virtual QStringList vanillaArchives() const override;
  virtual QStringList archives(const MOBase::IProfile *profile) const override;
  virtual void addArchive(MOBase::IProfile *profile, int index, const QString &archiveName) override;
  virtual void removeArchive(MOBase::IProfile *profile, const QString &archiveName) override;
};

class GamePluginsWrapper : public GamePlugins, public boost::python::wrapper<GamePlugins>
{
public:
  static constexpr const char* className = "GamePluginsWrapper";
  using boost::python::wrapper<GamePlugins>::get_override;

  virtual void writePluginLists(const MOBase::IPluginList *pluginList) override;
  virtual void readPluginLists(MOBase::IPluginList *pluginList) override;
  virtual QStringList getLoadOrder() override;
  virtual bool lightPluginsAreSupported() override;
};

class LocalSavegamesWrapper : public LocalSavegames, public boost::python::wrapper<LocalSavegames>
{
public:
  static constexpr const char* className = "LocalSavegamesWrapper";
  using boost::python::wrapper<LocalSavegames>::get_override;

  virtual MappingType mappings(const QDir &profileSaveDir) const override;
  virtual bool prepareProfile(MOBase::IProfile *profile) override;
};

class ModDataCheckerWrapper : public ModDataChecker, public boost::python::wrapper<ModDataChecker>
{
public:
  static constexpr const char* className = "ModDataCheckerWrapper";
  using boost::python::wrapper<ModDataChecker>::get_override;

  virtual CheckReturn dataLooksValid(std::shared_ptr<const MOBase::IFileTree> fileTree) const override;
  virtual std::shared_ptr<MOBase::IFileTree> fix(std::shared_ptr<MOBase::IFileTree> fileTree) const override;
};

class ModDataContentWrapper : public ModDataContent, public boost::python::wrapper<ModDataContent>
{
public:
  static constexpr const char* className = "ModDataContentWrapper";
  using boost::python::wrapper<ModDataContent>::get_override;

  virtual std::vector<Content> getAllContents() const override;
  virtual std::vector<int> getContentsFor(std::shared_ptr<const MOBase::IFileTree> fileTree) const override;

};

class SaveGameInfoWrapper : public SaveGameInfo, public boost::python::wrapper<SaveGameInfo>
{
public:
  static constexpr const char* className = "SaveGameInfoWrapper";
  using boost::python::wrapper<SaveGameInfo>::get_override;

  virtual MissingAssets getMissingAssets(MOBase::ISaveGame const& save) const override;
  virtual MOBase::ISaveGameInfoWidget *getSaveGameWidget(QWidget *parent = 0) const override;

private:
  // We need to keep the python objects alive:
  mutable std::map<QString, boost::python::object> m_SaveGames;
  mutable boost::python::object m_SaveGameWidget;
};

class ScriptExtenderWrapper : public ScriptExtender, public boost::python::wrapper<ScriptExtender>
{
public:
  static constexpr const char* className = "ScriptExtenderWrapper";
  using boost::python::wrapper<ScriptExtender>::get_override;

  virtual QString BinaryName() const override;
  virtual QString PluginPath() const override;
  virtual QString loaderName() const override;
  virtual QString loaderPath() const override;
  virtual QStringList saveGameAttachmentExtensions() const override;
  virtual bool isInstalled() const override;
  virtual QString getExtenderVersion() const override;
  virtual WORD getArch() const  override;
};

class UnmanagedModsWrapper : public UnmanagedMods, public boost::python::wrapper<UnmanagedMods>
{
public:
  static constexpr const char* className = "UnmanagedModsWrapper";
  using boost::python::wrapper<UnmanagedMods>::get_override;

  virtual QStringList mods(bool onlyOfficial) const override;
  virtual QString displayName(const QString &modName) const override;
  virtual QFileInfo referenceFile(const QString &modName) const override;
  virtual QStringList secondaryFiles(const QString &modName) const override;
};

/// end Wrapper declarations
/////////////////////////////

struct game_features_map_from_python
{
  game_features_map_from_python();
  static void *convertible(PyObject *objPtr);
  static void construct(PyObject *objPtr, boost::python::converter::rvalue_from_python_stage1_data *data);
};

void registerGameFeaturesPythonConverters();

#endif // GAMEFEATURESWRAPPERS_H
