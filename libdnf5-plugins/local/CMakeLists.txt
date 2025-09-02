if(NOT WITH_PLUGIN_LOCAL)
    return()
endif()

# set gettext domain for translations
set(GETTEXT_DOMAIN libdnf5-plugin-local)
add_definitions(-DGETTEXT_DOMAIN=\"${GETTEXT_DOMAIN}\")

add_library(local_plugin MODULE local.cpp)

# disable the 'lib' prefix in order to create local.so
set_target_properties(local_plugin PROPERTIES PREFIX "")
set_target_properties(local_plugin PROPERTIES OUTPUT_NAME "local")

target_link_libraries(local_plugin PRIVATE libdnf5)
target_link_libraries(local_plugin PRIVATE common_obj)

install(TARGETS local_plugin LIBRARY DESTINATION "${CMAKE_INSTALL_FULL_LIBDIR}/libdnf5/plugins/")
install(FILES "local.conf" DESTINATION "${CMAKE_INSTALL_FULL_SYSCONFDIR}/dnf/libdnf5-plugins")
