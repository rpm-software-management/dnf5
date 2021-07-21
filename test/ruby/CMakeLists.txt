if(NOT WITH_RUBY)
    return()
endif()


find_package(Ruby REQUIRED)


macro(add_ruby_test)
    set(target "test_ruby_${ARGV0}_${ARGV1}")
    add_test(
        NAME "${target}"
        # explicitly add path to support.rb to the includes to ensure that `require 'support'` always works as expected
        COMMAND "${RUBY_EXECUTABLE}" "-I${CMAKE_BINARY_DIR}/bindings/ruby" "-I${CMAKE_SOURCE_DIR}/test/ruby/libdnf/support" "${ARGV1}"
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    )

    # TODO the ODR violation seems like a problem with the Ruby bindings that
    # should be fixed
    set(ASAN_OPTIONS "${ASAN_OPTIONS},detect_odr_violation=0")
    set_property(TEST ${target} PROPERTY ENVIRONMENT
        "PROJECT_BINARY_DIR=${PROJECT_BINARY_DIR}"
        "PROJECT_SOURCE_DIR=${PROJECT_SOURCE_DIR}"
        "${ASAN_OPTIONS}"
    )
endmacro()


add_subdirectory(libdnf)
add_subdirectory(libdnf_cli)
