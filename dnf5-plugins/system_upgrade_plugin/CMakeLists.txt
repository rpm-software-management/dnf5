# set gettext domain for translations
add_definitions(-DGETTEXT_DOMAIN=\"dnf5_cmd_system_upgrade\")

file(GLOB SYSTEM_UPGRADE_SOURCES *.cpp)
add_library(system_upgrade_cmd_plugin MODULE ${SYSTEM_UPGRADE_SOURCES})

# disable the 'lib' prefix in order to create copr_cmd_plugin.so
set_target_properties(system_upgrade_cmd_plugin PROPERTIES PREFIX "")

target_link_libraries(system_upgrade_cmd_plugin PRIVATE dnf5 libdnf5 libdnf5-cli)

install(TARGETS system_upgrade_cmd_plugin LIBRARY DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}/dnf5/plugins/)
