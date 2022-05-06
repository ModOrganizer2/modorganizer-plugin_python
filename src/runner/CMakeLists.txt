cmake_minimum_required(VERSION 3.16)

add_library(runner STATIC)
mo2_configure_library(runner
	SOURCE_TREE
	WARNINGS 4
	EXTERNAL_WARNINGS 4
	AUTOMOC ON
	TRANSLATIONS OFF
	PUBLIC_DEPENDS uibase Qt::Core
)
target_link_libraries(runner PRIVATE pybind11::embed pybind11::qt)
target_include_directories(runner PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
mo2_install_target(runner)

# force runner to build mobase
add_dependencies(runner mobase)
