cmake_minimum_required(VERSION 3.22)

# setting-up the tests for the runner is a bit complex because we need a tons of
# things

# first we configure the tests as with other tests
add_executable(pythonrunner-tests EXCLUDE_FROM_ALL)
mo2_configure_tests(pythonrunner-tests WARNINGS OFF)

# add mocks
target_include_directories(pythonrunner-tests
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../mocks)

# link to pythonrunner - we set PYTHONRUNNER_LIBRARY to not export the symbol but
# loads it directly, we are going to add the DLL path below
target_compile_definitions(pythonrunner-tests PUBLIC PYTHONRUNNER_LIBRARY)
target_link_libraries(pythonrunner-tests PUBLIC pythonrunner)

set(PYLIB_DIR ${CMAKE_CURRENT_BINARY_DIR}/pylibs)
mo2_python_pip_install(pythonrunner-tests
	DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/pylibs
	PACKAGES pytest PyQt6==6.3.0)

add_dependencies(pythonrunner-tests mobase)

# we set multiple properties, including:
# - updated PATH for DLLs (Qt, etc.), Python and pythonrunner
# - PYTHONPATH

set(pythoncore "${PYTHON_ROOT}/PCbuild/amd64/pythoncore")
file(GLOB pythoncorezip "${pythoncore}/python*.zip")
set(PYTHONPATH "${PYLIB_DIR}\\;$<TARGET_FILE_DIR:mobase>\\;${pythoncore}\\;${pythoncorezip}")

set(extra_paths "${MO2_INSTALL_PATH}/bin/dlls")
string(APPEND extra_paths "\\;${PYTHON_ROOT}/PCbuild/amd64")
string(APPEND extra_paths "\\;$<TARGET_FILE_DIR:pythonrunner>")
set_tests_properties(${pythonrunner-tests_gtests}
    PROPERTIES
    WORKING_DIRECTORY "${MO2_INSTALL_PATH}/bin"
    ENVIRONMENT "PLUGIN_DIR=${CMAKE_CURRENT_SOURCE_DIR}/plugins"
    ENVIRONMENT_MODIFICATION
    "PATH=path_list_prepend:${extra_paths};PYTHONPATH=set:${PYTHONPATH}")
