#pragma warning( push )
#pragma warning( disable : 4100 ) // a lot of unreferenced formal parameters from boost libraries
#pragma warning( disable : 4996 ) // strncpy claimed to be unsafe

#include "proxypython.h"

#pragma warning( pop )

#include <utility.h>
#include <versioninfo.h>
#include <QtPlugin>
#include <QDirIterator>
#include <QWidget>
#include "resource.h"


using namespace MOBase;


const char *ProxyPython::s_DownloadPythonURL = "http://www.python.org/download/releases/";


HMODULE GetOwnModuleHandle()
{
  HMODULE hMod = NULL;
  GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
      reinterpret_cast<LPCWSTR>(&GetOwnModuleHandle), &hMod);

  return hMod;
}


QString ExtractResource(WORD resourceID, const QString &szFilename)
{
  bool success = false;

  HMODULE mod = GetOwnModuleHandle();

  HRSRC hResource = FindResourceW(mod, MAKEINTRESOURCE(resourceID), L"BINARY");
  if (hResource == NULL) {
    throw MyException("embedded dll not available: " + windowsErrorString(::GetLastError()));
  }

  HGLOBAL hFileResource = LoadResource(mod, hResource);
  if (hFileResource == NULL) {
    throw MyException("failed to load embedded dll resource: " + windowsErrorString(::GetLastError()));
  }

  LPVOID lpFile = LockResource(hFileResource);
  DWORD dwSize = SizeofResource(mod, hResource);

  QString outFile = QDir::tempPath() + "/" + szFilename;

  HANDLE hFile = CreateFileW(ToWString(outFile).c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  HANDLE hFileMap = CreateFileMapping(hFile, NULL, PAGE_READWRITE, 0, dwSize, NULL);
  LPVOID lpAddress = MapViewOfFile(hFileMap, FILE_MAP_WRITE, 0, 0, 0);

  CopyMemory(lpAddress, lpFile, dwSize);

  UnmapViewOfFile(lpAddress);

  CloseHandle(hFileMap);
  CloseHandle(hFile);

  return outFile;
}


ProxyPython::ProxyPython()
  : m_MOInfo(NULL), m_Runner(NULL), m_LoadFailure(FAIL_NOTINIT)
{
}

ProxyPython::~ProxyPython()
{
  if (!m_TempRunnerFile.isEmpty()) {
    ::FreeLibrary(m_RunnerLib);
    QFile(m_TempRunnerFile).remove();
  }
}

typedef IPythonRunner* (*CreatePythonRunner_func)(const MOBase::IOrganizer *moInfo, const QString &pythonPath);


bool ProxyPython::init(IOrganizer *moInfo)
{
  m_MOInfo = moInfo;

  m_LoadFailure = FAIL_OTHER;

  QString pythonPath = m_MOInfo->pluginSetting(name(), "python_dir").toString();

  if (!pythonPath.isEmpty() && !QFile::exists(pythonPath + "/python.exe")) {
    m_LoadFailure = FAIL_WRONGPYTHONPATH;
    return true;
  }


  m_TempRunnerFile = ExtractResource(IDR_LOADER_DLL, "__pythonRunner.dll");
  m_RunnerLib = ::LoadLibraryW(ToWString(m_TempRunnerFile).c_str());
  if (m_RunnerLib != NULL) {
    CreatePythonRunner_func CreatePythonRunner = (CreatePythonRunner_func)::GetProcAddress(m_RunnerLib, "CreatePythonRunner");
    if (CreatePythonRunner == NULL) {
      throw MyException("embedded dll is invalid: " + windowsErrorString(::GetLastError()));
    }
    m_Runner = CreatePythonRunner(moInfo, pythonPath);
    if (m_Runner != NULL) {
      m_LoadFailure = FAIL_NONE;
    } else {
      m_LoadFailure = FAIL_INITFAIL;
    }
    return true;
  } else {
    DWORD error = ::GetLastError();
    qCritical("Failed to load python runner: %s", qPrintable(windowsErrorString(error)));
    if (error == ERROR_MOD_NOT_FOUND) {
      m_LoadFailure = FAIL_MISSINGDEPENDENCIES;
    }
    return true;
  }
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
  return VersionInfo(1, 1, 0, VersionInfo::RELEASE_FINAL);
}

bool ProxyPython::isActive() const
{
  return m_LoadFailure == FAIL_NONE;
}

QList<PluginSetting> ProxyPython::settings() const
{
  QList<PluginSetting> result;
  result.push_back(PluginSetting("python_dir", "Path to your python installation. Leave empty for auto-detection", ""));
  return result;
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


QObject *ProxyPython::instantiate(const QString &pluginName)
{
  if (m_Runner != NULL) {
    QObject *result = m_Runner->instantiate(pluginName);
    return result;
  } else {
    return NULL;
  }
}


std::vector<unsigned int> ProxyPython::activeProblems() const
{
  std::vector<unsigned int> result;
  if (m_LoadFailure == FAIL_MISSINGDEPENDENCIES) {
    result.push_back(PROBLEM_PYTHONMISSING);
  } else if (m_LoadFailure == FAIL_WRONGPYTHONPATH) {
    result.push_back(PROBLEM_WRONGPYTHONPATH);
  } else if (m_LoadFailure == FAIL_INITFAIL) {
    result.push_back(PROBLEM_INITFAIL);
  } else if (m_Runner != NULL) {
    if (!m_Runner->isPythonInstalled()) {
      // don't know how this could happen but wth
      result.push_back(PROBLEM_PYTHONMISSING);
    }
    if (!m_Runner->isPythonVersionSupported()) {
      result.push_back(PROBLEM_PYTHONWRONGVERSION);
    }
  }

  return result;
}

QString ProxyPython::shortDescription(unsigned int key) const
{
  switch (key) {
    case PROBLEM_PYTHONMISSING: {
      return tr("Python not installed or not found");
    } break;
    case PROBLEM_PYTHONWRONGVERSION: {
      return tr("Python version is incompatible");
    } break;
    case PROBLEM_WRONGPYTHONPATH: {
      return tr("Invalid python path");
    } break;
    case PROBLEM_INITFAIL: {
      return tr("Initializing Python failed");
    } break;
    default:
      throw MyException(tr("invalid problem key %1").arg(key));
  }
}


QString ProxyPython::fullDescription(unsigned int key) const
{
  switch (key) {
    case PROBLEM_PYTHONMISSING: {
      return tr("Some plugins require the python interpreter to be installed. "
                "These plugins will not even show up in settings-&gt;plugins.<br>"
                "If you want to use those plugins, please install the 32-bit version of Python 2.7.x from <a href=\"%1\">%1</a>.").arg(s_DownloadPythonURL);
    } break;
    case PROBLEM_PYTHONWRONGVERSION: {
      return tr("Your installed python version has a different version than 2.7. "
                "Some plugins may not work.<br>"
                "If you have multiple versions of python installed you may have to configure the path to 2.7 "
                "in the settings dialog.");
    } break;
    case PROBLEM_WRONGPYTHONPATH: {
      return tr("Please set python_dir in Settings->Plugins->ProxyPython to the path of your python 2.7 installation.");
    } break;
    case PROBLEM_INITFAIL: {
      return tr("Sorry, I don't know any details. Most likely your python installation is not supported.");
    } break;
    default:
      throw MyException(tr("invalid problem key %1").arg(key));
  }
}

bool ProxyPython::hasGuidedFix(unsigned int key) const
{
  return (key == PROBLEM_PYTHONMISSING) || (key == PROBLEM_PYTHONWRONGVERSION);
}

void ProxyPython::startGuidedFix(unsigned int) const
{
  ::ShellExecuteA(NULL, "open", s_DownloadPythonURL, NULL, NULL, SW_SHOWNORMAL);
}


#if QT_VERSION < QT_VERSION_CHECK(5,0,0)
Q_EXPORT_PLUGIN2(proxyPython, ProxyPython)
#endif
