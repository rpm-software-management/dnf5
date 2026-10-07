# set gettext domain for translations
set(GETTEXT_DOMAIN dnf5-plugin-deptree)
add_definitions(-DGETTEXT_DOMAIN=\"${GETTEXT_DOMAIN}\")

add_library(deptree_cmd_plugin MODULE deptree.cpp deptree_graph.cpp deptree_cmd_plugin.cpp)
set_target_properties(deptree_cmd_plugin PROPERTIES PREFIX "")

target_link_libraries(deptree_cmd_plugin PRIVATE libdnf5 libdnf5-cli dnf5)

install(TARGETS deptree_cmd_plugin LIBRARY DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}/dnf5/plugins/)

add_subdirectory(po)
