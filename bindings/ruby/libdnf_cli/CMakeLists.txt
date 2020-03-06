if(NOT WITH_LIBDNF_CLI)
    return()
endif()


message("Building bindings for ruby: libdnf_cli")


foreach(MODULE_NAME ${SWIG_LIBDNF_CLI_MODULES})
    add_ruby_module(libdnf_cli ${MODULE_NAME})
endforeach()
