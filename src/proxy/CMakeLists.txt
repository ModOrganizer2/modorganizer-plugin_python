cmake_minimum_required(VERSION 3.16)

find_package(mo2-uibase CONFIG REQUIRED)

set(PLUGIN_NAME "plugin_python")

add_library(proxy SHARED proxypython.cpp proxypython.h)
mo2_configure_plugin(proxy
    NO_SOURCES
    WARNINGS 4
    EXTERNAL_WARNINGS 4
    TRANSLATIONS OFF
    EXTRA_TRANSLATIONS
    ${CMAKE_CURRENT_SOURCE_DIR}/../runner
    ${CMAKE_CURRENT_SOURCE_DIR}/../mobase
    ${CMAKE_CURRENT_SOURCE_DIR}/../pybind11-qt)
mo2_default_source_group()
target_link_libraries(proxy PRIVATE runner mo2::uibase)
set_target_properties(proxy PROPERTIES OUTPUT_NAME ${PLUGIN_NAME})
mo2_install_plugin(proxy FOLDER)

set(PLUGIN_PYTHON_DIR bin/plugins/${PLUGIN_NAME})

# install runner
target_link_options(proxy PRIVATE "/DELAYLOAD:runner.dll")
install(FILES $<TARGET_FILE:runner> DESTINATION ${PLUGIN_PYTHON_DIR}/dlls)

# translations (custom location)
mo2_add_translations(proxy
    TS_FILE ${CMAKE_CURRENT_SOURCE_DIR}/../${PLUGIN_NAME}_en.ts
    SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../runner
    ${CMAKE_CURRENT_SOURCE_DIR}/../mobase
    ${CMAKE_CURRENT_SOURCE_DIR}/../pybind11-qt)

# install DLLs files needed
set(DLL_DIRS ${PLUGIN_PYTHON_DIR}/dlls)
file(GLOB dlls_to_install
	${Python_HOME}/dlls/libffi*.dll
	${Python_HOME}/dlls/sqlite*.dll
	${Python_HOME}/dlls/libssl*.dll
	${Python_HOME}/dlls/libcrypto*.dll
	${Python_HOME}/python${Python_VERSION_MAJOR}*.dll)
install(FILES ${dlls_to_install} DESTINATION ${DLL_DIRS})

# install Python .pyd files
set(PYLIB_DIR ${PLUGIN_PYTHON_DIR}/libs)
file(GLOB libs_to_install ${Python_DLL_DIR}/*.pyd)
install(FILES ${libs_to_install} DESTINATION ${PYLIB_DIR})

# generate + install standard library
set(pythoncore_zip "${CMAKE_CURRENT_BINARY_DIR}/pythoncore.zip")
add_custom_command(
    TARGET proxy POST_BUILD
    COMMAND ${Python_EXECUTABLE}
        "${CMAKE_CURRENT_SOURCE_DIR}\\build_pythoncore.py"
        ${pythoncore_zip}
    )
install(FILES ${pythoncore_zip} DESTINATION ${PYLIB_DIR})

# install mobase
install(TARGETS mobase DESTINATION ${PYLIB_DIR})

# install PyQt6
install(
    DIRECTORY ${CMAKE_BINARY_DIR}/pylibs/PyQt${MO2_QT_VERSION_MAJOR}
    DESTINATION ${PYLIB_DIR}
    PATTERN "*.pyd"
    PATTERN "*.pyi"
    PATTERN "__pycache__" EXCLUDE
    PATTERN "bindings" EXCLUDE
    PATTERN "lupdate" EXCLUDE
    PATTERN "Qt6" EXCLUDE
    PATTERN "uic" EXCLUDE
)
