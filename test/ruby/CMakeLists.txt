if(NOT WITH_RUBY)
    return()
endif()


find_package(Ruby REQUIRED)


macro(add_ruby_test)
    set(target "test_ruby_${ARGV0}_${ARGV1}")
    add_test(
        NAME "${target}"
        COMMAND "${Ruby_EXECUTABLE}" "-I${CMAKE_BINARY_DIR}/bindings/ruby" "-I${CMAKE_CURRENT_SOURCE_DIR}" "${ARGV1}"
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    )

    set_property(TEST "${target}" PROPERTY RUN_SERIAL TRUE)

    # TODO the ODR violation seems like a problem with the Ruby bindings that
    # should be fixed
    set(ASAN_OPTIONS "${ASAN_OPTIONS},detect_odr_violation=0")
    set_property(TEST ${target} PROPERTY ENVIRONMENT
        "LC_ALL=C"
        "PROJECT_BINARY_DIR=${PROJECT_BINARY_DIR}"
        "PROJECT_SOURCE_DIR=${PROJECT_SOURCE_DIR}"
        "${ASAN_OPTIONS}"
    )
endmacro()


add_subdirectory(libdnf5)
add_subdirectory(libdnf5_cli)
