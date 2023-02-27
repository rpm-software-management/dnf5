# use any sources found under the current directory
file(GLOB_RECURSE LIBDNF5_SOURCES *.cpp)

# create config header file
configure_file("config.h.in" ${CMAKE_CURRENT_SOURCE_DIR}/conf/config.h)

# gather all pkg-config requires and write them to a .pc file later
list(APPEND LIBDNF5_PC_REQUIRES)
list(APPEND LIBDNF5_PC_REQUIRES_PRIVATE)

# set gettext domain for translations
set(GETTEXT_DOMAIN libdnf5)
add_definitions(-DGETTEXT_DOMAIN=\"${GETTEXT_DOMAIN}\")

# If defined, libsolv adds the prefix "dep_" to solvable dependencies.
# As a result, `requires` is renamed to `dep_requires`.
# Needed for C++20. `requires` is a keyword in C++20.
add_definitions(-DLIBSOLV_SOLVABLE_PREPEND_DEP)

include_directories(.)

# build libdnf.so
add_library(libdnf SHARED ${LIBDNF5_SOURCES})
set(DNF_SO_VERSION 1)
set_target_properties(libdnf PROPERTIES OUTPUT_NAME "dnf5")
set_target_properties(libdnf PROPERTIES SOVERSION ${DNF_SO_VERSION})
# required to have dlopen symbol
target_link_libraries(libdnf ${CMAKE_DL_LIBS})

# required by clang
target_link_libraries(libdnf stdc++)

# link stdc++fs to make experimental std::filesystem available on gcc 8:
# https://gcc.gnu.org/onlinedocs/libstdc++/manual/using.html
target_link_libraries(libdnf stdc++fs)

# install libdnf.so
install(TARGETS libdnf LIBRARY DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR})

target_link_libraries(libdnf common)

# link libraries and set pkg-config requires

find_package(toml11 REQUIRED)

pkg_check_modules(LIBFMT REQUIRED fmt)
list(APPEND LIBDNF5_PC_REQUIRES "${LIBFMT_MODULE_NAME}")
target_link_libraries(libdnf ${LIBFMT_LIBRARIES})

pkg_check_modules(JSONC REQUIRED json-c)
include_directories(${JSONC_INCLUDE_DIRS})
target_link_libraries(libdnf ${JSONC_LIBRARIES})

pkg_check_modules(LIBMODULEMD REQUIRED modulemd-2.0>=2.11.2)
list(APPEND LIBDNF5_PC_REQUIRES "${LIBMODULEMD_MODULE_NAME}")
target_link_libraries(libdnf ${LIBMODULEMD_LIBRARIES})

pkg_check_modules(LIBSOLV REQUIRED libsolv>=0.7.21)
list(APPEND LIBDNF5_PC_REQUIRES "${LIBSOLV_MODULE_NAME}")
target_link_libraries(libdnf ${LIBSOLV_LIBRARIES})

pkg_check_modules(LIBSOLVEXT REQUIRED libsolvext>=0.7.7)
list(APPEND LIBDNF5_PC_REQUIRES_PRIVATE "${LIBSOLVEXT_MODULE_NAME}")
target_link_libraries(libdnf ${LIBSOLVEXT_LIBRARIES})

if(WITH_STATIC_LIBSOLV)
    pkg_check_modules(BZIP2 REQUIRED bzip2)
    list(APPEND LIBDNF5_PC_REQUIRES_PRIVATE "${BZIP2_MODULE_NAME}")
    target_link_libraries(libdnf ${BZIP2_LIBRARIES})

    pkg_check_modules(LZMA REQUIRED liblzma)
    list(APPEND LIBDNF5_PC_REQUIRES_PRIVATE "${LZMA_MODULE_NAME}")
    target_link_libraries(libdnf ${LZMA_LIBRARIES})

    pkg_check_modules(ZLIB REQUIRED zlib)
    list(APPEND LIBDNF5_PC_REQUIRES_PRIVATE "${ZLIB_MODULE_NAME}")
    target_link_libraries(libdnf ${ZLIB_LIBRARIES})

    pkg_check_modules(ZSTD REQUIRED libzstd)
    list(APPEND LIBDNF5_PC_REQUIRES_PRIVATE "${ZSTD_MODULE_NAME}")
    target_link_libraries(libdnf ${ZSTD_LIBRARIES})
endif()

if(WITH_COMPS OR WITH_STATIC_LIBSOLV)
    pkg_check_modules(LIBXML2 REQUIRED libxml-2.0)
    list(APPEND LIBDNF5_PC_REQUIRES_PRIVATE "${LIBXML2_MODULE_NAME}")
    include_directories(${LIBXML2_INCLUDE_DIRS})
    target_link_libraries(libdnf ${LIBXML2_LIBRARIES})
endif()

pkg_check_modules(RPM REQUIRED rpm>=4.17.0)
list(APPEND LIBDNF5_PC_REQUIRES "${RPM_MODULE_NAME}")
target_link_libraries(libdnf ${RPM_LIBRARIES})

if (WITH_ZCHUNK)
    pkg_check_modules(ZCHUNKLIB zck>=0.9.11 REQUIRED)
    add_definitions(-DWITH_ZCHUNK)
endif()

# GLIB librepo and libmodulemd uses glib2 in API :(
pkg_check_modules (GLIB2 glib-2.0>=2.46.0)
include_directories(${GLIB2_INCLUDE_DIRS})
target_link_libraries(libdnf ${GLIB2_LIBRARIES})

pkg_check_modules(LIBREPO REQUIRED librepo>=1.15.0)
list(APPEND LIBDNF5_PC_REQUIRES "${LIBREPO_MODULE_NAME}")
target_include_directories(libdnf PRIVATE ${LIBREPO_INCLUDE_DIRS})
target_link_libraries(libdnf ${LIBREPO_LDFLAGS})

# SQLite3
pkg_check_modules(SQLite3 REQUIRED sqlite3)
list(APPEND LIBDNF5_PC_REQUIRES "${SQLite3_MODULE_NAME}")
target_link_libraries(libdnf ${SQLite3_LIBRARIES})


# sort the pkg-config requires and concatenate them into a string
list(SORT LIBDNF5_PC_REQUIRES)
list(JOIN LIBDNF5_PC_REQUIRES ", " LIBDNF5_PC_REQUIRES_STRING)
list(SORT LIBDNF5_PC_REQUIRES_PRIVATE)
list(JOIN LIBDNF5_PC_REQUIRES_PRIVATE ", " LIBDNF5_PC_REQUIRES_PRIVATE_STRING)


# create a .pc file
configure_file("libdnf.pc.in" ${CMAKE_CURRENT_BINARY_DIR}/libdnf.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libdnf.pc DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}/pkgconfig)

# Makes an empty directory for libdnf cache
install(DIRECTORY DESTINATION ${CMAKE_INSTALL_FULL_LOCALSTATEDIR}/cache/libdnf)


add_subdirectory(po)
