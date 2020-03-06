if(NOT WITH_LIBDNF_CLI)
    return()
endif()


file(GLOB_RECURSE LIBDNF_CLI_HEADERS *.hpp)
install(FILES ${LIBDNF_CLI_HEADERS} DESTINATION include/libdnf-cli)
