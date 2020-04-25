cmake_minimum_required(VERSION 3.16)

project(pythonrunner)
set(project_type plugin)
set(enable_warnings OFF)
set(install_dir bin/plugins/data)

add_compile_definitions(
	QT_NO_KEYWORDS
	PYTHONRUNNER_LIBRARY)

include(../../../cmake_common/project.cmake)
include(../../../cmake_common/src.cmake)

requires_project(game_features)
requires_library(python)
