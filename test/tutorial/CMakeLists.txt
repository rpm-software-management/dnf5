pkg_check_modules(CPPUNIT REQUIRED cppunit)


# use any sources found under the current directory
file(GLOB TEST_TUTORIAL_SOURCES *.cpp)

include_directories(${PROJECT_SOURCE_DIR}/libdnf)


add_executable(run_tests_tutorial ${TEST_TUTORIAL_SOURCES})
target_link_libraries(run_tests_tutorial stdc++ libdnf libdnf-cli cppunit)


add_test(NAME test_tutorial COMMAND run_tests_tutorial)
