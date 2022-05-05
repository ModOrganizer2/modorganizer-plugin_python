cmake_minimum_required(VERSION 3.16)

add_library(pythonrunner SHARED)
mo2_configure_library(pythonrunner
	SOURCE_TREE
	WARNINGS 4
	EXTERNAL_WARNINGS 4
	AUTOMOC ON
	TRANSLATIONS OFF
	PUBLIC_DEPENDS uibase Qt::Core
)
target_link_libraries(pythonrunner PRIVATE pybind11::embed pybind11::qt)
target_include_directories(pythonrunner
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
	PRIVATE ${PYTHON_ROOT}/Include)

# this is kind of broken but it only works with this...
target_compile_definitions(pythonrunner
	PRIVATE QT_NO_KEYWORDS PYTHONRUNNER_LIBRARY)
mo2_install_target(pythonrunner INSTALLDIR bin/plugins/plugin_python)

# install DLLs files needed
set(DLL_DIRS ${MO2_INSTALL_PATH}/bin/plugins/plugin_python/dlls)
file(GLOB dlls_to_install
	${PYTHON_BUILD_PATH}/libffi*.dll
	${PYTHON_BUILD_PATH}/python${PYVERSION}.dll)
install(FILES ${dlls_to_install} DESTINATION ${DLL_DIRS})

# install Python files
set(PYLIB_DIR ${MO2_INSTALL_PATH}/bin/plugins/plugin_python/libs)
file(GLOB libs_to_install ${PYTHON_BUILD_PATH}/pythoncore/*.pyd)
install(FILES ${libs_to_install} DESTINATION ${PYLIB_DIR})
install(FILES ${PYTHON_BUILD_PATH}/pythoncore/python${PYVERSION}.zip
	DESTINATION ${PYLIB_DIR} RENAME pythoncore.zip)

# install PyQt6
file(GLOB PYQT_DIR ${MO2_BUILD_PATH}/PyQt${QT_MAJOR_VERSION}*)
set(PYQT_LIB_DIR ${PYQT_DIR}/Lib/site-packages/PyQt${QT_MAJOR_VERSION})
set(PYQT_TARGET_DIR ${PYLIB_DIR}/PyQt${QT_MAJOR_VERSION})
file(GLOB pyqt_files ${PYQT_LIB_DIR}/*.py ${PYQT_LIB_DIR}/*.pyd ${PYQT_LIB_DIR}/*.pyi)
install(FILES ${pyqt_files} DESTINATION ${PYQT_TARGET_DIR})
