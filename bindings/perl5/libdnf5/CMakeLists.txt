message("Building bindings for perl5: libdnf5")


foreach(MODULE_NAME ${SWIG_LIBDNF5_MODULES})
    add_perl5_module(libdnf5 ${MODULE_NAME})
endforeach()
