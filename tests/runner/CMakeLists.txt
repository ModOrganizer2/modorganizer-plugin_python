cmake_minimum_required(VERSION 3.16)

add_executable(pythonrunner-tests)
mo2_configure_tests(pythonrunner-tests
    WARNINGS OFF)
mo2_add_dependencies(pythonrunner-tests PUBLIC uibase)
target_include_directories(pythonrunner-tests
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../mocks)
