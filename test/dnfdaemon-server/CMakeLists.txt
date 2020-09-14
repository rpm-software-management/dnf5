if(NOT WITH_DNFDAEMON_SERVER OR NOT WITH_DNFDAEMON_TESTS)
    return()
endif()

find_package(Python3 REQUIRED)

add_test(
    NAME test_dnfdaemon_server
    COMMAND ${Python3_EXECUTABLE} -m unittest
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
