add_subdirectory(libdnf5-plugin)
add_subdirectory(dnf5-plugin)

# Copy libdnf5-plugin configuration file to build directory
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/libdnf5-plugin/template.conf
    ${CMAKE_CURRENT_BINARY_DIR}/libdnf5-plugin/template.conf
    COPYONLY
)

pkg_check_modules(CPPUNIT REQUIRED cppunit)

# use any sources found under the current directory
file(GLOB TEST_TUTORIAL_TEMPLATE_SOURCES *.cpp)

include_directories(${PROJECT_SOURCE_DIR}/libdnf5)
include_directories(${PROJECT_SOURCE_DIR}/dnf5)

add_executable(run_tests_tutorial_templates ${TEST_TUTORIAL_TEMPLATE_SOURCES})
target_link_libraries(run_tests_tutorial_templates PRIVATE stdc++ libdnf5 libdnf5-cli cppunit)
target_compile_definitions(run_tests_tutorial_templates PRIVATE
    DNF5_BINARY_PATH="${PROJECT_BINARY_DIR}/dnf5/dnf5"
    DNF5_PLUGIN_DIR="${PROJECT_BINARY_DIR}/test/tutorial-templates/dnf5-plugin"
    LIBDNF5_PLUGIN_DIR="${PROJECT_BINARY_DIR}/test/tutorial-templates/libdnf5-plugin")

# Ensure plugins and dnf5 are built before the test executable
add_dependencies(run_tests_tutorial_templates template_plugin template_dnf5_cmd_plugin dnf5)

add_test(NAME test_tutorial_templates COMMAND run_tests_tutorial_templates)
set_tests_properties(test_tutorial_templates PROPERTIES
    RUN_SERIAL TRUE
    ENVIRONMENT "LC_ALL=C"
)
