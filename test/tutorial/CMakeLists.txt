pkg_check_modules(CPPUNIT REQUIRED cppunit)


# use any sources found under the current directory
file(GLOB TEST_TUTORIAL_SOURCES *.cpp)

# share the test runner code with libdnf
list(APPEND TEST_TUTORIAL_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/../libdnf/run_tests.cpp)


add_executable(run_tests_tutorial ${TEST_TUTORIAL_SOURCES})
target_link_libraries(run_tests_tutorial stdc++ libdnf libdnf-cli cppunit)


add_test(NAME test_tutorial COMMAND ${CMAKE_CURRENT_BINARY_DIR}/run_tests_tutorial DEPENDS run_tests_cli COMMENT "Testing tutorial code snippets...")
