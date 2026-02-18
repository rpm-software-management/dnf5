# set gettext domain for translations
set(GETTEXT_DOMAIN template-dnf5-cmd-plugin)
add_definitions(-DGETTEXT_DOMAIN=\"${GETTEXT_DOMAIN}\")

#include_directories(${PROJECT_SOURCE_DIR}/libdnf5)
include_directories(${PROJECT_SOURCE_DIR}/dnf5/include)

add_library(template_dnf5_cmd_plugin MODULE template_cmd.cpp template_cmd_plugin.cpp)

# disable the 'lib' prefix in order to create template_cmd_plugin.so
set_target_properties(template_dnf5_cmd_plugin PROPERTIES PREFIX "")

#target_link_libraries(template_cmd_plugin PRIVATE libdnf5 libdnf5-cli)
target_link_libraries(template_dnf5_cmd_plugin PRIVATE dnf5)

# install plugin library
#install(TARGETS template_dnf5_cmd_plugin LIBRARY DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}/dnf5/plugins/)
