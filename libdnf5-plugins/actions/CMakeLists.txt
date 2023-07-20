if(NOT WITH_PLUGIN_ACTIONS)
    return()
endif()

# set gettext domain for translations
add_definitions(-DGETTEXT_DOMAIN=\"libdnf5\")

add_library(actions MODULE actions.cpp)
# disable the 'lib' prefix in order to create actions.so
set_target_properties(actions PROPERTIES PREFIX "")

install(TARGETS actions LIBRARY DESTINATION "${CMAKE_INSTALL_FULL_LIBDIR}/libdnf5/plugins/")

install(FILES "actions.conf" DESTINATION "${CMAKE_INSTALL_FULL_SYSCONFDIR}/dnf/libdnf5-plugins")
install(DIRECTORY DESTINATION "${CMAKE_INSTALL_FULL_SYSCONFDIR}/dnf/libdnf5-plugins/actions.d")
