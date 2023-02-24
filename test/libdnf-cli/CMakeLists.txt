if(NOT WITH_LIBDNF5_CLI)
    return()
endif()


pkg_check_modules(CPPUNIT REQUIRED cppunit)


# use any sources found under the current directory
file(GLOB_RECURSE TEST_LIBDNF5_CLI_SOURCES *.cpp)

include_directories(.)
include_directories(${PROJECT_SOURCE_DIR}/libdnf-cli)
include_directories(${PROJECT_SOURCE_DIR}/libdnf)


add_executable(run_tests_cli ${TEST_LIBDNF5_CLI_SOURCES})
target_link_directories(run_tests_cli PUBLIC ${CMAKE_BINARY_DIR}/libdnf)
target_link_libraries(run_tests_cli stdc++ libdnf libdnf-cli cppunit test_shared)


add_test(NAME test_libdnf_cli COMMAND run_tests_cli)
