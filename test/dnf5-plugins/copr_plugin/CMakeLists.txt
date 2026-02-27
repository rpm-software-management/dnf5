pkg_check_modules(CPPUNIT REQUIRED cppunit)
add_definitions(-DGETTEXT_DOMAIN=\"dnf5_cmd_copr\")

# use any sources found under the current directory
file(GLOB TEST_COPR_SOURCES *.cpp
    ${PROJECT_SOURCE_DIR}/dnf5-plugins/copr_plugin/helpers.cpp
    ${PROJECT_SOURCE_DIR}/dnf5-plugins/copr_plugin/copr_repo.cpp
    ${PROJECT_SOURCE_DIR}/dnf5-plugins/copr_plugin/copr_config.cpp
    ${PROJECT_SOURCE_DIR}/dnf5-plugins/copr_plugin/os_release.cpp
    ${PROJECT_SOURCE_DIR}/dnf5-plugins/copr_plugin/json.cpp
)

include_directories(${PROJECT_SOURCE_DIR}/dnf5-plugins/copr_plugin)
include_directories(${PROJECT_SOURCE_DIR}/libdnf5)

include_directories(${JSONC_INCLUDE_DIRS})

add_executable(run_tests_copr ${TEST_COPR_SOURCES})
target_link_libraries(run_tests_copr PRIVATE stdc++ libdnf5 libdnf5-cli test_shared)

pkg_check_modules(JSONC REQUIRED json-c)
include_directories(${JSONC_INCLUDE_DIRS})
target_link_libraries(run_tests_copr PRIVATE ${JSONC_LIBRARIES})

add_compile_definitions(TEST_DATADIR="${CMAKE_CURRENT_SOURCE_DIR}/data")

add_test(NAME test_copr COMMAND run_tests_copr)
set_tests_properties(test_copr PROPERTIES
    RUN_SERIAL TRUE
    ENVIRONMENT "LC_ALL=C"
)
