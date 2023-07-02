cmake_minimum_required(VERSION 3.16)

# pytest
find_package(GTest REQUIRED)

set(PYLIB_DIR ${CMAKE_CURRENT_BINARY_DIR}/pylibs)

if (TARGET uibase)
	set(UIBASE_PATH $<TARGET_FILE_DIR:uibase>)
else()
	set(UIBASE_PATH "${MO2_INSTALL_PATH}/bin")
endif()

add_custom_target(python-tests)

add_test(NAME pytest
	COMMAND ${CMAKE_CURRENT_BINARY_DIR}/pylibs/bin/pytest.exe ${CMAKE_CURRENT_SOURCE_DIR} -s
)

set(extra_paths "${UIBASE_PATH}\\;${MO2_INSTALL_PATH}/bin/dlls")
set_tests_properties(pytest
	PROPERTIES
	DEPENDS python-tests
	WORKING_DIRECTORY ${MO2_INSTALL_PATH}/bin
	ENVIRONMENT_MODIFICATION
	"PYTHONPATH=set:${PYLIB_DIR}\\;$<TARGET_FILE_DIR:mobase>;\
UIBASE_PATH=set:${UIBASE_PATH};\
QT_ROOT=set:${QT_ROOT}"
)

mo2_python_pip_install(python-tests
	DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/pylibs
	PACKAGES pytest
		PyQt${QT_MAJOR_VERSION}==${QT_VERSION}
		PyQt${QT_MAJOR_VERSION}-Qt${QT_MAJOR_VERSION}==${QT_VERSION})
add_dependencies(python-tests mobase)
set_target_properties(python-tests PROPERTIES FOLDER tests/python)

file(GLOB test_files CONFIGURE_DEPENDS "test_*.cpp")
foreach (test_file ${test_files})
	get_filename_component(target ${test_file} NAME_WLE)

	string(REPLACE "test_" "" pymodule ${target})
	pybind11_add_module(${target} EXCLUDE_FROM_ALL THIN_LTO ${test_file})
	set_target_properties(${target}
		PROPERTIES
		OUTPUT_NAME ${pymodule}
		FOLDER tests/python
		LIBRARY_OUTPUT_DIRECTORY "${PYLIB_DIR}/mobase_tests")

	if(DEFINED CMAKE_CONFIGURATION_TYPES)
		foreach(config ${CMAKE_CONFIGURATION_TYPES})
			string(TOUPPER ${config} config)
			set_target_properties(${target} PROPERTIES
				LIBRARY_OUTPUT_DIRECTORY_${config} "${PYLIB_DIR}/mobase_tests")
		endforeach()
	endif()

	mo2_add_dependencies(${target} PRIVATE uibase Qt::Core Qt::Widgets)
	target_link_libraries(${target} PRIVATE pybind11::qt pybind11::utils GTest::gmock)

	target_include_directories(${target}
    	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../mocks)
	add_dependencies(python-tests ${target})
endforeach()
