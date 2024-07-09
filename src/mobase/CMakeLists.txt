cmake_minimum_required(VERSION 3.16)

find_package(Qt6 COMPONENTS Core)
find_package(mo2-uibase CONFIG REQUIRED)

pybind11_add_module(mobase MODULE)
mo2_configure_library(mobase
	SOURCE_TREE
	WARNINGS 4
	EXTERNAL_WARNINGS 4
	AUTOMOC ON
	TRANSLATIONS OFF
)
target_link_libraries(mobase PRIVATE pybind11::qt pybind11::utils mo2::uibase Qt6::Core)
