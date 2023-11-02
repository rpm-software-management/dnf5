# set gettext domain for translations
set(GETTEXT_DOMAIN dnf5-plugin-copr)
add_definitions(-DGETTEXT_DOMAIN=\"${GETTEXT_DOMAIN}\")

file(GLOB COPR_SOURCES *.cpp)
add_library(copr_cmd_plugin MODULE ${COPR_SOURCES})

# disable the 'lib' prefix in order to create copr_cmd_plugin.so
set_target_properties(copr_cmd_plugin PROPERTIES PREFIX "")

pkg_check_modules(JSONC REQUIRED json-c)
include_directories(${JSONC_INCLUDE_DIRS})
target_link_libraries(copr_cmd_plugin PRIVATE ${JSONC_LIBRARIES})

target_link_libraries(copr_cmd_plugin PRIVATE libdnf5 libdnf5-cli)
target_link_libraries(copr_cmd_plugin PRIVATE dnf5)

install(TARGETS copr_cmd_plugin LIBRARY DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}/dnf5/plugins/)

add_subdirectory(po)
