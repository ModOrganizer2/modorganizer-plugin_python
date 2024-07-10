cmake_minimum_required(VERSION 3.16)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

mo2_find_python_executable(PYTHON_EXE)

add_library(pybind11-qt STATIC)
mo2_configure_target(pybind11-qt
	SOURCE_TREE
	WARNINGS 4
	EXTERNAL_WARNINGS 4
	AUTOMOC OFF
	TRANSLATIONS OFF
)
target_link_libraries(pybind11-qt PUBLIC pybind11::pybind11 PRIVATE Qt6::Core Qt6::Widgets)
target_include_directories(pybind11-qt PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# this is kind of broken but it only works with this...
target_compile_definitions(pybind11-qt PUBLIC QT_NO_KEYWORDS)

# we need sip.h for pybind11-qt
add_custom_target(PyQt6-siph DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/sip.h")
add_custom_command(
	OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/sip.h"
	COMMAND
		${CMAKE_COMMAND} -E env PYTHONPATH=${CMAKE_BINARY_DIR}/pylibs
		${CMAKE_BINARY_DIR}/pylibs/bin/sip-module.exe
		--sip-h PyQt${MO2_QT_VERSION_MAJOR}.sip
		--target-dir ${CMAKE_CURRENT_BINARY_DIR}
)
add_dependencies(PyQt6-siph PyQt6)
add_dependencies(pybind11-qt PyQt6-siph)

target_include_directories(pybind11-qt PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

add_library(pybind11::qt ALIAS pybind11-qt)
