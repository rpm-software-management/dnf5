if(NOT WITH_DNFDAEMON_SERVER)
    return()
endif()


pkg_check_modules(CPPUNIT REQUIRED cppunit)


# use any sources found under the current directory
file(GLOB_RECURSE TEST_DNFDAEMON_SERVER_SOURCES *.cpp)


set(TEST_DNFDAEMON_SERVER_SOURCES
    ${TEST_DNFDAEMON_SERVER_SOURCES}
    # share the test runner code with libdnf
    ${CMAKE_CURRENT_SOURCE_DIR}/../libdnf/run_tests.cpp
)


add_executable(run_tests_dnfdaemon_server ${TEST_DNFDAEMON_SERVER_SOURCES})
target_link_directories(run_tests_dnfdaemon_server PUBLIC ${CMAKE_BINARY_DIR}/libdnf ${CMAKE_BINARY_DIR}/libdnf-cli)
target_link_libraries(run_tests_dnfdaemon_server stdc++ libdnf libdnf-cli cppunit)


add_test(NAME test_dnfdaemon_server COMMAND ${CMAKE_CURRENT_BINARY_DIR}/run_tests_dnfdaemon_server DEPENDS run_tests_dnfdaemon_server COMMENT "Running dnfdaemon-server tests...")
