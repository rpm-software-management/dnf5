pkg_check_modules(CPPUNIT REQUIRED cppunit)
add_definitions(-DGETTEXT_DOMAIN=\"dnf5-plugin-automatic\")

# Use all test sources in the current directory and select compilation units
# of the plugin we want to test.
file(GLOB TEST_AUTOMATIC_SOURCES *.cpp
    ${PROJECT_SOURCE_DIR}/dnf5-plugins/automatic_plugin/email_message.cpp
)

include_directories(${PROJECT_SOURCE_DIR}/dnf5-plugins/automatic_plugin)
include_directories(${PROJECT_SOURCE_DIR}/libdnf5)

add_executable(run_tests_automatic ${TEST_AUTOMATIC_SOURCES})
target_link_libraries(run_tests_automatic PRIVATE stdc++ libdnf5 libdnf5-cli test_shared)

add_test(NAME test_automatic COMMAND run_tests_automatic)
set_tests_properties(test_automatic PROPERTIES
    RUN_SERIAL FALSE
    ENVIRONMENT "LC_ALL=C"
)
