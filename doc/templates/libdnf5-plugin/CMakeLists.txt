# setup conditional build
if(NOT WITH_PLUGIN_TEMPLATE)
    return()
endif()

# set gettext domain for translations
add_definitions(-DGETTEXT_DOMAIN=\"libdnf5\")

# add your source files
add_library(template MODULE template.cpp)

# disable the 'lib' prefix in order to create template.so
set_target_properties(template PROPERTIES PREFIX "")

# link the libdnf5 library
target_link_libraries(template PRIVATE libdnf5)

# install the plugin into the common libdnf5-plugins location
install(TARGETS template LIBRARY DESTINATION "${CMAKE_INSTALL_FULL_LIBDIR}/libdnf5/plugins/")
