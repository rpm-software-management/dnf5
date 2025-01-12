if(NOT WITH_PLUGIN_APPSTREAM)
    return()
endif()

add_library(appstream_plugin MODULE appstream.cpp)

# disable the 'lib' prefix in order to create appstream.so
set_target_properties(appstream_plugin PROPERTIES PREFIX "")
set_target_properties(appstream_plugin PROPERTIES OUTPUT_NAME "appstream")

pkg_check_modules(APPSTREAM REQUIRED appstream>=0.16)
include_directories(${APPSTREAM_INCLUDE_DIRS})
target_link_libraries(appstream_plugin PRIVATE ${APPSTREAM_LIBRARIES})
target_link_libraries(appstream_plugin PRIVATE libdnf5)

install(TARGETS appstream_plugin LIBRARY DESTINATION "${CMAKE_INSTALL_FULL_LIBDIR}/libdnf5/plugins/")
install(FILES "appstream.conf" DESTINATION "${CMAKE_INSTALL_FULL_SYSCONFDIR}/dnf/libdnf5-plugins")
