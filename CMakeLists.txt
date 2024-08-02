cmake_minimum_required(VERSION 3.16)

cmake_policy(SET CMP0144 NEW)

project(plugin_python CXX)

set(Python_FIND_VIRTUALENV STANDARD)

# find Python before include mo2-cmake, otherwise this will trigger a bunch of CMP0111
# due to the imported configuration mapping variables defined in mo2.cmake
find_package(Python ${MO2_PYTHON_VERSION} COMPONENTS Interpreter Development REQUIRED)
find_package(pybind11 CONFIG REQUIRED)

find_package(mo2-cmake CONFIG REQUIRED)

get_filename_component(Python_HOME ${Python_EXECUTABLE} PATH)
set(Python_DLL_DIR "${Python_HOME}/DLLs")
set(Python_LIB_DIR "${Python_HOME}/Lib")

mo2_python_install_pyqt()

# useful for naming DLL, zip, etc. (3.10 -> 310)
set(Python_VERSION_SHORT ${Python_VERSION_MAJOR}${Python_VERSION_MINOR})

# projects
add_subdirectory(src)

# tests (if requested)
set(PLUGIN_PYTHON_TESTING ${BUILD_TESTING} CACHE BOOL "build tests for plugin_python")
if (PLUGIN_PYTHON_TESTING)
	enable_testing()
	add_subdirectory(tests)
endif()
