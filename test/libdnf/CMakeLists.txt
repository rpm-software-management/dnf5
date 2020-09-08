pkg_check_modules(CPPUNIT REQUIRED cppunit)


# use any sources found under the current directory
file(GLOB_RECURSE TEST_LIBDNF_SOURCES *.cpp)


add_executable(run_tests ${TEST_LIBDNF_SOURCES})

# unused comparisons and values are frequently used in CPPUNIT_ASSERT_THROW
target_compile_options(run_tests PRIVATE "-Wno-unused-comparison")
target_compile_options(run_tests PRIVATE "-Wno-unused-value")

target_link_directories(run_tests PUBLIC ${CMAKE_BINARY_DIR}/libdnf)
target_link_libraries(run_tests stdc++ libdnf cppunit)

pkg_check_modules(JSONC REQUIRED json-c)
include_directories(${JSONC_INCLUDE_DIRS})
target_link_libraries(run_tests ${JSONC_LIBRARIES})


if(WITH_PERFORMANCE_TESTS)
    target_compile_options(run_tests PRIVATE -DWITH_PERFORMANCE_TESTS)
endif()


add_test(NAME test_libdnf COMMAND ${CMAKE_CURRENT_BINARY_DIR}/run_tests DEPENDS run_tests COMMENT "Running libdnf C++ tests...")
