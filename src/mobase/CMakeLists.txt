cmake_minimum_required(VERSION 3.16)

pybind11_add_module(mobase MODULE)
mo2_configure_library(mobase
	SOURCE_TREE
	WARNINGS OFF
	AUTOMOC ON
	TRANSLATIONS OFF
	PRIVATE_DEPENDS uibase Qt::Core
)
target_link_libraries(mobase PRIVATE pybind11::qt)
target_include_directories(mobase PRIVATE ${PYTHON_ROOT}/Include)

install(TARGETS mobase DESTINATION bin/plugins/plugin_python/libs)
