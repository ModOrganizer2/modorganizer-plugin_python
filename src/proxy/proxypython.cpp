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
#include "log.h"
#include <QCoreApplication>
#include <QDirIterator>
#include <QMessageBox>
#include <QWidget>
#include <QtPlugin>
#include <utility.h>
#include <versioninfo.h>

using namespace MOBase;

ProxyPython::ProxyPython()
    : m_MOInfo{nullptr}, m_RunnerLib{nullptr}, m_Runner{nullptr},
      m_LoadFailure(FailureType::NONE)
{
}

bool ProxyPython::init(IOrganizer* moInfo)
{
    using CreatePythonRunner_func = IPythonRunner* (*)();

    m_MOInfo = moInfo;

    if (m_MOInfo && !m_MOInfo->isPluginEnabled(this)) {
        return false;
    }

    if (QCoreApplication::applicationDirPath().contains(';')) {
        m_LoadFailure = FailureType::SEMICOLON;
        return true;
    }

    // load the pythonrunner library
    m_RunnerLib = ::LoadLibraryW(
        QDir::toNativeSeparators(IOrganizer::getPluginDataPath() + "/pythonRunner.dll")
            .toStdWString()
            .c_str());

    if (!m_RunnerLib) {
        DWORD error = ::GetLastError();
        log::error("failed to load python runner ({}): {}",
                   qUtf8Printable(windowsErrorString(error)));
        if (error == ERROR_MOD_NOT_FOUND) {
            m_LoadFailure = FailureType::DLL_NOT_FOUND;
        }
        else {
            m_LoadFailure = FailureType::INVALID_DLL;
        }
        return true;
    }

    const CreatePythonRunner_func createPythonRunner =
        (CreatePythonRunner_func)::GetProcAddress(m_RunnerLib, "CreatePythonRunner");
    if (!createPythonRunner) {
        m_LoadFailure = FailureType::INVALID_DLL;
        return true;
    }

    if (m_MOInfo && m_MOInfo->persistent(name(), "tryInit", false).toBool()) {
        // m_LoadFailure = FailureType::INITIALIZATION;
        // if (QMessageBox::question(parentWidget(), tr("Python Initialization failed"),
        //   tr("On a previous start the Python Plugin failed to initialize.\n"
        //      "Do you want to try initializing python again (at the risk of another
        //      crash)?\n" "Suggestion: Select \"no\", and click the warning sign for
        //      further help. Afterwards you have to re-enable the python plugin."),
        //   QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::No) {

        //   // we force enabled here (note: this is a persistent settings since MO2 2.4
        //   or something), plugin
        //   // usually should not handle enabled/disabled themselves but this is a base
        //   plugin so... m_MOInfo->setPersistent(name(), "enabled", false, true);
        //   return true;
        // }
    }

    if (m_MOInfo) {
        m_MOInfo->setPersistent(name(), "tryInit", true);
    }

    m_Runner = std::unique_ptr<IPythonRunner>{createPythonRunner()};

    if (m_MOInfo) {
        m_MOInfo->setPersistent(name(), "tryInit", false);
    }

    if (!m_Runner) {
        m_LoadFailure = FailureType::INITIALIZATION;
    }

    return true;
}

QString ProxyPython::name() const
{
    return "Python Proxy";
}

QString ProxyPython::localizedName() const
{
    return tr("Python Proxy");
}

QString ProxyPython::author() const
{
    return "AnyOldName3, Holt59, Silarn, Tannin";
}

QString ProxyPython::description() const
{
    return tr("Proxy Plugin to allow plugins written in python to be loaded");
}

VersionInfo ProxyPython::version() const
{
    return VersionInfo(2, 3, 0, VersionInfo::RELEASE_FINAL);
}

QList<PluginSetting> ProxyPython::settings() const
{
    return {};
}

QStringList ProxyPython::pluginList(const QDir& pluginPath) const
{
    QDir dir(pluginPath);
    dir.setFilter(dir.filter() | QDir::NoDotAndDotDot);
    QDirIterator iter(dir);

    // Note: We put python script (.py) and directory names, not the __init__.py
    // files in those since it is easier for the runner to import them.
    QStringList result;
    while (iter.hasNext()) {
        QString name   = iter.next();
        QFileInfo info = iter.fileInfo();

        if (info.isFile() && name.endsWith(".py")) {
            result.append(name);
        }
        else if (info.isDir() && QDir(info.absoluteFilePath()).exists("__init__.py")) {
            result.append(name);
        }
    }

    return result;
}

QList<QObject*> ProxyPython::load(const QString& identifier)
{
    if (!m_Runner) {
        return {};
    }

    auto plugins = m_Runner->load(identifier);
    for (auto* plugin : plugins) {
        plugin->setParent(this);
    }
    return plugins;
}

void ProxyPython::unload(const QString& identifier)
{
    if (m_Runner) {
        return m_Runner->unload(identifier);
    }
}

std::vector<unsigned int> ProxyPython::activeProblems() const
{
    auto failure = m_LoadFailure;

    // don't know how this could happen but wth
    if (m_Runner && !m_Runner->isPythonInitialized()) {
        failure = FailureType::INITIALIZATION;
    }

    if (failure != FailureType::NONE) {
        return {static_cast<std::underlying_type_t<FailureType>>(failure)};
    }

    return {};
}

QString ProxyPython::shortDescription(unsigned int key) const
{
    switch (static_cast<FailureType>(key)) {
    case FailureType::SEMICOLON:
        return tr("ModOrganizer path contains a semicolon");
    case FailureType::DLL_NOT_FOUND:
        return tr("Python DLL not found");
    case FailureType::INVALID_DLL:
        return tr("Invalid Python DLL");
    case FailureType::INITIALIZATION:
        return tr("Initializing Python failed");
    default:
        return tr("invalid problem key %1").arg(key);
    }
}

QString ProxyPython::fullDescription(unsigned int key) const
{
    switch (static_cast<FailureType>(key)) {
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
        return tr("invalid problem key %1").arg(key);
    }
}

bool ProxyPython::hasGuidedFix(unsigned int key) const
{
    return false;
}

void ProxyPython::startGuidedFix(unsigned int key) const {}
