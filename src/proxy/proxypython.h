#ifndef PROXYPYTHON_H
#define PROXYPYTHON_H


#include <ipluginproxy.h>
#include <iplugindiagnose.h>
#include <map>
#include <Windows.h>
#include <pythonrunner.h>


class ProxyPython : public QObject, MOBase::IPluginProxy, MOBase::IPluginDiagnose
{
  Q_OBJECT
  Q_INTERFACES(MOBase::IPlugin MOBase::IPluginProxy MOBase::IPluginDiagnose)
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
  Q_PLUGIN_METADATA(IID "org.tannin.ProxyPython" FILE "proxypython.json")
#endif

public:
  ProxyPython();
  ~ProxyPython();

  virtual bool init(MOBase::IOrganizer *moInfo);
  virtual QString name() const;
  virtual QString author() const;
  virtual QString description() const;
  virtual MOBase::VersionInfo version() const;
  virtual bool isActive() const;
  virtual QList<MOBase::PluginSetting> settings() const;

  QStringList pluginList(const QString &pluginPath) const;
  QObject *instantiate(const QString &pluginName);

  /**
   * @return the parent widget for newly created dialogs
   * @note needs to be public so it can be exposed to plugins
   */
  virtual QWidget *getParentWidget() { return parentWidget(); }

public: // IPluginDiagnose

  virtual std::vector<unsigned int> activeProblems() const;
  virtual QString shortDescription(unsigned int key) const;
  virtual QString fullDescription(unsigned int key) const;
  virtual bool hasGuidedFix(unsigned int key) const;
  virtual void startGuidedFix(unsigned int key) const;

private:

  static const unsigned int PROBLEM_PYTHONMISSING = 1;
  static const unsigned int PROBLEM_PYTHONWRONGVERSION = 2;
  static const char *s_DownloadPythonURL;

  const MOBase::IOrganizer *m_MOInfo;
  QString m_TempRunnerFile;
  HMODULE m_RunnerLib;
  IPythonRunner *m_Runner;

  enum {
    FAIL_NONE,
    FAIL_NOTINIT,
    FAIL_MISSINGDEPENDENCIES,
    FAIL_OTHER
  } m_LoadFailure;

};

#endif // PROXYPYTHON_H
