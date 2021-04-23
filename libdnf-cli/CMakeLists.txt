if(NOT WITH_LIBDNF_CLI)
    return()
endif()


# use any sources found under the current directory
file(GLOB_RECURSE LIBDNF_CLI_SOURCES *.cpp)


# gather all pkg-config requires and write them to a .pc file later
list(APPEND LIBDNF_CLI_PC_REQUIRES)
list(APPEND LIBDNF_CLI_PC_REQUIRES_PRIVATE)


# build libdnf-cli.so
add_library(libdnf-cli SHARED ${LIBDNF_CLI_SOURCES})
set(DNF_CLI_SO_VERSION 0)
set_target_properties(libdnf-cli PROPERTIES OUTPUT_NAME "dnf-cli")
set_target_properties(libdnf-cli PROPERTIES SOVERSION ${DNF_CLI_SO_VERSION})
# required by clang
target_link_libraries(libdnf-cli stdc++)

# install libdnf-cli.so
install(TARGETS libdnf-cli LIBRARY DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR})


# link libraries and set pkg-config requires

target_link_libraries(libdnf-cli libdnf)

pkg_check_modules(LIBFMT REQUIRED fmt)
list(APPEND LIBDNF_CLI_PC_REQUIRES "${LIBFMT_MODULE_NAME}")
target_link_libraries(libdnf-cli ${LIBFMT_LIBRARIES})

pkg_check_modules(SMARTCOLS REQUIRED smartcols)
list(APPEND LIBDNF_CLI_PC_REQUIRES_PRIVATE "${SMARTCOLS_MODULE_NAME}")
target_link_libraries(libdnf-cli ${SMARTCOLS_LIBRARIES})


# sort the pkg-config requires and concatenate them into a string
list(SORT LIBDNF_CLI_PC_REQUIRES)
list(JOIN LIBDNF_CLI_PC_REQUIRES ", " LIBDNF_CLI_PC_REQUIRES_STRING)
list(SORT LIBDNF_CLI_PC_REQUIRES_PRIVATE)
list(JOIN LIBDNF_CLI_PC_REQUIRES_PRIVATE ", " LIBDNF_CLI_PC_REQUIRES_PRIVATE_STRING)


# create a .pc file
configure_file("libdnf-cli.pc.in" ${CMAKE_CURRENT_BINARY_DIR}/libdnf-cli.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libdnf-cli.pc DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}/pkgconfig)
