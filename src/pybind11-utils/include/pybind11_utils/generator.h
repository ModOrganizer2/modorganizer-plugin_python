#ifndef PYTHON_PYBIND11_GENERATOR_H
#define PYTHON_PYBIND11_GENERATOR_H

#include <generator>

#include <pybind11/pybind11.h>

namespace mo2::python {

    // create a Python generator from a C++ generator
    //
    template <typename T>
    auto register_generator_type(pybind11::handle scope,
                                 const char* name = "_mo2_generator")
    {
        namespace py = pybind11;

        return py::class_<std::generator<T>>(scope, name, py::module_local())
            .def("__iter__",
                 [](std::generator<T>& gen) -> std::generator<T>& {
                     return gen;
                 })
            .def("__next__", [](std::generator<T>& gen) {
                auto it = gen.begin();
                if (it != gen.end()) {
                    return *it;
                }
                else {
                    throw py::stop_iteration();
                }
            });
    }

}  // namespace mo2::python

#endif
