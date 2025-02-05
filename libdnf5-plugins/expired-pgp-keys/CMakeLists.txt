if(NOT WITH_PLUGIN_EXPIRED_PGP_KEYS)
    return()
endif()

# set gettext domain for translations
set(GETTEXT_DOMAIN libdnf5-plugin-expired-pgp-keys)
add_definitions(-DGETTEXT_DOMAIN=\"${GETTEXT_DOMAIN}\")

add_library(expired-pgp-keys MODULE expired-pgp-keys.cpp)

# disable the 'lib' prefix in order to create expired-pgp-keys.so
set_target_properties(expired-pgp-keys PROPERTIES PREFIX "")

target_link_libraries(expired-pgp-keys PRIVATE common_obj)
target_link_libraries(expired-pgp-keys PRIVATE libdnf5 libdnf5-cli)

pkg_check_modules(RPM REQUIRED rpm)
target_link_libraries(expired-pgp-keys PRIVATE ${RPM_LIBRARIES})

install(TARGETS expired-pgp-keys LIBRARY DESTINATION "${CMAKE_INSTALL_FULL_LIBDIR}/libdnf5/plugins/")

install(FILES "expired-pgp-keys.conf" DESTINATION "${CMAKE_INSTALL_FULL_SYSCONFDIR}/dnf/libdnf5-plugins")

add_subdirectory(po)
