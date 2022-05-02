cmake_minimum_required(VERSION 3.16)

# pytest
find_package(GTest REQUIRED)

set(PYLIB_DIR ${CMAKE_CURRENT_BINARY_DIR}/pylibs)

if (TARGET uibase)
	set(UIBASE_PATH $<TARGET_FILE:uibase>)
else()
	set(UIBASE_PATH "${MO2_INSTALL_PATH}/bin/uibase.dll")
endif()

add_custom_target(pytest
	${CMAKE_COMMAND} -E env
		PYTHONPATH="${PYLIB_DIR}\\;$<TARGET_FILE_DIR:mobase>"
		"UIBASE_PATH=${UIBASE_PATH}" "QT_ROOT=${QT_ROOT}"
		${CMAKE_CURRENT_BINARY_DIR}/pylibs/bin/pytest.exe ${CMAKE_CURRENT_SOURCE_DIR} -s
	WORKING_DIRECTORY ${MO2_INSTALL_PATH}

)
mo2_python_pip_install(pytest
	DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/pylibs
	PACKAGES pytest PyQt6==6.3.0)
add_dependencies(pytest mobase)


file(GLOB test_files CONFIGURE_DEPENDS "test_*.cpp")
foreach (test_file ${test_files})
	get_filename_component(target ${test_file} NAME_WLE)

	string(REPLACE "test_" "" pymodule ${target})
	pybind11_add_module(${target} THIN_LTO ${test_file})
	set_target_properties(${target}
		PROPERTIES
		OUTPUT_NAME ${pymodule}
		LIBRARY_OUTPUT_DIRECTORY "${PYLIB_DIR}/mobase_tests")

	if(DEFINED CMAKE_CONFIGURATION_TYPES)
		foreach(config ${CMAKE_CONFIGURATION_TYPES})
			string(TOUPPER ${config} config)
			set_target_properties(${target} PROPERTIES
				LIBRARY_OUTPUT_DIRECTORY_${config} "${PYLIB_DIR}/mobase_tests")
		endforeach()
	endif()

	mo2_add_dependencies(${target} PRIVATE uibase Qt::Core Qt::Widgets)
	# set_property(TARGET ${target} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded")
	target_link_libraries(${target} PRIVATE pybind11::qt GTest::gmock)

	target_include_directories(${target}
    	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../mocks)
	add_dependencies(pytest ${target})
endforeach()
