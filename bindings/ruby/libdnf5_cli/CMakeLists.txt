if(NOT WITH_LIBDNF5_CLI)
    return()
endif()


message("Building bindings for ruby: libdnf5_cli")


foreach(MODULE_NAME ${SWIG_LIBDNF5_CLI_MODULES})
    add_ruby_module(libdnf5_cli ${MODULE_NAME})
endforeach()
