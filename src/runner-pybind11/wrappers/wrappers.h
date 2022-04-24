#ifndef PYTHON_WRAPPERS_WRAPPERS_H
#define PYTHON_WRAPPERS_WRAPPERS_H

#include <pybind11/pybind11.h>

namespace mo2::python {

    /**
     * @brief Add bindings for the various classes in uibase that are not
     * wrappers (i.e., cannot be extended from Python).
     *
     * @param m Python module to add bindings to.
     */
    void add_basic_bindings(pybind11::module_ m);

    /**
     * @brief Add bindings for the various custom widget classes in uibase that
     * cannot be extended from Python.
     *
     * @param m Python module to add bindings to.
     */
    void add_widget_bindings(pybind11::module_ m);

    /**
     * @brief Add bindings for the uibase wrappers to the given module. uibase
     * wrappers include classes from uibase that can be extended from Python but
     * are neither plugins nor game features (e.g., ISaveGame).
     *
     * @param m Python module to add bindings to.
     */
    void add_wrapper_bindings(pybind11::module_ m);

    /**
     * @brief Add bindings for the various plugin classes in uibase that can be
     * extended from Python.
     *
     * @param m Python module to add bindings to.
     */
    void add_plugins_bindings(pybind11::module_ m);

    /**
     * @brief Add bindings for the various game features classes in uibase that
     * can be extended from Python.
     *
     * @param m Python module to add bindings to.
     */
    void add_game_feature_bindings(pybind11::module_ m);

}  // namespace mo2::python

#endif  // PYTHON_WRAPPERS_WRAPPERS_H
