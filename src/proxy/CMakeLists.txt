cmake_minimum_required(VERSION 3.16)

add_library(plugin_python SHARED)
mo2_configure_plugin(plugin_python
    WARNINGS OFF
    EXTRA_TRANSLATIONS ${CMAKE_CURRENT_SOURCE_DIR}/../runner)
target_link_libraries(plugin_python PRIVATE pythonrunner)
mo2_install_target(plugin_python FOLDER)
