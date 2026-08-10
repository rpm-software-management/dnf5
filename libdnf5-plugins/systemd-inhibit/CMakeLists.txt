if(NOT WITH_PLUGIN_SYSTEMD_INHIBIT OR NOT WITH_SYSTEMD)
    return()
endif()

# set gettext domain for translations
set(GETTEXT_DOMAIN libdnf5-plugin-systemd-inhibit)
add_definitions(-DGETTEXT_DOMAIN=\"${GETTEXT_DOMAIN}\")

include(sdbus_cpp)

add_library(systemd_inhibit_plugin MODULE systemd-inhibit.cpp)

# disable the 'lib' prefix in order to create systemd-inhibit.so
set_target_properties(systemd_inhibit_plugin PROPERTIES PREFIX "")
set_target_properties(systemd_inhibit_plugin PROPERTIES OUTPUT_NAME "systemd-inhibit")

target_link_libraries(systemd_inhibit_plugin PRIVATE libdnf5)
target_link_libraries(systemd_inhibit_plugin PRIVATE ${SDBUS_CPP_LIBRARIES})

install(TARGETS systemd_inhibit_plugin LIBRARY DESTINATION "${CMAKE_INSTALL_FULL_LIBDIR}/libdnf5/plugins/")
install(FILES "00-systemd-inhibit.conf" DESTINATION "${CMAKE_INSTALL_FULL_SYSCONFDIR}/dnf/libdnf5-plugins")
