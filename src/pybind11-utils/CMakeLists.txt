cmake_minimum_required(VERSION 3.16)

add_library(pybind11-utils STATIC)
mo2_configure_library(pybind11-utils
	SOURCE_TREE
	WARNINGS 4
	EXTERNAL_WARNINGS 4
	AUTOMOC OFF
	TRANSLATIONS OFF
)
target_link_libraries(pybind11-utils PUBLIC pybind11::pybind11)
target_include_directories(pybind11-utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(pybind11::utils ALIAS pybind11-utils)
