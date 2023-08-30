# set gettext domain for translations
add_definitions(-DGETTEXT_DOMAIN=\"dnf5_cmd_repoclosure\")

add_library(repoclosure_cmd_plugin MODULE repoclosure.cpp repoclosure_cmd_plugin.cpp)

# disable the 'lib' prefix in order to create repoclosure_cmd_plugin.so
set_target_properties(repoclosure_cmd_plugin PROPERTIES PREFIX "")

target_link_libraries(repoclosure_cmd_plugin PRIVATE libdnf5 libdnf5-cli)
target_link_libraries(repoclosure_cmd_plugin PRIVATE dnf5)

install(TARGETS repoclosure_cmd_plugin LIBRARY DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}/dnf5/plugins/)
