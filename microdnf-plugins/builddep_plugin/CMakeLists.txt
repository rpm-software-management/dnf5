add_library(builddep_cmd_plugin MODULE builddep.cpp builddep_cmd_plugin.cpp)

# disable the 'lib' prefix in order to create builddep_cmd_plugin.so
set_target_properties(builddep_cmd_plugin PROPERTIES PREFIX "")

find_library(RPMBUILD NAMES rpmbuild REQUIRED)
target_link_libraries(builddep_cmd_plugin ${RPMBUILD})

install(TARGETS builddep_cmd_plugin LIBRARY DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}/microdnf5/plugins/)
