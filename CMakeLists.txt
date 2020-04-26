cmake_minimum_required(VERSION 3.16)

project(plugin_python)
set(project_type plugin)
set(enable_warnings OFF)

include(../cmake_common/project.cmake)
add_subdirectory(src/proxy)

add_subdirectory(src/runner)
