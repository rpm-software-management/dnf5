message("Building bindings for ruby: libdnf5")


foreach(MODULE_NAME ${SWIG_LIBDNF5_MODULES})
    add_ruby_module(libdnf5 ${MODULE_NAME})
endforeach()
