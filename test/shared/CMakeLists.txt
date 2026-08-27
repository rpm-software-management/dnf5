pkg_check_modules(CPPUNIT REQUIRED cppunit)

# configure test config header
configure_file(test_config.h.in test_config.h @ONLY)

# use any sources found under the current directory
file(GLOB_RECURSE TEST_SHARED_SOURCES *.cpp)

include_directories(.)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${PROJECT_SOURCE_DIR}/libdnf5)

add_library(test_shared OBJECT ${TEST_SHARED_SOURCES})

target_link_directories(test_shared PUBLIC ${CMAKE_BINARY_DIR}/libdnf5)
target_link_libraries(test_shared PUBLIC cppunit)
