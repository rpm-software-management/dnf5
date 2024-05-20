if(NOT WITH_LIBDNF5_CLI)
    return()
endif()

add_definitions(-DLIBDNF_CLI_BUILD_LIBRARY)

set(GETTEXT_DOMAIN libdnf5-cli)
add_definitions(-DGETTEXT_DOMAIN=\"${GETTEXT_DOMAIN}\")


# use any sources found under the current directory
file(GLOB_RECURSE LIBDNF5_CLI_SOURCES *.cpp)


# gather all pkg-config requires and write them to a .pc file later
list(APPEND LIBDNF5_CLI_PC_REQUIRES)
list(APPEND LIBDNF5_CLI_PC_REQUIRES_PRIVATE)

include_directories(.)

# build libdnf5-cli.so
add_library(libdnf5-cli SHARED ${LIBDNF5_CLI_SOURCES})
set(DNF_CLI_SO_VERSION 2)
set_target_properties(libdnf5-cli PROPERTIES OUTPUT_NAME "dnf5-cli")
set_target_properties(libdnf5-cli PROPERTIES SOVERSION ${DNF_CLI_SO_VERSION})
set_target_properties(libdnf5-cli PROPERTIES C_VISIBILITY_PRESET hidden CXX_VISIBILITY_PRESET hidden)
# required by clang
target_link_libraries(libdnf5-cli PUBLIC stdc++)

# install libdnf5-cli.so
install(TARGETS libdnf5-cli LIBRARY DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR})


# link libraries and set pkg-config requires

target_link_libraries(libdnf5-cli PUBLIC libdnf5)

pkg_check_modules(LIBFMT REQUIRED fmt)
list(APPEND LIBDNF5_CLI_PC_REQUIRES "${LIBFMT_MODULE_NAME}")
target_link_libraries(libdnf5-cli PUBLIC ${LIBFMT_LIBRARIES})

pkg_check_modules(SMARTCOLS REQUIRED smartcols)
list(APPEND LIBDNF5_CLI_PC_REQUIRES_PRIVATE "${SMARTCOLS_MODULE_NAME}")
target_link_libraries(libdnf5-cli PRIVATE ${SMARTCOLS_LIBRARIES})

pkg_check_modules(JSONC REQUIRED json-c)
include_directories(${JSONC_INCLUDE_DIRS})
target_link_libraries(libdnf5-cli PRIVATE ${JSONC_LIBRARIES})


# sort the pkg-config requires and concatenate them into a string
list(SORT LIBDNF5_CLI_PC_REQUIRES)
list(JOIN LIBDNF5_CLI_PC_REQUIRES ", " LIBDNF5_CLI_PC_REQUIRES_STRING)
list(SORT LIBDNF5_CLI_PC_REQUIRES_PRIVATE)
list(JOIN LIBDNF5_CLI_PC_REQUIRES_PRIVATE ", " LIBDNF5_CLI_PC_REQUIRES_PRIVATE_STRING)


# create a .pc file
configure_file("libdnf5-cli.pc.in" ${CMAKE_CURRENT_BINARY_DIR}/libdnf5-cli.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libdnf5-cli.pc DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}/pkgconfig)


add_subdirectory(po)
