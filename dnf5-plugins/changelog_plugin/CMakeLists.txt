# set gettext domain for translations
set(GETTEXT_DOMAIN dnf5-plugin-changelog)
add_definitions(-DGETTEXT_DOMAIN=\"${GETTEXT_DOMAIN}\")

add_library(changelog_cmd_plugin MODULE changelog.cpp changelog_cmd_plugin.cpp)

# disable the 'lib' prefix in order to create changelog_cmd_plugin.so
set_target_properties(changelog_cmd_plugin PROPERTIES PREFIX "")

target_link_libraries(changelog_cmd_plugin PRIVATE libdnf5 libdnf5-cli)
target_link_libraries(changelog_cmd_plugin PRIVATE dnf5)

install(TARGETS changelog_cmd_plugin LIBRARY DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}/dnf5/plugins/)

add_subdirectory(po)
