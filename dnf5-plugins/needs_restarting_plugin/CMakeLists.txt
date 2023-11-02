# set gettext domain for translations
add_definitions(-DGETTEXT_DOMAIN=\"dnf5_cmd_needs_restarting\")

add_library(needs_restarting_cmd_plugin MODULE needs_restarting.cpp needs_restarting_cmd_plugin.cpp)

# disable the 'lib' prefix in order to create needs_restarting_cmd_plugin.so
set_target_properties(needs_restarting_cmd_plugin PROPERTIES PREFIX "")

pkg_check_modules(SDBUS_CPP REQUIRED sdbus-c++)

target_link_libraries(needs_restarting_cmd_plugin PRIVATE dnf5 libdnf5 libdnf5-cli ${SDBUS_CPP_LIBRARIES})

install(TARGETS needs_restarting_cmd_plugin LIBRARY DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}/dnf5/plugins/)
