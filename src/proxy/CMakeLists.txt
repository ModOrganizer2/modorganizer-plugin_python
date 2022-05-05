cmake_minimum_required(VERSION 3.16)

add_library(plugin_python SHARED)
mo2_configure_plugin(plugin_python
    WARNINGS 4
    EXTERNAL_WARNINGS 4
    EXTRA_TRANSLATIONS
    ${CMAKE_CURRENT_SOURCE_DIR}/../runner
    ${CMAKE_CURRENT_SOURCE_DIR}/../mobase
    ${CMAKE_CURRENT_SOURCE_DIR}/../pybind11-qt)
target_link_libraries(plugin_python PRIVATE pythonrunner)
mo2_install_target(plugin_python FOLDER)
