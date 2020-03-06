message("Building bindings for python3: libdnf")


file(REMOVE ${CMAKE_CURRENT_BINARY_DIR}/__init__.py)
foreach(MODULE_NAME ${SWIG_LIBDNF_MODULES})
    add_python3_module(libdnf ${MODULE_NAME})
    file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/__init__.py "from . import ${MODULE_NAME}\n")
endforeach()


install(FILES __init__.py DESTINATION ${Python3_SITEARCH}/libdnf)
