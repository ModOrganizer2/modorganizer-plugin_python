/*
Copyright (C) 2022 Sebastian Herbord & MO2 Team. All rights reserved.

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

#include <filesystem>

#include <Windows.h>

#include <QCoreApplication>
#include <QDirIterator>
#include <QMessageBox>
#include <QWidget>
#include <QtPlugin>

#include <log.h>
#include <utility.h>
#include <versioninfo.h>

namespace fs = std::filesystem;
using namespace MOBase;

// retrieve the path to the folder containing the proxy DLL
fs::path getPluginFolder()
{
    wchar_t path[MAX_PATH];
    HMODULE hm = NULL;

    if (GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                              GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                          (LPCWSTR)&getPluginFolder, &hm) == 0) {
        return {};
    }
    if (GetModuleFileName(hm, path, sizeof(path)) == 0) {
        return {};
    }

    return fs::path(path).parent_path();
}

ProxyPython::ProxyPython() : m_RunnerLib{nullptr}, m_Runner{nullptr} {}

bool ProxyPython::initialize(QString& errorMessage)
{
    errorMessage = "";

    if (QCoreApplication::applicationDirPath().contains(';')) {
        errorMessage = failureMessage(FailureType::SEMICOLON);
        return true;
    }

    const auto pluginFolder = getPluginFolder();
    if (pluginFolder.empty()) {
        DWORD error  = ::GetLastError();
        errorMessage = failureMessage(FailureType::DLL_NOT_FOUND);
        log::error("failed to resolve Python proxy directory ({}): {}", error,
                   qUtf8Printable(windowsErrorString(::GetLastError())));
        return false;
    }

    // load the pythonrunner library, this is done in multiple steps:
    //
    // 1. we set the dlls/ subfolder (from the plugin) as the DLL directory so Windows
    // will look for DLLs in it, this is required to find the Python and libffi DLL, but
    // also the runner DLL
    //
    const auto dllPaths = pluginFolder / "dlls";
    if (SetDllDirectoryW(dllPaths.c_str()) == 0) {
        DWORD error  = ::GetLastError();
        errorMessage = failureMessage(FailureType::DLL_NOT_FOUND);
        log::error("failed to add python DLL directory ({}): {}", error,
                   qUtf8Printable(windowsErrorString(::GetLastError())));
        return false;
    }

    // 2. we create the Python runner, we do not need to use ::LinkLibrary and
    // ::GetProcAddress because we use delayed load for the runner DLL (see the
    // CMakeLists.txt)
    //
    m_Runner = mo2::python::createPythonRunner();

    if (m_Runner) {
        const auto libpath = pluginFolder / "libs";
        const std::vector<fs::path> paths{libpath / "pythoncore.zip", libpath};
        m_Runner->initialize(paths);
    }

    // reset DLL directory
    SetDllDirectoryW(NULL);

    if (!m_Runner || !m_Runner->isInitialized()) {
        errorMessage = failureMessage(FailureType::INITIALIZATION);
        return false;
    }
    else {
        m_Runner->addDllSearchPath(pluginFolder / "dlls");
    }

    return true;
}

QList<QList<QObject*>> ProxyPython::load(const PluginExtension& extension)
{
    if (!m_Runner) {
        return {};
    }

    // currently, only handle __init__.py directly in the folder
    const auto pyIniFile = extension.directory() / "__init__.py";
    if (!exists(pyIniFile)) {
        return {};
    }

    m_ExtensionModules[&extension] = {pyIniFile};

    QList<QList<QObject*>> plugins;
    for (auto&& pythonModule : m_ExtensionModules[&extension]) {
        plugins.append(m_Runner->load(pythonModule));
    }

    return plugins;
}

void ProxyPython::unload(const PluginExtension& extension)
{
    if (!m_Runner) {
        return;
    }

    if (auto it = m_ExtensionModules.find(&extension); it != m_ExtensionModules.end()) {
        for (auto&& pythonModule : it->second) {
            m_Runner->unload(pythonModule);
        }
        m_ExtensionModules.erase(it);
    }
}

void ProxyPython::unloadAll()
{
    if (m_Runner) {
        for (auto& [ext, modules] : m_ExtensionModules) {
            for (auto& pythonModule : modules) {
                m_Runner->unload(pythonModule);
            }
        }
    }
    m_ExtensionModules.clear();
}

QString ProxyPython::failureMessage(FailureType key)
{
    switch (key) {
    case FailureType::SEMICOLON:
        return tr("The path to Mod Organizer (%1) contains a semicolon. <br>"
                  "While this is legal on NTFS drives, many softwares do not handle it "
                  "correctly.<br>"
                  "Unfortunately MO depends on libraries that seem to fall into that "
                  "group.<br>"
                  "As a result the python plugin cannot be loaded, and the only "
                  "solution we can"
                  "offer is to remove the semicolon or move MO to a path without a "
                  "semicolon.")
            .arg(QCoreApplication::applicationDirPath());
    case FailureType::DLL_NOT_FOUND:
        return tr("The Python plugin DLL was not found, maybe your antivirus deleted "
                  "it. Re-installing MO2 might fix the problem.");
    case FailureType::INVALID_DLL:
        return tr(
            "The Python plugin DLL is invalid, maybe your antivirus is blocking it. "
            "Re-installing MO2 and adding exclusions for it to your AV might fix the "
            "problem.");
    case FailureType::INITIALIZATION:
        return tr("The initialization of the Python plugin DLL failed, unfortunately "
                  "without any details.");
    default:
        return tr("no failure");
    }
}
