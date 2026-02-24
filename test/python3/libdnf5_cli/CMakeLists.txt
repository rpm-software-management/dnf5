if(NOT WITH_LIBDNF5_CLI)
    return()
endif()


add_test(NAME test_python3_libdnf_cli COMMAND ${Python3_EXECUTABLE} -m unittest WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
set_property(TEST test_python3_libdnf_cli PROPERTY ENVIRONMENT
    "LC_ALL=C"
    "PYTHONPATH=${CMAKE_BINARY_DIR}/bindings/python3"
    "${ASAN_OPTIONS}"
)
