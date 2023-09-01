if(NOT WITH_PLUGIN_RHSM)
    return()
endif()

# set gettext domain for translations
add_definitions(-DGETTEXT_DOMAIN=\"libdnf5\")

pkg_check_modules(RHSM REQUIRED librhsm>=0.0.3)
include_directories(${RHSM_INCLUDE_DIRS})

pkg_check_modules(GLIB REQUIRED glib-2.0>=2.44.0)
include_directories(${GLIB_INCLUDE_DIRS})

add_library(rhsm_plugin MODULE rhsm.cpp)

# disable the 'lib' prefix and set output_name in order to create rhsm.so
set_target_properties(rhsm_plugin PROPERTIES PREFIX "")
set_target_properties(rhsm_plugin PROPERTIES OUTPUT_NAME "rhsm")

target_link_libraries(rhsm_plugin PRIVATE ${RHSM_LIBRARIES})

target_link_libraries(rhsm_plugin PRIVATE ${GLIB_LIBRARIES})

target_link_libraries(rhsm_plugin PRIVATE libdnf5)

install(TARGETS rhsm_plugin LIBRARY DESTINATION "${CMAKE_INSTALL_FULL_LIBDIR}/libdnf5/plugins/")

install(FILES "rhsm.conf" DESTINATION "${CMAKE_INSTALL_FULL_SYSCONFDIR}/dnf/libdnf5-plugins")
