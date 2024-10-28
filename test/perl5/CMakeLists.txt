if(NOT WITH_PERL5)
    return()
endif()


find_package(Perl REQUIRED)
foreach(MODULE "strict" "Test::More" "Test::Exception" "warnings" "FindBin")
    message(STATUS "Checking for ${MODULE} Perl module")
    execute_process(
        COMMAND "${PERL_EXECUTABLE}" -e "require ${MODULE}"
        RESULT_VARIABLE EXIT_CODE
    )
    if(EXIT_CODE GREATER 0)
        message(FATAL_ERROR "${MODULE} Perl module needed for tests was not found")
    endif()
endforeach()


macro(add_perl_test)
    set(target "test_perl5_${ARGV0}_${ARGV1}")
    add_test(
        NAME "${target}"
        COMMAND "${PERL_EXECUTABLE}" "-I${CMAKE_BINARY_DIR}/bindings/perl5" "${ARGV1}"
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    )

    set_property(TEST "${target}" PROPERTY RUN_SERIAL TRUE)

    set_property(TEST ${target} PROPERTY ENVIRONMENT
        "PROJECT_BINARY_DIR=${PROJECT_BINARY_DIR}"
        "PROJECT_SOURCE_DIR=${PROJECT_SOURCE_DIR}"
        "${ASAN_OPTIONS}"
    )
endmacro()


add_subdirectory(libdnf5)
add_subdirectory(libdnf5_cli)
