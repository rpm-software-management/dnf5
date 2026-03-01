if(NOT WITH_LIBDNF5_CLI)
    return()
endif()


pkg_check_modules(CPPUNIT REQUIRED cppunit)


# use any sources found under the current directory
file(GLOB_RECURSE TEST_LIBDNF5_CLI_SOURCES *.cpp)

include_directories(.)
include_directories(${PROJECT_SOURCE_DIR}/libdnf5-cli)
include_directories(${PROJECT_SOURCE_DIR}/libdnf5)


add_executable(run_tests_cli ${TEST_LIBDNF5_CLI_SOURCES})
target_link_directories(run_tests_cli PRIVATE ${CMAKE_BINARY_DIR}/libdnf5)
target_link_libraries(run_tests_cli PRIVATE stdc++ libdnf5 libdnf5-cli test_shared)


add_test(NAME test_libdnf_cli_C_UTF8 COMMAND run_tests_cli)
set_tests_properties(test_libdnf_cli_C_UTF8 PROPERTIES
    RUN_SERIAL TRUE
    ENVIRONMENT "LC_ALL=C.UTF-8"
)

add_test(NAME test_libdnf_cli_C COMMAND run_tests_cli)
set_tests_properties(test_libdnf_cli_C PROPERTIES
    RUN_SERIAL TRUE
    ENVIRONMENT "LC_ALL=C"
)
