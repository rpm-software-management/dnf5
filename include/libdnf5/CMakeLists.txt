file(GLOB_RECURSE LIBDNF5_HEADERS *.hpp *.h)

# preserve relative paths of the header files
foreach(abspath ${LIBDNF5_HEADERS})
    # relative path to the header file
    file(RELATIVE_PATH relpath ${CMAKE_CURRENT_SOURCE_DIR} ${abspath})
    # dirname of the relative path
    get_filename_component(relpath_dir ${relpath} DIRECTORY)
    install(FILES ${abspath} DESTINATION include/libdnf5/${relpath_dir})
endforeach()
