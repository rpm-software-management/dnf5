if(NOT WITH_PYTHON3)
    return()
endif()

if(NOT WITH_PYTHON_PLUGINS_LOADER)
    return()
endif()

find_package(Python3 REQUIRED COMPONENTS Interpreter Development)
include_directories(${Python3_INCLUDE_DIRS})

add_library(python_plugins_loader MODULE python_plugins_loader.cpp)
# disable the 'lib' prefix in order to create python_plugins_loader.so
set_target_properties(python_plugins_loader PROPERTIES PREFIX "")

target_link_libraries(python_plugins_loader PUBLIC libdnf libdnf-cli)
target_link_libraries(python_plugins_loader PUBLIC ${Python3_LIBRARIES})

install(TARGETS python_plugins_loader LIBRARY DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}/libdnf5/plugins/)
install(FILES "README" DESTINATION ${Python3_SITELIB}/libdnf_plugins/)
