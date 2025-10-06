pkg_check_modules(CPPUNIT REQUIRED cppunit)
add_definitions(-DGETTEXT_DOMAIN=\"dnf5_cmd_needs_restarting\")

# use any sources found under the current directory
file(GLOB TEST_NEEDS_RESTARTING_SOURCES *.cpp
    ${PROJECT_SOURCE_DIR}/dnf5-plugins/needs_restarting_plugin/needs_restarting.cpp
    ${PROJECT_SOURCE_DIR}/dnf5/context.cpp
    ${PROJECT_SOURCE_DIR}/dnf5/plugins.cpp
    ${PROJECT_SOURCE_DIR}/dnf5/download_callbacks.cpp
    ${PROJECT_SOURCE_DIR}/dnf5/library.cpp
    ${PROJECT_SOURCE_DIR}/dnf5/version.cpp
)

include_directories(${PROJECT_SOURCE_DIR}/dnf5-plugins/needs_restarting_plugin)
include_directories(${PROJECT_SOURCE_DIR}/libdnf5)
include_directories(${PROJECT_SOURCE_DIR}/dnf5)
include_directories(${PROJECT_SOURCE_DIR}/dnf5/include)

pkg_check_modules(SDBUS_CPP REQUIRED sdbus-c++)
include_directories(${SDBUS_CPP_INCLUDE_DIRS})

pkg_check_modules(RPM REQUIRED rpm>=4.17.0)

add_executable(run_tests_needs_restarting ${TEST_NEEDS_RESTARTING_SOURCES})
target_link_libraries(run_tests_needs_restarting PRIVATE common_obj stdc++ libdnf5 libdnf5-cli test_shared ${SDBUS_CPP_LIBRARIES} ${RPM_LIBRARIES})

add_test(NAME test_needs_restarting COMMAND run_tests_needs_restarting)
set_tests_properties(test_needs_restarting PROPERTIES RUN_SERIAL TRUE)
