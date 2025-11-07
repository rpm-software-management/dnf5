# set gettext domain for translations
set(GETTEXT_DOMAIN dnf5-plugin-obs)
add_definitions(-DGETTEXT_DOMAIN=\"${GETTEXT_DOMAIN}\")

file(GLOB OBS_SOURCES *.cpp)
add_library(obs_cmd_plugin MODULE ${OBS_SOURCES})

# disable the 'lib' prefix in order to create obs_cmd_plugin.so
set_target_properties(obs_cmd_plugin PROPERTIES PREFIX "")

pkg_check_modules(LIBXML2 REQUIRED libxml-2.0)
include_directories(${LIBXML2_INCLUDE_DIRS})
target_link_libraries(obs_cmd_plugin PRIVATE ${LIBXML2_LIBRARIES})

target_link_libraries(obs_cmd_plugin PRIVATE libdnf5 libdnf5-cli)
target_link_libraries(obs_cmd_plugin PRIVATE dnf5)

install(TARGETS obs_cmd_plugin LIBRARY DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}/dnf5/plugins/)

add_subdirectory(po)
