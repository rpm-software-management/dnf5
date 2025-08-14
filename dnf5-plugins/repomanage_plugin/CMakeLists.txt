# set gettext domain for translations
set(GETTEXT_DOMAIN dnf5-plugin-repomanage)
add_definitions(-DGETTEXT_DOMAIN=\"${GETTEXT_DOMAIN}\")

add_library(repomanage_cmd_plugin MODULE repomanage.cpp repomanage_cmd_plugin.cpp)

# disable the 'lib' prefix in order to create repomanage_cmd_plugin.so
set_target_properties(repomanage_cmd_plugin PROPERTIES PREFIX "")

target_link_libraries(repomanage_cmd_plugin PRIVATE libdnf5 libdnf5-cli)
target_link_libraries(repomanage_cmd_plugin PRIVATE dnf5)
target_link_libraries(repomanage_cmd_plugin PRIVATE common_obj)

install(TARGETS repomanage_cmd_plugin LIBRARY DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}/dnf5/plugins/)
