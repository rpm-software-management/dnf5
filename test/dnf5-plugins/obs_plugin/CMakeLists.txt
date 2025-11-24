pkg_check_modules(CPPUNIT REQUIRED cppunit)
add_definitions(-DGETTEXT_DOMAIN=\"dnf5_cmd_obs\")

# use any sources found under the current directory
file(GLOB TEST_OBS_SOURCES *.cpp
    ${PROJECT_SOURCE_DIR}/dnf5-plugins/obs_plugin/obs_repo.cpp
    ${PROJECT_SOURCE_DIR}/dnf5-plugins/obs_plugin/obs_config.cpp
    ${PROJECT_SOURCE_DIR}/dnf5-plugins/obs_plugin/html.cpp
)

include_directories(${PROJECT_SOURCE_DIR}/dnf5-plugins/obs_plugin)
include_directories(${PROJECT_SOURCE_DIR}/libdnf5)

add_executable(run_tests_obs ${TEST_OBS_SOURCES})
target_link_libraries(run_tests_obs PRIVATE stdc++ libdnf5 libdnf5-cli test_shared)

pkg_check_modules(LIBXML2 REQUIRED libxml-2.0)
include_directories(${LIBXML2_INCLUDE_DIRS})
target_link_libraries(run_tests_obs PRIVATE ${LIBXML2_LIBRARIES})

add_compile_definitions(TEST_DATADIR="${CMAKE_CURRENT_SOURCE_DIR}/data")

add_test(NAME test_obs COMMAND run_tests_obs)
set_tests_properties(test_obs PROPERTIES RUN_SERIAL TRUE)
