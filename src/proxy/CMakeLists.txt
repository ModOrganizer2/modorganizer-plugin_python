cmake_minimum_required(VERSION 3.16)
include(../../../cmake_common/src.cmake)

requires_library(python)
target_include_directories(${PROJECT_NAME} PRIVATE
	${CMAKE_SOURCE_DIR}/src/runner)
