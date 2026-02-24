add_test(NAME test_python3_libdnf COMMAND ${Python3_EXECUTABLE} -m unittest WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

set_property(TEST test_python3_libdnf PROPERTY RUN_SERIAL TRUE)

set_property(TEST test_python3_libdnf PROPERTY ENVIRONMENT
    "LC_ALL=C"
    "PROJECT_BINARY_DIR=${PROJECT_BINARY_DIR}"
    "PROJECT_SOURCE_DIR=${PROJECT_SOURCE_DIR}"
    "PYTHONPATH=${CMAKE_BINARY_DIR}/bindings/python3"
    "${ASAN_OPTIONS}"
)
