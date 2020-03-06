if(NOT WITH_PERL5)
    return()
endif()


find_package(Perl REQUIRED)
foreach(MODULE "strict" "Test::More" "warnings")
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
    add_test(
        NAME "test_perl5_${ARGV0}_${ARGV1}"
        COMMAND "${PERL_EXECUTABLE}" "-I${CMAKE_BINARY_DIR}/bindings/perl5" "${ARGV1}"
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    )
endmacro()


add_subdirectory(libdnf)
add_subdirectory(libdnf_cli)
