# set gettext domain for translations
add_definitions(-DGETTEXT_DOMAIN=\"dnf5_cmd_automatic\")

file(GLOB_RECURSE AUTOMATIC_SOURCES *.cpp)
add_library(automatic_cmd_plugin MODULE ${AUTOMATIC_SOURCES})

# disable the 'lib' prefix in order to create automatic_cmd_plugin.so
set_target_properties(automatic_cmd_plugin PROPERTIES PREFIX "")

target_link_libraries(automatic_cmd_plugin PRIVATE libdnf5 libdnf5-cli)
target_link_libraries(automatic_cmd_plugin PRIVATE dnf5)

install(TARGETS automatic_cmd_plugin LIBRARY DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}/dnf5/plugins/)
install(DIRECTORY "config/etc/" DESTINATION "${CMAKE_INSTALL_FULL_SYSCONFDIR}")
install(DIRECTORY "config/usr/" DESTINATION "${CMAKE_INSTALL_PREFIX}")

install(PROGRAMS bin/dnf-automatic TYPE BIN)
