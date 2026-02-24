pkg_check_modules(CPPUNIT REQUIRED cppunit)


# use any sources found under the current directory
file(GLOB TEST_TUTORIAL_SOURCES *.cpp)

include_directories(${PROJECT_SOURCE_DIR}/libdnf5)


add_executable(run_tests_tutorial ${TEST_TUTORIAL_SOURCES})
target_link_libraries(run_tests_tutorial PRIVATE stdc++ libdnf5 libdnf5-cli cppunit)


add_test(NAME test_tutorial COMMAND run_tests_tutorial)
set_tests_properties(test_tutorial PROPERTIES
    RUN_SERIAL TRUE
    ENVIRONMENT "LC_ALL=C"
)
