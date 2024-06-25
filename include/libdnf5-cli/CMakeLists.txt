if(NOT WITH_LIBDNF5_CLI)
    return()
endif()


file(GLOB_RECURSE LIBDNF5_CLI_HEADERS *.hpp)

if (NOT WITH_MODULEMD)
    file(GLOB_RECURSE LIBDNF5_CLI_HEADERS_MODULES
        output/adapters/module.hpp
        output/adapters/module_tmpl.hpp
        output/interfaces/module.hpp
        output/moduleinfo.hpp
        output/modulelist.hpp)

    list(REMOVE_ITEM LIBDNF5_CLI_HEADERS ${LIBDNF5_CLI_HEADERS_MODULES})
endif()


# preserve relative paths of the header files
foreach(abspath ${LIBDNF5_CLI_HEADERS})
    # relative path to the header file
    file(RELATIVE_PATH relpath ${CMAKE_CURRENT_SOURCE_DIR} ${abspath})
    # dirname of the relative path
    get_filename_component(relpath_dir ${relpath} DIRECTORY)
    install(FILES ${abspath} DESTINATION include/libdnf5-cli/${relpath_dir})
endforeach()
