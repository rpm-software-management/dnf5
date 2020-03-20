# use any sources found under the current directory
file(GLOB_RECURSE LIBDNF_SOURCES *.cpp)


# gather all pkg-config requires and write them to a .pc file later
list(APPEND LIBDNF_PC_REQUIRES)
list(APPEND LIBDNF_PC_REQUIRES_PRIVATE)


# build libdnf.so
add_library(libdnf SHARED ${LIBDNF_SOURCES})
set(DNF_SO_VERSION 2)
set_target_properties(libdnf PROPERTIES OUTPUT_NAME "dnf")
set_target_properties(libdnf PROPERTIES SOVERSION ${DNF_SO_VERSION})
# required by clang
target_link_libraries(libdnf stdc++)

# install libdnf.so
install(TARGETS libdnf LIBRARY DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR})


# link libraries and set pkg-config requires

pkg_check_modules(LIBFMT REQUIRED fmt)
list(APPEND LIBDNF_PC_REQUIRES "${LIBFMT_MODULE_NAME}")
target_link_libraries(libdnf ${LIBFMT_LIBRARIES})

pkg_check_modules(LIBSOLV REQUIRED libsolv>=0.7.7)
list(APPEND LIBDNF_PC_REQUIRES "${LIBSOLV_MODULE_NAME}")
target_link_libraries(libdnf ${LIBSOLV_LIBRARIES})

pkg_check_modules(LIBSOLVEXT REQUIRED libsolvext>=0.7.7)
list(APPEND LIBDNF_PC_REQUIRES_PRIVATE "${LIBSOLVEXT_MODULE_NAME}")
target_link_libraries(libdnf ${LIBSOLVEXT_LIBRARIES})

pkg_check_modules(RPM REQUIRED rpm>=4.11.0)
list(APPEND LIBDNF_PC_REQUIRES "${RPM_MODULE_NAME}")
target_link_libraries(libdnf ${RPM_LIBRARIES})

if(WITH_COMPS)
    pkg_check_modules(LIBCOMPS REQUIRED libcomps)
    list(APPEND LIBDNF_PC_REQUIRES_PRIVATE "${LIBCOMPS_MODULE_NAME}")
    target_link_libraries(libdnf ${LIBCOMPS_LIBRARIES})
endif()


# sort the pkg-config requires and concatenate them into a string
list(SORT LIBDNF_PC_REQUIRES)
list(JOIN LIBDNF_PC_REQUIRES ", " LIBDNF_PC_REQUIRES_STRING)
list(SORT LIBDNF_PC_REQUIRES_PRIVATE)
list(JOIN LIBDNF_PC_REQUIRES_PRIVATE ", " LIBDNF_PC_REQUIRES_PRIVATE_STRING)


# create a .pc file
configure_file("libdnf.pc.in" ${CMAKE_CURRENT_BINARY_DIR}/libdnf.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libdnf.pc DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}/pkgconfig)
