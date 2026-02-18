# set gettext domain for translations
set(GETTEXT_DOMAIN libdnf5-plugin-template)
add_definitions(-DGETTEXT_DOMAIN=\"${GETTEXT_DOMAIN}\")

# add your source files
add_library(template_plugin MODULE template.cpp)

# disable the 'lib' prefix in order to create template.so
set_target_properties(template_plugin PROPERTIES PREFIX "")

# link the libdnf5 library
target_link_libraries(template_plugin PRIVATE libdnf5)

# install the plugin into the common libdnf5-plugins location
#install(TARGETS template_plugin LIBRARY DESTINATION "${CMAKE_INSTALL_FULL_LIBDIR}/libdnf5/plugins/")

# install default plugin configuration file
#install(FILES "template.conf" DESTINATION "${CMAKE_INSTALL_FULL_SYSCONFDIR}/dnf/libdnf5-plugins")
