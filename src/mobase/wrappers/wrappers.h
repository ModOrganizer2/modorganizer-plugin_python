#ifndef PYTHON_WRAPPERS_WRAPPERS_H
#define PYTHON_WRAPPERS_WRAPPERS_H

#include <any>
#include <map>
#include <typeindex>

#include <pybind11/pybind11.h>

#include <QList>
#include <QObject>

#include <iplugingame.h>

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
     * @brief Extract plugins from the given object. For each plugin implemented, an
     * object is returned.
     *
     * The returned QObject* are set as owner of the given object so that the Python
     * object lifetime does not end immediately after returning to C++.
     *
     * @param object Python object to extract plugins from.
     *
     * @return a QObject* for each plugin implemented by the given object.
     */
    QList<QObject*> extract_plugins(pybind11::object object);

    /**
     * @brief Add bindings for the various game features classes in uibase that
     * can be extended from Python.
     *
     * @param m Python module to add bindings to.
     */
    void add_game_feature_bindings(pybind11::module_ m);

    /**
     * @brief Create the game feature corresponding to the given Python type from the
     * given game.
     *
     * @param game Game plugin to extract the feature from.
     * @param type Type of the feature to extract.
     *
     * @return the feature from the game, or None is the game as no such feature.
     */
    pybind11::object extract_feature(MOBase::IPluginGame const& game,
                                     pybind11::object type);

    /**
     * @brief Create Python dictionary mapping game feature classes to the game feature
     * instances for the given game.
     *
     * @param game Game plugin to extract features from.
     *
     * @return a python dictionary mapping feature types (in Python) to feature objects.
     */
    pybind11::dict extract_feature_list(MOBase::IPluginGame const& game);

    /**
     * @brief Convert the given python map of features to a C++ one.
     *
     * @param py_features Python features to convert (type to feature).
     *
     * @return the map of features.
     */
    std::map<std::type_index, std::any>
    convert_feature_list(pybind11::dict const& py_features);

}  // namespace mo2::python

#endif  // PYTHON_WRAPPERS_WRAPPERS_H
