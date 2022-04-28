cmake_minimum_required(VERSION 3.16)

if(DEFINED DEPENDENCIES_DIR)
	include(${DEPENDENCIES_DIR}/modorganizer_super/cmake_common/mo2.cmake)
else()
	include(${CMAKE_CURRENT_LIST_DIR}/../cmake_common/mo2.cmake)
endif()

set(PYTHON_BUILD_PATH ${PYTHON_ROOT}/PCBuild/amd64)
set(PYVERSION 310)

set(PYBIND11_FINDPYTHON true)
set(PYTHON_EXECUTABLE ${PYTHON_BUILD_PATH}/python.exe)
set(PYTHON_INCLUDE_DIRS ${PYTHON_ROOT}/Include)
set(PYTHON_LIBRARIES ${MO2_INSTALL_LIBS_PATH}/python${PYVERSION}.lib)

add_subdirectory(${MO2_BUILD_PATH}/pybind11 ${CMAKE_CURRENT_BINARY_DIR}/pybind11)

project(plugin_python)

# order matters!
add_subdirectory(src/pybind11-qt)
add_subdirectory(src/mobase)
add_subdirectory(src/runner)
add_subdirectory(src/proxy)

# force plugin_python to build mobase
add_dependencies(plugin_python mobase)

# add_subdirectory(tests)

set_property(DIRECTORY ${PROJECT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT plugin_python)
