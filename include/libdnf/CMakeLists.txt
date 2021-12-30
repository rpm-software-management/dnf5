file(GLOB_RECURSE LIBDNF_HEADERS *.hpp)

# preserve relative paths of the header files
foreach(abspath ${LIBDNF_HEADERS})
    # relative path to the header file
    file(RELATIVE_PATH relpath ${CMAKE_CURRENT_SOURCE_DIR} ${abspath})
    # dirname of the relative path
    get_filename_component(relpath_dir ${relpath} DIRECTORY)
    install(FILES ${abspath} DESTINATION include/libdnf/${relpath_dir})
endforeach()
