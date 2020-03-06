# use any tests found under the current directory
file(GLOB_RECURSE PERL5_TESTS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.t)


foreach(FILE ${PERL5_TESTS})
    add_perl_test(libdnf "${FILE}")
endforeach()
