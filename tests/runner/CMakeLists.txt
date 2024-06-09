cmake_minimum_required(VERSION 3.22)

# setting-up the tests for the runner is a bit complex because we need a tons of
# things

# first we configure the tests as with other tests
add_executable(runner-tests EXCLUDE_FROM_ALL)
mo2_configure_tests(runner-tests WARNINGS OFF)

set_target_properties(runner-tests PROPERTIES FOLDER tests/runner)

# add mocks
target_include_directories(runner-tests
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../mocks)

# link to runner
target_link_libraries(runner-tests PUBLIC runner)

set(PYLIB_DIR ${CMAKE_CURRENT_BINARY_DIR}/pylibs)
mo2_python_pip_install(runner-tests
	DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/pylibs
  PACKAGES
    pytest
		PyQt${QT_MAJOR_VERSION}==${QT_VERSION}
		PyQt${QT_MAJOR_VERSION}-Qt${QT_MAJOR_VERSION}==${QT_VERSION})

add_dependencies(runner-tests mobase)

# we set multiple properties, including:
# - updated PATH for DLLs (Qt, etc.), Python and runner
# - PYTHONPATH

set(pythoncore "${PYTHON_ROOT}/PCbuild/amd64/pythoncore")
set(pythoncorezip "${pythoncore}/python${Python_VERSION_SHORT}.zip")
set(PYTHONPATH "${PYLIB_DIR}\\;$<TARGET_FILE_DIR:mobase>\\;${pythoncore}\\;${pythoncorezip}")

set(extra_paths "${MO2_INSTALL_PATH}/bin/dlls")
string(APPEND extra_paths "\\;${PYTHON_ROOT}/PCbuild/amd64")
string(APPEND extra_paths "\\;$<TARGET_FILE_DIR:runner>")
set_tests_properties(${runner-tests_gtests}
    PROPERTIES
    ENVIRONMENT "PLUGIN_DIR=${CMAKE_CURRENT_SOURCE_DIR}/plugins"
    ENVIRONMENT_MODIFICATION
    "PATH=path_list_prepend:${extra_paths};PYTHONPATH=set:${PYTHONPATH}"
)
