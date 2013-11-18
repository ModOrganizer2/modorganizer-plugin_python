/*
Copyright (C) 2013 Sebastian Herbord. All rights reserved.

This file is part of python proxy plugin for MO

python proxy plugin is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Python proxy plugin is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with python proxy plugin.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "proxypython.h"
#include <utility.h>
#include <versioninfo.h>
#include <QtPlugin>
#include <QDirIterator>
#include <QWidget>
#include <QMessageBox>
#include <QCoreApplication>
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
  if (!m_MOInfo->pluginSetting(name(), "enabled").toBool()) {
    m_LoadFailure = FAIL_NONE;
    return false;
  }

  m_LoadFailure = FAIL_OTHER;
  if (QCoreApplication::applicationDirPath().contains(';')) {
    m_LoadFailure = FAIL_SEMICOLON;
    return true;
  }

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
    if (m_MOInfo->persistent(name(), "tryInit", false).toBool()) {
      if (pythonPath.isEmpty()) {
        m_LoadFailure = FAIL_PYTHONDETECTION;
      } else {
        m_LoadFailure = FAIL_WRONGPYTHONPATH;
      }
      if (QMessageBox::question(parentWidget(), tr("Python Initialization failed"),
        tr("On a previous start the Python Plugin failed to initialize.\n"
           "Either the value in Settings->Plugins->ProxyPython->plugin_dir is set incorrectly or it is empty and auto-detection doesn't work "
           "for whatever reason.\n"
           "Do you want to try initializing python again (at the risk of another crash)?\n"
           "Suggestion: Select \"no\", and click the warning sign for further help. Afterwards you have to re-enable the python plugin."),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::No) {
        m_MOInfo->setPluginSetting(name(), "enabled", false);
        return true;
      }
    }

    m_MOInfo->setPersistent(name(), "tryInit", true);
    m_Runner = CreatePythonRunner(moInfo, pythonPath);
    m_MOInfo->setPersistent(name(), "tryInit", false);

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
  return VersionInfo(1, 2, 0, VersionInfo::RELEASE_FINAL);
}

bool ProxyPython::isActive() const
{
  return m_LoadFailure == FAIL_NONE;
}

QList<PluginSetting> ProxyPython::settings() const
{
  QList<PluginSetting> result;
  result.push_back(PluginSetting("python_dir", "Path to your python installation. Leave empty for auto-detection", ""));
  result.push_back(PluginSetting("enabled", "Set to true to enable support for python plugins", true));
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
  } else if (m_LoadFailure == FAIL_PYTHONDETECTION) {
    result.push_back(PROBLEM_PYTHONDETECTION);
  } else if (m_LoadFailure == FAIL_INITFAIL) {
    result.push_back(PROBLEM_INITFAIL);
  } else if (m_LoadFailure == FAIL_SEMICOLON) {
    result.push_back(PROBLEM_SEMICOLON);
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
    case PROBLEM_PYTHONDETECTION: {
      return tr("Python auto-detection failed");
    } break;
    case PROBLEM_SEMICOLON: {
      return tr("ModOrganizer path contains a semicolon");
    } break;
    default:
      throw MyException(tr("invalid problem key %1").arg(key));
  }
}


QString ProxyPython::fullDescription(unsigned int key) const
{
  switch (key) {
    case PROBLEM_PYTHONMISSING: {
      return tr("Some MO plugins require the python interpreter to be installed. "
                "These plugins will not even show up in settings-&gt;plugins.<br>"
                "If you want to use those plugins, please install the 32-bit version of Python 2.7.x from <a href=\"%1\">%1</a>.<br>"
                "This is only required to use some extended functionality in MO, you do not need Python to play the game.").arg(s_DownloadPythonURL);
    } break;
    case PROBLEM_PYTHONWRONGVERSION: {
      return tr("Your installed python version has a different version than 2.7. "
                "Some MO plugins may not work.<br>"
                "If you have multiple versions of python installed you may have to configure the path to 2.7 (32 bit) "
                "in the settings dialog.<br>"
                "This is only required to use some extended functionality in MO, you do not need Python to play the game.");
    } break;
    case PROBLEM_WRONGPYTHONPATH: {
      return tr("Please set python_dir in Settings->Plugins->ProxyPython to the path of your python 2.7 (32 bit) installation.");
    } break;
    case PROBLEM_PYTHONDETECTION: {
      return tr("The auto-detection of the python path failed. I don't know why this would happen but you can try to fix it "
                "by setting python_dir in Settings->Plugins->ProxyPython to the path of your python 2.7 (32 bit) installation.");
    } break;
    case PROBLEM_INITFAIL: {
      return tr("Sorry, I don't know any details. Most likely your python installation is not supported.");
    } break;
    case PROBLEM_SEMICOLON: {
      return tr("The path to Mod Organizer (%1) contains a semicolon. <br>"
                "While this is legal on NTFS drives there is a lot of software that doesn't handle it correctly.<br>"
                "Unfortunately MO depends on libraries that seem to fall into that group.<br>"
                "As a result the python plugin can't be loaded.<br>"
                "The only solution I can offer is to remove the semicolon / move MO to a path without a semicolon.").arg(QCoreApplication::applicationDirPath());
    } break;
    default:
      throw MyException(tr("invalid problem key %1").arg(key));
  }
}

bool ProxyPython::hasGuidedFix(unsigned int key) const
{
  return (key == PROBLEM_PYTHONMISSING) || (key == PROBLEM_PYTHONWRONGVERSION);
}

void ProxyPython::startGuidedFix(unsigned int key) const
{
  if ((key == PROBLEM_PYTHONMISSING) || (key == PROBLEM_PYTHONWRONGVERSION)) {
    ::ShellExecuteA(NULL, "open", s_DownloadPythonURL, NULL, NULL, SW_SHOWNORMAL);
  }
}


#if QT_VERSION < QT_VERSION_CHECK(5,0,0)
Q_EXPORT_PLUGIN2(proxyPython, ProxyPython)
#endif
