cmake_minimum_required(VERSION 3.16)

pybind11_add_module(pythonrunner SHARED)
mo2_configure_library(pythonrunner
	WARNINGS OFF
	AUTOMOC ON
	TRANSLATIONS OFF
	PRIVATE_DEPENDS uibase Qt::Core
)
set_target_properties(pythonrunner
	PROPERTIES
	OUTPUT_NAME "pythonrunner"
	PREFIX ""
	SUFFIX ".dll")
target_link_libraries(pythonrunner PRIVATE pybind11::embed)
target_include_directories(pythonrunner PRIVATE ${PYTHON_ROOT}/Include)

# this is kind of broken but it only works with this...
target_compile_definitions(pythonrunner
	PRIVATE QT_NO_KEYWORDS PYTHONRUNNER_LIBRARY)
mo2_install_target(pythonrunner INSTALLDIR bin/plugins/data)
