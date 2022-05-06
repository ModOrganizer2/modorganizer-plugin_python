#pragma warning(disable : 4100)
#pragma warning(disable : 4996)

#include <tuple>
#include <variant>

#include <pybind11/embed.h>

#include "pybind11_all.h"

#include "wrappers/pyfiletree.h"
#include "wrappers/wrappers.h"

// TODO: remove these include (only for testing)
#include <QDir>
#include <QFile>
#include <QWidget>
#include <fmt/format.h>
#include <iplugin.h>
#include <iplugindiagnose.h>
#include <ipluginfilemapper.h>
#include <iplugingame.h>
#include <iplugininstaller.h>
#include <iplugininstallersimple.h>
#include <ipluginlist.h>
#include <ipluginmodpage.h>
#include <ipluginpreview.h>
#include <iplugintool.h>
#include <isavegame.h>
#include <isavegameinfowidget.h>

using namespace MOBase;
namespace py = pybind11;

PYBIND11_MODULE(mobase, m)
{
    using namespace mo2::python;

    m.add_object("PyQt6", py::module_::import("PyQt6"));
    m.add_object("PyQt6.QtCore", py::module_::import("PyQt6.QtCore"));
    m.add_object("PyQt6.QtGui", py::module_::import("PyQt6.QtGui"));
    m.add_object("PyQt6.QtWidgets", py::module_::import("PyQt6.QtWidgets"));

    // bindings
    //
    mo2::python::add_basic_bindings(m);
    mo2::python::add_wrapper_bindings(m);

    // game features must be added before plugins
    mo2::python::add_game_feature_bindings(m);

    mo2::python::add_plugins_bindings(m);

    // widgets
    //
    py::module_ widgets = m.def_submodule("widgets");
    mo2::python::add_widget_bindings(widgets);

    // functions
    //
    m.def("getFileVersion", wrap_for_filepath(&MOBase::getFileVersion),
          py::arg("filepath"));
    m.def("getProductVersion", wrap_for_filepath(&MOBase::getProductVersion),
          py::arg("executable"));
    m.def("getIconForExecutable", wrap_for_filepath(&MOBase::iconForExecutable),
          py::arg("executable"));

    // typing stuff to be consistent with stubs and allow plugin developers to properly
    // type their code if they want
    {
        m.add_object("Path", py::module_::import("pathlib").attr("Path"));

        auto s = m.attr("__dict__");

        // expose MoVariant - MoVariant is a fake object whose only purpose is to be
        // used as a type-hint on the python side (e.g., def foo(x:
        // mobase.MoVariant))
        //
        // the real MoVariant is defined in the generated stubs, since it's only
        // relevant when doing type-checking, but this needs to be defined,
        // otherwise MoVariant is not found when actually running plugins through
        // MO2, making them crash
        //
        m.add_object(
            "MoVariant",
            py::eval("None | bool | int | str | list[object] | dict[str, object]"));

        // same things for FileWrapper and DirectoryWrapper
        //
        m.add_object("FileWrapper", py::eval("str | PyQt6.QtCore.QFileInfo | Path", s));
        m.add_object("DirectoryWrapper", py::eval("str | PyQt6.QtCore.QDir | Path", s));
    }

    // private stuff for debugging/test
    py::module_ moprivate = m.def_submodule("private");

    // expose a function to create a particular tree, only for debugging
    // purpose, not in mobase.
    mo2::python::add_make_tree_function(moprivate);
    moprivate.def("extract_plugins", &mo2::python::extract_plugins);
}
