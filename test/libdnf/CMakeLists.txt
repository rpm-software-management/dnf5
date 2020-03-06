pkg_check_modules(CPPUNIT REQUIRED cppunit)


# use any sources found under the current directory
file(GLOB_RECURSE TEST_LIBDNF_SOURCES *.cpp)


add_executable(run_tests ${TEST_LIBDNF_SOURCES})
target_link_directories(run_tests PUBLIC ${CMAKE_BINARY_DIR}/libdnf)
target_link_libraries(run_tests stdc++ libdnf cppunit)


add_test(NAME test_libdnf COMMAND ${CMAKE_CURRENT_BINARY_DIR}/run_tests DEPENDS run_tests COMMENT "Running libdnf C++ tests...")
