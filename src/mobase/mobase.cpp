#pragma warning(disable : 4100)
#pragma warning(disable : 4996)

#include <tuple>
#include <variant>

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

    py::module_::import("PyQt6.QtCore");
    py::module_::import("PyQt6.QtWidgets");

    // bindings
    //
    mo2::python::add_basic_bindings(m);
    mo2::python::add_wrapper_bindings(m);

    // game features must be added before plugins
    mo2::python::add_game_feature_bindings(m);

    mo2::python::add_plugins_bindings(m);

    // widgets
    //
    py::module_ widgets(
        py::reinterpret_borrow<py::module_>(PyImport_AddModule("mobase.widgets")));
    m.attr("widgets") = widgets;
    mo2::python::add_widget_bindings(widgets);

    // functions
    //
    m.def("getFileVersion", &MOBase::getFileVersion, py::arg("filepath"));
    m.def("getProductVersion", &MOBase::getProductVersion, py::arg("executable"));
    m.def("getIconForExecutable", &MOBase::iconForExecutable, py::arg("executable"));

    // expose MoVariant - MoVariant is a fake object whose only purpose is to be
    // used as a type-hint on the python side (e.g., def foo(x:
    // mobase.MoVariant))
    //
    // the real MoVariant is defined in the generated stubs, since it's only
    // relevant when doing type-checking, but this needs to be defined,
    // otherwise MoVariant is not found when actually running plugins through
    // MO2, making them crash
    m.attr("MoVariant") = py::none();

    // private stuff for debugging/test
    py::module_ moprivate(
        py::reinterpret_borrow<py::module_>(PyImport_AddModule("mobase.private")));
    m.attr("private") = moprivate;

    // expose a function to create a particular tree, only for debugging
    // purpose, not in mobase.
    mo2::python::add_make_tree_function(moprivate);
    moprivate.def("extract_plugins", &mo2::python::extract_plugins);
}
