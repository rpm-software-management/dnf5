add_library(changelog_cmd_plugin MODULE changelog.cpp changelog_cmd_plugin.cpp)
# disable the 'lib' prefix in order to create changelog_cmd_plugin.so
set_target_properties(changelog_cmd_plugin PROPERTIES PREFIX "")

install(TARGETS changelog_cmd_plugin LIBRARY DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}/microdnf5/plugins/)
