file(GLOB_RECURSE LIBDNF5_HEADERS *.hpp *.h)

if (NOT WITH_MODULEMD)
    file(GLOB_RECURSE LIBDNF5_HEADERS_MODULES module/*.hpp)

    # module_sack_weak.hpp is needed for base.hpp API
    # its not actually used and it doesn't include any other libdnf5 header
    list(FILTER LIBDNF5_HEADERS_MODULES EXCLUDE REGEX .*module_sack_weak.hpp)

    list(REMOVE_ITEM LIBDNF5_HEADERS ${LIBDNF5_HEADERS_MODULES})
endif()

# preserve relative paths of the header files
foreach(abspath ${LIBDNF5_HEADERS})
    # relative path to the header file
    file(RELATIVE_PATH relpath ${CMAKE_CURRENT_SOURCE_DIR} ${abspath})
    # dirname of the relative path
    get_filename_component(relpath_dir ${relpath} DIRECTORY)
    install(FILES ${abspath} DESTINATION include/libdnf5/${relpath_dir})
endforeach()
