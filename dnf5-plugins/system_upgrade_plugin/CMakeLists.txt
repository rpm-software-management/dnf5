set(SYSTEMD_UNIT_DIR /usr/lib/systemd/system)

# set gettext domain for translations
add_definitions(-DGETTEXT_DOMAIN=\"dnf5_cmd_system_upgrade\")

file(GLOB SYSTEM_UPGRADE_SOURCES *.cpp)
add_library(system_upgrade_cmd_plugin MODULE ${SYSTEM_UPGRADE_SOURCES})

# disable the 'lib' prefix in order to create system_upgrade_cmd_plugin.so
set_target_properties(system_upgrade_cmd_plugin PROPERTIES PREFIX "")

pkg_check_modules(SDBUS_CPP REQUIRED sdbus-c++)

target_link_libraries(system_upgrade_cmd_plugin PRIVATE dnf5 libdnf5 libdnf5-cli ${SDBUS_CPP_LIBRARIES})

install(FILES "dnf5-system-upgrade.service" DESTINATION ${SYSTEMD_UNIT_DIR})
install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
    ${SYSTEMD_UNIT_DIR}/dnf5-system-upgrade.service
    ${SYSTEMD_UNIT_DIR}/system-update.target.wants/dnf5-system-upgrade.service)"
)
install(TARGETS system_upgrade_cmd_plugin LIBRARY DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}/dnf5/plugins/)
