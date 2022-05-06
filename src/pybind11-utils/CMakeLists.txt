cmake_minimum_required(VERSION 3.16)

add_library(pybind11-utils INTERFACE)
target_link_libraries(pybind11-utils INTERFACE pybind11::pybind11)
target_include_directories(pybind11-utils INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(pybind11::utils ALIAS pybind11-utils)
