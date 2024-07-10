cmake_minimum_required(VERSION 3.16)

find_package(mo2-uibase CONFIG REQUIRED)

add_library(runner SHARED)
mo2_configure_target(runner
	SOURCE_TREE
	WARNINGS 4
	EXTERNAL_WARNINGS 4
	AUTOMOC ON
	TRANSLATIONS OFF
)
target_link_libraries(runner PUBLIC mo2::uibase PRIVATE pybind11::embed pybind11::qt)
target_include_directories(runner PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(runner PRIVATE RUNNER_BUILD)

# proxy will install runner

# force runner to build mobase
add_dependencies(runner mobase)
