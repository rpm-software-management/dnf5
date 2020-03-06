if(NOT WITH_LIBDNF_CLI)
    return()
endif()


message("Building bindings for python3: libdnf_cli")


file(REMOVE ${CMAKE_CURRENT_BINARY_DIR}/__init__.py)
foreach(MODULE_NAME ${SWIG_LIBDNF_CLI_MODULES})
    add_python3_module(libdnf_cli ${MODULE_NAME})
    file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/__init__.py "from . import ${MODULE_NAME}\n")
endforeach()


install(FILES __init__.py DESTINATION ${Python3_SITEARCH}/libdnf_cli)
