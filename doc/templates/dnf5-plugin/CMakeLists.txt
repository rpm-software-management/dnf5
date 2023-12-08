# set gettext domain for translations
add_definitions(-DGETTEXT_DOMAIN=\"dnf5_cmd_template\")

# add your source files
add_library(template_cmd_plugin MODULE template.cpp template_cmd_plugin.cpp)

# disable the 'lib' prefix in order to create template_cmd_plugin.so
set_target_properties(template_cmd_plugin PROPERTIES PREFIX "")

# optionally, add your dependencies and link them to the plugin
# pkg_check_modules(RPM REQUIRED rpm)
# target_link_libraries(template_cmd_plugin PRIVATE ${RPM_LIBRARIES})

# link the default dnf libraries
target_link_libraries(template_cmd_plugin PRIVATE dnf5 libdnf5 libdnf5-cli)

# install the plugin into the common dnf5-plugins location
install(TARGETS template_cmd_plugin LIBRARY DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}/dnf5/plugins/)
