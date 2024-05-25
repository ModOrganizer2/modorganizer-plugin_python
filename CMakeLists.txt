cmake_minimum_required(VERSION 3.16)

cmake_policy(SET CMP0144 NEW)

if(DEFINED DEPENDENCIES_DIR)
	include(${DEPENDENCIES_DIR}/modorganizer_super/cmake_common/mo2.cmake)
else()
	include(${CMAKE_CURRENT_LIST_DIR}/../cmake_common/mo2.cmake)
endif()

project(plugin_python CXX)

set(PYTHON_BUILD_PATH ${PYTHON_ROOT}/PCBuild/amd64)

# find Python - lots of "Hints" since we have a weird setup
set(Python_USE_STATIC_LIBS False)
set(Python_INCLUDE_DIR ${PYTHON_ROOT}/Include)
set(Python_EXECUTABLE ${PYTHON_BUILD_PATH}/python.exe)
file(GLOB Python_LIBRARY ${PYTHON_BUILD_PATH}/python[0-9][0-9]*.lib)
find_package(Python COMPONENTS Interpreter Development REQUIRED)

# pybind11 needs uppercase (at least EXECUTABLE and LIBRARY)
set(PYTHON_EXECUTABLE ${Python_EXECUTABLE})
set(PYTHON_INCLUDE_DIR ${Python_INCLUDE_DIR})
set(PYTHON_LIBRARY ${Python_LIBRARY})

# useful for naming DLL, zip, etc. (3.10 -> 310)
set(Python_VERSION_SHORT ${Python_VERSION_MAJOR}${Python_VERSION_MINOR})

# pybind11
add_subdirectory(${MO2_BUILD_PATH}/pybind11 ${CMAKE_CURRENT_BINARY_DIR}/pybind11)

# projects
add_subdirectory(src)
set_property(DIRECTORY ${PROJECT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT proxy)

# tests (if requested)
set(PLUGIN_PYTHON_TESTS ${PLUGIN_PYTHON_TESTS} CACHE BOOL "build tests for plugin_python")
if (PLUGIN_PYTHON_TESTS)
	enable_testing()
	add_subdirectory(tests)
endif()
