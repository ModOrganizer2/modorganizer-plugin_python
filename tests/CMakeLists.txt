cmake_minimum_required(VERSION 3.16)

set(PYLIB_DIR ${CMAKE_CURRENT_BINARY_DIR}/pylibs)

add_custom_target(pytest
	${CMAKE_COMMAND} -E env PYTHONPATH=${PYLIB_DIR}
		${CMAKE_CURRENT_BINARY_DIR}/pylibs/bin/pytest.exe ${CMAKE_CURRENT_SOURCE_DIR}/../tests -s
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
mo2_python_pip_install(pytest
	DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/pylibs
	PACKAGES pytest PyQt6==6.3.0)

file(GLOB test_files "test_*.cpp")
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
	target_link_libraries(${target} PRIVATE pybind11::qt)
	add_dependencies(pytest ${target})
endforeach()
