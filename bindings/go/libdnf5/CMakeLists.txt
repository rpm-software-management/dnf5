message("Building bindings for go: libdnf5")


foreach(MODULE_NAME ${SWIG_LIBDNF5_MODULES})
    add_go_module(libdnf5 ${MODULE_NAME})
endforeach()
