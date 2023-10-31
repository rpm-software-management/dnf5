if(NOT WITH_PLUGIN_ACTIONS)
    return()
endif()

# set gettext domain for translations
set(GETTEXT_DOMAIN libdnf5-plugin-actions)
add_definitions(-DGETTEXT_DOMAIN=\"${GETTEXT_DOMAIN}\")

add_library(actions MODULE actions.cpp)

# disable the 'lib' prefix in order to create actions.so
set_target_properties(actions PROPERTIES PREFIX "")

pkg_check_modules(JSONC REQUIRED json-c)
include_directories(${JSONC_INCLUDE_DIRS})
target_link_libraries(actions PRIVATE ${JSONC_LIBRARIES})

target_link_libraries(actions PRIVATE libdnf5)

install(TARGETS actions LIBRARY DESTINATION "${CMAKE_INSTALL_FULL_LIBDIR}/libdnf5/plugins/")

install(FILES "actions.conf" DESTINATION "${CMAKE_INSTALL_FULL_SYSCONFDIR}/dnf/libdnf5-plugins")
install(DIRECTORY DESTINATION "${CMAKE_INSTALL_FULL_SYSCONFDIR}/dnf/libdnf5-plugins/actions.d")

add_subdirectory(po)
