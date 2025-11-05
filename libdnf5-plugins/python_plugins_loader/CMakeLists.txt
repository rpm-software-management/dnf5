if(NOT WITH_PYTHON3)
    return()
endif()

if(NOT WITH_PYTHON_PLUGINS_LOADER)
    return()
endif()

find_package(Python3 REQUIRED COMPONENTS Interpreter Development)
find_package(SWIG REQUIRED)
include_directories(${Python3_INCLUDE_DIRS})

add_custom_target(swigpyrun-header
    ${SWIG_EXECUTABLE} -python -external-runtime
    COMMENT "Generating SWIG Python runtime header"
)

# The generated SWIG Python runtime header: swigpyrun.h contains a problematic section on
# certain architectures (i686):
# error: comparison of integer expressions of different signedness: ‘int’ and ‘unsigned int’
# 658 |     for (cast = head; (cast - head) <= head->value; cast++) {
# Since we don't control that code don't treat it as an error.
add_compile_options(-Wno-error=sign-compare)

add_library(python_plugins_loader MODULE python_plugins_loader.cpp)

# python_plugins_loader requires swigpyrun.h so lets generate it
add_dependencies(python_plugins_loader swigpyrun-header)
# Include directory with the gereated swigpyrun.h header
target_include_directories(python_plugins_loader PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
# This is need because the generated swigpyrun.h contains warnings
# (which get treated as errors)
target_compile_options(python_plugins_loader PRIVATE -Wno-sign-conversion -Wno-missing-field-initializers)

# disable the 'lib' prefix in order to create python_plugins_loader.so
set_target_properties(python_plugins_loader PROPERTIES PREFIX "")

target_link_libraries(python_plugins_loader PRIVATE ${Python3_LIBRARIES})

target_link_libraries(python_plugins_loader PRIVATE libdnf5)

install(TARGETS python_plugins_loader LIBRARY DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}/libdnf5/plugins/)
install(FILES "README" DESTINATION ${Python3_SITELIB}/libdnf_plugins/)
install(FILES "python_plugins_loader.conf" DESTINATION "${CMAKE_INSTALL_FULL_SYSCONFDIR}/dnf/libdnf5-plugins")
# directory for Python plugin's configs
install(DIRECTORY DESTINATION "${CMAKE_INSTALL_FULL_SYSCONFDIR}/dnf/libdnf5-plugins/python_plugins_loader.d")
