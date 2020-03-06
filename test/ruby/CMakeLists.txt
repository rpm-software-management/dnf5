if(NOT WITH_RUBY)
    return()
endif()


find_package(Ruby REQUIRED)


macro(add_ruby_test)
    add_test(
        NAME "test_ruby_${ARGV0}_${ARGV1}"
        COMMAND "${RUBY_EXECUTABLE}" "-I${CMAKE_BINARY_DIR}/bindings/ruby" "${ARGV1}"
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    )
endmacro()


add_subdirectory(libdnf)
add_subdirectory(libdnf_cli)
