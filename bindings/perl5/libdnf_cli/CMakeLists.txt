if(NOT WITH_LIBDNF_CLI)
    return()
endif()


message("Building bindings for perl5: libdnf_cli")


foreach(MODULE_NAME ${SWIG_LIBDNF_CLI_MODULES})
    add_perl5_module(libdnf_cli ${MODULE_NAME})
endforeach()
