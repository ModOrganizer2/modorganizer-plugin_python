cmake_minimum_required(VERSION 3.16)

if(DEFINED DEPENDENCIES_DIR)
	include(${DEPENDENCIES_DIR}/modorganizer_super/cmake_common/mo2.cmake)
else()
	include(${CMAKE_CURRENT_LIST_DIR}/../cmake_common/mo2.cmake)
endif()

set(PYBIND11_FINDPYTHON true)
set(PYTHON_EXECUTABLE ${PYTHON_ROOT}/PCBuild/amd64/python.exe)
set(PYTHON_INCLUDE_DIRS ${PYTHON_ROOT}/Include)
set(PYTHON_LIBRARIES ${PYTHON_ROOT}/PCBuild/amd64/python310.lib)

add_subdirectory(pybind11)

project(plugin_python)

# order matters!
# add_subdirectory(src/runner)
add_subdirectory(src/runner-pybind11)
add_subdirectory(src/proxy)
