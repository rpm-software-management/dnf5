if(NOT WITH_PLUGIN_RHSM_CONTAINER)
    return()
endif()

# set gettext domain for translations
set(GETTEXT_DOMAIN libdnf5-plugin-rhsm-container)
add_definitions(-DGETTEXT_DOMAIN=\"${GETTEXT_DOMAIN}\")

pkg_check_modules(RHSM REQUIRED librhsm>=0.0.3)
include_directories(${RHSM_INCLUDE_DIRS})

pkg_check_modules(GLIB REQUIRED glib-2.0>=2.44.0)
include_directories(${GLIB_INCLUDE_DIRS})

add_library(rhsm_plugin_container MODULE rhsm-container.cpp)

# disable the 'lib' prefix and set output_name in order to create rhsm-container.so
set_target_properties(rhsm_plugin_container PROPERTIES PREFIX "")
set_target_properties(rhsm_plugin_container PROPERTIES OUTPUT_NAME "rhsm-container")

target_link_libraries(rhsm_plugin_container PRIVATE ${RHSM_LIBRARIES})

target_link_libraries(rhsm_plugin_container PRIVATE ${GLIB_LIBRARIES})

target_link_libraries(rhsm_plugin_container PRIVATE libdnf5)

install(TARGETS rhsm_plugin_container LIBRARY DESTINATION "${CMAKE_INSTALL_FULL_LIBDIR}/libdnf5/plugins/")

install(FILES "rhsm-container.conf" DESTINATION "${CMAKE_INSTALL_FULL_SYSCONFDIR}/dnf/libdnf5-plugins")

add_subdirectory(po)
