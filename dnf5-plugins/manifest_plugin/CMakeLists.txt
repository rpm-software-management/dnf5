# set gettext domain for translations
add_definitions(-DGETTEXT_DOMAIN=\"dnf5_cmd_manifest\")

add_library(manifest_cmd_plugin MODULE manifest.cpp manifest_cmd_plugin.cpp)

set_target_properties(manifest_cmd_plugin PROPERTIES PREFIX "")

pkg_check_modules(LIBPKGMANIFEST REQUIRED libpkgmanifest)
target_link_libraries(manifest_cmd_plugin PRIVATE ${LIBPKGMANIFEST})

target_link_libraries(manifest_cmd_plugin PRIVATE dnf5 libdnf5 libdnf5-cli)

install(TARGETS manifest_cmd_plugin LIBRARY DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}/dnf5/plugins/)
