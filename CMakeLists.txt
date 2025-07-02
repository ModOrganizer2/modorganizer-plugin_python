cmake_minimum_required(VERSION 3.16)

cmake_policy(SET CMP0144 NEW)

project(plugin_python CXX)

# we need mo2-cmake to obtain the Python version, but mo2-cmake will set
# CMAKE_MAP_IMPORTED_CONFIG_* which will trigger a tons of CMP0111 warnings for Python
# below so we need to reset these before finding Python and then reset them after
find_package(mo2-cmake CONFIG REQUIRED)

set(_CMAKE_MAP_IMPORTED_CONFIG_MINSIZEREL ${CMAKE_MAP_IMPORTED_CONFIG_MINSIZEREL})
set(_CMAKE_MAP_IMPORTED_CONFIG_RELWITHDEBINFO ${CMAKE_MAP_IMPORTED_CONFIG_RELWITHDEBINFO})
set(_CMAKE_MAP_IMPORTED_CONFIG_RELEASE ${CMAKE_MAP_IMPORTED_CONFIG_RELEASE})

set(CMAKE_MAP_IMPORTED_CONFIG_MINSIZEREL "")
set(CMAKE_MAP_IMPORTED_CONFIG_RELWITHDEBINFO "")
set(CMAKE_MAP_IMPORTED_CONFIG_RELEASE "")

# find Python before include mo2-cmake, otherwise this will trigger a bunch of CMP0111
# due to the imported configuration mapping variables defined in mo2.cmake
find_package(Python ${MO2_PYTHON_VERSION} EXACT COMPONENTS Interpreter Development REQUIRED)
find_package(pybind11 CONFIG REQUIRED)

set(CMAKE_MAP_IMPORTED_CONFIG_MINSIZEREL ${_CMAKE_MAP_IMPORTED_CONFIG_MINSIZEREL})
set(CMAKE_MAP_IMPORTED_CONFIG_RELWITHDEBINFO ${_CMAKE_MAP_IMPORTED_CONFIG_RELWITHDEBINFO})
set(CMAKE_MAP_IMPORTED_CONFIG_RELEASE ${_CMAKE_MAP_IMPORTED_CONFIG_RELEASE})

get_filename_component(Python_HOME ${Python_EXECUTABLE} PATH)
set(Python_DLL_DIR "${Python_HOME}/DLLs")
set(Python_LIB_DIR "${Python_HOME}/Lib")

mo2_python_install_pyqt()

# useful for naming DLL, zip, etc. (3.10 -> 310)
set(Python_VERSION_SHORT ${Python_VERSION_MAJOR}${Python_VERSION_MINOR})

# projects
add_subdirectory(src)

# tests (if requested)
set(BUILD_TESTING ${BUILD_TESTING} CACHE BOOL "build tests for plugin_python")
if (BUILD_TESTING)
	enable_testing()
	add_subdirectory(tests)
endif()
