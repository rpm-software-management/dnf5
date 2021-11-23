if(NOT WITH_LIBDNF_CLI)
    return()
endif()


pkg_check_modules(CPPUNIT REQUIRED cppunit)


# use any sources found under the current directory
file(GLOB_RECURSE TEST_LIBDNF_CLI_SOURCES *.cpp)

# share the test runner code with libdnf
list(APPEND TEST_LIBDNF_CLI_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/../libdnf/run_tests.cpp)

include_directories(.)
include_directories(${PROJECT_SOURCE_DIR}/libdnf-cli)


add_executable(run_tests_cli ${TEST_LIBDNF_CLI_SOURCES})
target_link_directories(run_tests_cli PUBLIC ${CMAKE_BINARY_DIR}/libdnf)
target_link_libraries(run_tests_cli stdc++ libdnf libdnf-cli cppunit)


add_test(NAME test_libdnf_cli COMMAND ${CMAKE_CURRENT_BINARY_DIR}/run_tests_cli DEPENDS run_tests_cli COMMENT "Running libdnf-cli C++ tests...")
