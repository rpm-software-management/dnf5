pkg_check_modules(CPPUNIT REQUIRED cppunit)


# use any sources found under the current directory
file(GLOB_RECURSE TEST_SHARED_SOURCES *.cpp)

include_directories(.)
include_directories(${PROJECT_SOURCE_DIR}/libdnf5)

add_library(test_shared OBJECT ${TEST_SHARED_SOURCES})

target_link_directories(test_shared PUBLIC ${CMAKE_BINARY_DIR}/libdnf5)
target_link_libraries(test_shared PUBLIC stdc++ libdnf5_static cppunit)
