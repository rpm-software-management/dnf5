message("Building bindings for ruby: libdnf")


foreach(MODULE_NAME ${SWIG_LIBDNF_MODULES})
    add_ruby_module(libdnf ${MODULE_NAME})
endforeach()
