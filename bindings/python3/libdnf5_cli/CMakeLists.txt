if(NOT WITH_LIBDNF_CLI)
    return()
endif()


message("Building bindings for python3: libdnf_cli")

set(INIT_FILE "${CMAKE_CURRENT_BINARY_DIR}/__init__.py")
file(REMOVE "${INIT_FILE}")
foreach(MODULE_NAME ${SWIG_LIBDNF_CLI_MODULES})
    add_python3_module(libdnf5_cli ${MODULE_NAME})
    file(APPEND "${INIT_FILE}" "from . import ${MODULE_NAME}\n")
endforeach()


install(FILES "${INIT_FILE}" DESTINATION ${Python3_SITEARCH}/libdnf5_cli)
