# set gettext domain for translations
set(GETTEXT_DOMAIN dnf5-plugin-automatic)
add_definitions(-DGETTEXT_DOMAIN=\"${GETTEXT_DOMAIN}\")

file(GLOB_RECURSE AUTOMATIC_SOURCES *.cpp)
add_library(automatic_cmd_plugin MODULE ${AUTOMATIC_SOURCES})

# disable the 'lib' prefix in order to create automatic_cmd_plugin.so
set_target_properties(automatic_cmd_plugin PROPERTIES PREFIX "")

target_link_libraries(automatic_cmd_plugin PRIVATE libdnf5 libdnf5-cli)
target_link_libraries(automatic_cmd_plugin PRIVATE dnf5)

install(TARGETS automatic_cmd_plugin LIBRARY DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}/dnf5/plugins/)
install(DIRECTORY "config/usr/" DESTINATION "${CMAKE_INSTALL_PREFIX}")

install(PROGRAMS bin/dnf-automatic TYPE BIN)

add_subdirectory(po)
