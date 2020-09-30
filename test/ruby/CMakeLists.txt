if(NOT WITH_RUBY)
    return()
endif()


find_package(Ruby REQUIRED)


macro(add_ruby_test)
    set(target "test_ruby_${ARGV0}_${ARGV1}")
    add_test(
        NAME "${target}"
        COMMAND "${RUBY_EXECUTABLE}" "-I${CMAKE_BINARY_DIR}/bindings/ruby" "${ARGV1}"
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    )
    set_property(TEST ${target} PROPERTY ENVIRONMENT
        "${ASAN_LD_PRELOAD}"
        "${ASAN_OPTIONS}"
        "${LSAN_OPTIONS}"
    )
endmacro()


add_subdirectory(libdnf)
add_subdirectory(libdnf_cli)
