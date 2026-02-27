if(NOT WITH_DNF5DAEMON_SERVER OR NOT WITH_DNF5DAEMON_TESTS)
    return()
endif()

find_package(Python3 REQUIRED)

add_test(
    NAME test_dnf5daemon_server
    COMMAND ${Python3_EXECUTABLE} -m unittest
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)

set_property(TEST test_dnf5daemon_server PROPERTY RUN_SERIAL TRUE)

set_property(TEST test_dnf5daemon_server PROPERTY ENVIRONMENT
    "LC_ALL=C"
    "${ASAN_OPTIONS}"
    "PROJECT_BINARY_DIR=${PROJECT_BINARY_DIR}"
)
