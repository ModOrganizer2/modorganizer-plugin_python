cmake_minimum_required(VERSION 3.16)

project(pythonrunner)
set(project_type plugin)
set(enable_warnings OFF)
set(enable_bigobj ON)
set(install_dir bin/plugins/data)
set(create_translations ON)

add_compile_definitions(
	QT_NO_KEYWORDS
	PYTHONRUNNER_LIBRARY)

# appveyor does not build modorganizer in its standard location, so use
# DEPENDENCIES_DIR to find cmake_common
if(DEFINED DEPENDENCIES_DIR)
	include(${DEPENDENCIES_DIR}/modorganizer_super/cmake_common/project.cmake)
	include(${DEPENDENCIES_DIR}/modorganizer_super/cmake_common/src.cmake)
else()
	include(../../../cmake_common/project.cmake)
	include(../../../cmake_common/src.cmake)
endif()

requires_project(game_features)
requires_library(python)

add_filter(NAME src/converters GROUPS
	converters
	pythonutils
	shared_ptr_converter
	tuple_helper
	variant_helper
)

add_filter(NAME src/runner GROUPS
	pythonrunner
	pylogger
	widgets
)

add_filter(NAME src/utils GROUPS
	error
	gilock
	sipapiaccess
)

add_filter(NAME src/wrappers GROUPS
	gamefeatureswrappers
	proxypluginwrappers
	pythonwrapperutilities
	uibasewrappers
)
