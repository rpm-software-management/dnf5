if(NOT WITH_LIBDNF_CLI)
    return()
endif()


message("Building bindings for go: libdnf_cli")


foreach(MODULE_NAME ${SWIG_LIBDNF_CLI_MODULES})
    add_go_module(libdnf_cli ${MODULE_NAME})
endforeach()
