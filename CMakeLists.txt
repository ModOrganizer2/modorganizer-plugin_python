cmake_minimum_required(VERSION 3.16)

if(DEFINED DEPENDENCIES_DIR)
	include(${DEPENDENCIES_DIR}/modorganizer_super/cmake_common/mo2.cmake)
else()
	include(${CMAKE_CURRENT_LIST_DIR}/cmake_common/mo2.cmake)
endif()

project(plugin_python)

# order matters!
add_subdirectory(src/runner)
add_subdirectory(src/proxy)
