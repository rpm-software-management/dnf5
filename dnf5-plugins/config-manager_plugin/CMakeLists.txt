# set gettext domain for translations
set(GETTEXT_DOMAIN dnf5-plugin-config-manager)
add_definitions(-DGETTEXT_DOMAIN=\"${GETTEXT_DOMAIN}\")

file(GLOB CONFIG_MANAGER_SOURCES *.cpp)
add_library(config-manager_cmd_plugin MODULE ${CONFIG_MANAGER_SOURCES})

# disable the 'lib' prefix in order to create changelog_cmd_plugin.so
set_target_properties(config-manager_cmd_plugin PROPERTIES PREFIX "")

pkg_check_modules(LIBFMT REQUIRED fmt)
target_link_libraries(libdnf5-cli PUBLIC ${LIBFMT_LIBRARIES})

find_package(CURL 7.62.0 REQUIRED)
include_directories(${CURL_INCLUDE_DIR})

find_package(ZLIB REQUIRED)

target_link_libraries(config-manager_cmd_plugin PRIVATE ZLIB::ZLIB)
target_link_libraries(config-manager_cmd_plugin PRIVATE libdnf5 libdnf5-cli)
target_link_libraries(config-manager_cmd_plugin PRIVATE dnf5)

install(TARGETS config-manager_cmd_plugin LIBRARY DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}/dnf5/plugins/)

add_subdirectory(po)
