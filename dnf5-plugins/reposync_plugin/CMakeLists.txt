# set gettext domain for translations
set(GETTEXT_DOMAIN dnf5-plugin-reposync)
add_definitions(-DGETTEXT_DOMAIN=\"${GETTEXT_DOMAIN}\")

add_library(reposync_cmd_plugin MODULE reposync.cpp reposync_cmd_plugin.cpp)

# disable the 'lib' prefix in order to create reposync_cmd_plugin.so
set_target_properties(reposync_cmd_plugin PROPERTIES PREFIX "")

target_link_libraries(reposync_cmd_plugin PRIVATE libdnf5 libdnf5-cli)
target_link_libraries(reposync_cmd_plugin PRIVATE dnf5)

install(TARGETS reposync_cmd_plugin LIBRARY DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}/dnf5/plugins/)
install(DIRECTORY "config/usr/" DESTINATION "${CMAKE_INSTALL_PREFIX}")

add_subdirectory(po)
