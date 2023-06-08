if(NOT WITH_PERL5)
    return()
endif()


message("Building bindings for perl5")


find_package(Perl REQUIRED)
find_package(PerlLibs REQUIRED)


# Preprocessor flags (cppflags) are already included in compiler flags
# (ccflags). Except a path to perl.h that is recorded nowhere.
include_directories(${PERL_INCLUDE_PATH})
perl_get_info(PERL_CFLAGS "ccflags")
separate_arguments(PERL_CFLAGS)



# Temporarily change Perl's FORTIFY_SOURCE define. Since fedora change
# https://fedoraproject.org/wiki/Changes/Add_FORTIFY_SOURCE%3D3_to_distribution_build_flags
# FORTIFY_SOURCE was changed from 2 to 3 however since current perl was compiled with the
# old settings it has stored 2 in its configs which is then propagated into its ccflags.
# Many (but not all) of the Perl's ccflags are already set in fedora build infra and are
# therefore duplicated, FORTIFY_SOURCE is one of them but it has different value: 3 vs. 2
# which leads to:
# <command-line>: error: "_FORTIFY_SOURCE" redefined [-Werror]
# First we remove _FORTIFY_SOURCE=2 if present in the current Perl ccflags then
# since fedora rawhide has _FORTIFY_SOURCE=3 and older fedora versions have _FORTIFY_SOURCE=2
# we undefine _FORTIFY_SOURCE in order to avoid redefinitions and finally we use the new
# _FORTIFY_SOURCE=3
list(REMOVE_ITEM PERL_CFLAGS -Wp,-D_FORTIFY_SOURCE=2)
list(APPEND PERL_CFLAGS -Wp,-U_FORTIFY_SOURCE)
list(APPEND PERL_CFLAGS -Wp,-D_FORTIFY_SOURCE=3)

# remove options unused by clang
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    list(REMOVE_ITEM PERL_CFLAGS -flto=auto)
    list(REMOVE_ITEM PERL_CFLAGS -ffat-lto-objects)
    list(REMOVE_ITEM PERL_CFLAGS -fstack-clash-protection)
    list(REMOVE_ITEM PERL_CFLAGS -specs=/usr/lib/rpm/redhat/redhat-hardened-cc1)
    list(REMOVE_ITEM PERL_CFLAGS -specs=/usr/lib/rpm/redhat/redhat-annobin-cc1)
endif()

add_compile_options(${PERL_CFLAGS})
perl_get_info(PERL_LDFLAGS "ccdlflags")
separate_arguments(PERL_LDFLAGS)
add_link_options(${PERL_LDFLAGS})


set(PERL_INSTALLDIRS "site" CACHE STRING
    "Perl installation directory identifier: core, vendor, site (default is site)"
)
if(PERL_INSTALLDIRS STREQUAL "core")
    set(PERL_INSTALL_PATH "${PERL_ARCHLIB}")
elseif(PERL_INSTALLDIRS STREQUAL "vendor")
    set(PERL_INSTALL_PATH "${PERL_VENDORARCH}")
elseif(PERL_INSTALLDIRS STREQUAL "site")
    set(PERL_INSTALL_PATH "${PERL_SITEARCH}")
else()
    message(FATAL_ERROR "Uknown PERL_INSTALLDIRS value: ${PERL_INSTALLDIRS}")
endif()
message(STATUS "Perl5 files will be installed to ${PERL_INSTALL_PATH}")


function(add_perl5_module LIBRARY_NAME MODULE_NAME)
    set(TARGET_NAME "perl5_${MODULE_NAME}")
    set_source_files_properties(../../${LIBRARY_NAME}/${MODULE_NAME}.i PROPERTIES CPLUSPLUS ON)
    set_property(SOURCE ../../${LIBRARY_NAME}/${MODULE_NAME}.i PROPERTY SWIG_MODULE_NAME ${LIBRARY_NAME}::${MODULE_NAME})
    swig_add_library(${TARGET_NAME} LANGUAGE perl SOURCES ../../${LIBRARY_NAME}/${MODULE_NAME}.i)
    set_property(TARGET ${TARGET_NAME} PROPERTY LIBRARY_OUTPUT_DIRECTORY "../auto/${LIBRARY_NAME}/${MODULE_NAME}")
    set_property(TARGET ${TARGET_NAME} PROPERTY OUTPUT_NAME ${MODULE_NAME})
    target_compile_options(${TARGET_NAME} PUBLIC ${SWIG_COMPILE_OPTIONS})

    string(REPLACE "_" "-" C_LIBRARY_NAME ${LIBRARY_NAME})
    swig_link_libraries(${TARGET_NAME} ${C_LIBRARY_NAME})

    install(TARGETS ${TARGET_NAME} LIBRARY DESTINATION "${PERL_INSTALL_PATH}/auto/${LIBRARY_NAME}/${MODULE_NAME}")
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${MODULE_NAME}.pm" DESTINATION "${PERL_INSTALL_PATH}/${LIBRARY_NAME}")
endfunction()


add_subdirectory(libdnf5)
add_subdirectory(libdnf5_cli)
