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

#ifndef PROXYPYTHON_H
#define PROXYPYTHON_H

#include <map>
#include <memory>

#include <Windows.h>

#include <ipluginloader.h>

#include <pythonrunner.h>

class ProxyPython : public MOBase::IPluginLoader {
    Q_OBJECT
    Q_INTERFACES(MOBase::IPluginLoader)
    Q_PLUGIN_METADATA(IID "org.mo2.ProxyPython")

public:
    ProxyPython();

    bool initialize(QString& errorMessage) override;
    QList<QList<QObject*>> load(const MOBase::PluginExtension& extension) override;
    void unload(const MOBase::PluginExtension& extension) override;
    void unloadAll() override;

private:
    enum class FailureType : unsigned int {
        NONE           = 0,
        SEMICOLON      = 1,
        DLL_NOT_FOUND  = 2,
        INVALID_DLL    = 3,
        INITIALIZATION = 4
    };

    static QString failureMessage(FailureType failureType);

private:
    HMODULE m_RunnerLib;
    std::unique_ptr<mo2::python::IPythonRunner> m_Runner;
    std::unordered_map<const MOBase::PluginExtension*,
                       std::vector<std::filesystem::path>>
        m_ExtensionModules;
};

#endif  // PROXYPYTHON_H
