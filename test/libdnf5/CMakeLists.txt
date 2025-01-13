pkg_check_modules(CPPUNIT REQUIRED cppunit)


# use any sources found under the current directory
file(GLOB_RECURSE TEST_LIBDNF5_SOURCES *.cpp)

include_directories(.)
include_directories(${PROJECT_SOURCE_DIR}/libdnf5)

add_executable(run_tests ${TEST_LIBDNF5_SOURCES})

# Disable warnings for operations used intentionally to test how the case is handled
target_compile_options(run_tests PRIVATE "-Wno-unused-comparison")
target_compile_options(run_tests PRIVATE "-Wno-unused-value")
target_compile_options(run_tests PRIVATE "-Wno-self-assign-overloaded")
target_compile_options(run_tests PRIVATE "-Wno-self-move")

target_link_directories(run_tests PRIVATE ${CMAKE_BINARY_DIR}/libdnf5)
target_link_libraries(run_tests PRIVATE stdc++ libdnf5_iface test_shared)

pkg_check_modules(JSONC REQUIRED json-c)
include_directories(${JSONC_INCLUDE_DIRS})
target_link_libraries(run_tests PRIVATE ${JSONC_LIBRARIES})

pkg_check_modules(LIBSOLV REQUIRED libsolv>=0.7.21)
target_link_libraries(run_tests PRIVATE ${LIBSOLV_LIBRARIES})


if(WITH_PERFORMANCE_TESTS)
    target_compile_options(run_tests PRIVATE -DWITH_PERFORMANCE_TESTS)
endif()


add_test(NAME test_libdnf COMMAND run_tests)
set_tests_properties(test_libdnf PROPERTIES RUN_SERIAL TRUE)
