#include "wrappers.h"

#include <pybind11/functional.h>
#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "../pybind11_qt/pybind11_qt.h"

#include <moddatachecker.h>

namespace py = pybind11;
using namespace MOBase;

namespace mo2::python {

    void add_game_feature_bindings(pybind11::module_ m) {}

}  // namespace mo2::python
