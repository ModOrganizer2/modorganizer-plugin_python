cmake_minimum_required(VERSION 3.16)

set(PLUGIN_NAME "plugin_python")

add_library(proxy SHARED)
mo2_configure_plugin(proxy
    WARNINGS 4
    EXTERNAL_WARNINGS 4
    TRANSLATIONS OFF
    EXTRA_TRANSLATIONS
    ${CMAKE_CURRENT_SOURCE_DIR}/../runner
    ${CMAKE_CURRENT_SOURCE_DIR}/../mobase
    ${CMAKE_CURRENT_SOURCE_DIR}/../pybind11-qt)
target_link_libraries(proxy PRIVATE runner)
set_target_properties(proxy PROPERTIES OUTPUT_NAME ${PLUGIN_NAME})
mo2_install_target(proxy FOLDER)

set(PLUGIN_PYTHON_DIR ${MO2_INSTALL_PATH}/bin/plugins/${PLUGIN_NAME})

# install runner
target_link_options(proxy PRIVATE "/DELAYLOAD:runner.dll")
mo2_install_target(runner INSTALLDIR ${PLUGIN_PYTHON_DIR}/dlls)

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
	${PYTHON_BUILD_PATH}/libffi*.dll
	${PYTHON_BUILD_PATH}/python${Python_VERSION_SHORT}.dll)
install(FILES ${dlls_to_install} DESTINATION ${DLL_DIRS})

# install Python files
set(PYLIB_DIR ${PLUGIN_PYTHON_DIR}/libs)
file(GLOB libs_to_install ${PYTHON_BUILD_PATH}/pythoncore/*.pyd)
install(FILES ${libs_to_install} DESTINATION ${PYLIB_DIR})
install(FILES ${PYTHON_BUILD_PATH}/pythoncore/python${Python_VERSION_SHORT}.zip
    DESTINATION ${PYLIB_DIR} RENAME pythoncore.zip)

# install mobase
install(TARGETS mobase DESTINATION ${PYLIB_DIR})

# install PyQt6
#file(GLOB PYQT_DIR ${MO2_BUILD_PATH}/PyQt${QT_MAJOR_VERSION}*)
set(PYQT_LIB_DIR ${PYTHON_ROOT}/Lib/site-packages/PyQt${QT_MAJOR_VERSION})
set(PYQT_TARGET_DIR ${PYLIB_DIR}/PyQt${QT_MAJOR_VERSION})
file(GLOB pyqt_files ${PYQT_LIB_DIR}/*.py ${PYQT_LIB_DIR}/*.pyd ${PYQT_LIB_DIR}/*.pyi)
install(FILES ${pyqt_files} DESTINATION ${PYQT_TARGET_DIR})
