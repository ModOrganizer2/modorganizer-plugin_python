#include "wrappers.h"

#include <pybind11/functional.h>
#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "../pybind11_qt/pybind11_qt.h"

#include <moddatachecker.h>
#include <scriptextender.h>

#include "pyfiletree.h"

namespace py = pybind11;
using namespace MOBase;
using namespace pybind11::literals;

namespace mo2::python {

    class PyModDataChecker : public ModDataChecker {
    public:
        CheckReturn
        dataLooksValid(std::shared_ptr<const MOBase::IFileTree> fileTree) const override
        {
            PYBIND11_OVERRIDE_PURE(CheckReturn, ModDataChecker, dataLooksValid,
                                   fileTree);
        }

        std::shared_ptr<MOBase::IFileTree>
        fix(std::shared_ptr<MOBase::IFileTree> fileTree) const override
        {
            PYBIND11_OVERRIDE(std::shared_ptr<MOBase::IFileTree>, ModDataChecker, fix,
                              fileTree);
        }
    };

    class PyScriptExtender : public ScriptExtender {
    public:
        QString BinaryName() const override
        {
            PYBIND11_OVERRIDE_PURE(QString, ScriptExtender, BinaryName, );
        }

        QString PluginPath() const override
        {
            PYBIND11_OVERRIDE_PURE(QString, ScriptExtender, PluginPath, );
        }

        QString loaderName() const override
        {
            PYBIND11_OVERRIDE_PURE(QString, ScriptExtender, loaderName, );
        }

        QString loaderPath() const override
        {
            PYBIND11_OVERRIDE_PURE(QString, ScriptExtender, loaderPath, );
        }

        QString savegameExtension() const override
        {
            PYBIND11_OVERRIDE_PURE(QString, ScriptExtender, savegameExtension, );
        }

        bool isInstalled() const override
        {
            PYBIND11_OVERRIDE_PURE(bool, ScriptExtender, isInstalled, );
        }

        QString getExtenderVersion() const override
        {
            PYBIND11_OVERRIDE_PURE(QString, ScriptExtender, getExtenderVersion, );
        }

        WORD getArch() const override
        {
            PYBIND11_OVERRIDE_PURE(WORD, ScriptExtender, getArch, );
        }
    };

    void add_game_feature_bindings(pybind11::module_ m)
    {
        // ModDataChecker

        py::class_<ModDataChecker, PyModDataChecker> pyModDataChecker(m,
                                                                      "ModDataChecker");

        py::enum_<ModDataChecker::CheckReturn>(pyModDataChecker, "CheckReturn")
            .value("INVALID", ModDataChecker::CheckReturn::INVALID)
            .value("FIXABLE", ModDataChecker::CheckReturn::FIXABLE)
            .value("VALID", ModDataChecker::CheckReturn::VALID)
            .export_values();

        pyModDataChecker.def(py::init<>())
            .def("dataLooksValid", &ModDataChecker::dataLooksValid, "filetree"_a)
            .def("fix", &ModDataChecker::fix, "filetree"_a);

        // ScriptExtender

        py::class_<ScriptExtender, PyScriptExtender>(m, "ScriptExtender")
            .def(py::init<>())
            .def("BinaryName", &ScriptExtender::BinaryName)
            .def("PluginPath", &ScriptExtender::PluginPath)
            .def("loaderName", &ScriptExtender::loaderName)
            .def("loaderPath", &ScriptExtender::loaderPath)
            .def("savegameExtension", &ScriptExtender::savegameExtension)
            .def("isInstalled", &ScriptExtender::isInstalled)
            .def("getExtenderVersion", &ScriptExtender::getExtenderVersion)
            .def("getArch", &ScriptExtender::getArch);
    }

}  // namespace mo2::python
