cmake_minimum_required(VERSION 3.16)

# need to find Boost here with a dummy component to get Boost_LIBRARY_DIRS
find_package(Boost COMPONENTS thread REQUIRED)

pybind11_add_module(pythonrunner SHARED)
mo2_configure_library(pythonrunner
	WARNINGS OFF
	AUTOMOC ON
	TRANSLATIONS ON
	PRIVATE_DEPENDS uibase boost Qt::Core
)
set_target_properties(pythonrunner
	PROPERTIES
	OUTPUT_NAME "pythonrunner"
	PREFIX ""
	SUFFIX ".dll")
# target_link_libraries(pythonrunner PRIVATE pybind11::embed)
# target_include_directories(pythonrunner PRIVATE ${PYTHON_ROOT}/Include)

# this is kind of broken but it only works with this...
target_link_directories(pythonrunner
	PRIVATE
	${MO2_INSTALL_LIBS_PATH}
	${Boost_LIBRARY_DIRS})
target_compile_definitions(pythonrunner
	PRIVATE QT_NO_KEYWORDS PYTHONRUNNER_LIBRARY)
mo2_install_target(pythonrunner INSTALLDIR bin/plugins/data)

mo2_add_filter(NAME src/converters GROUPS
	converters
	pythonutils
	shared_ptr_converter
	tuple_helper
	variant_helper
)

mo2_add_filter(NAME src/runner GROUPS
	pythonrunner
	pylogger
	widgets
)

mo2_add_filter(NAME src/utils GROUPS
	error
	gilock
	sipapiaccess
)

mo2_add_filter(NAME src/wrappers GROUPS
	gamefeatureswrappers
	proxypluginwrappers
	pythonwrapperutilities
	uibasewrappers
)
