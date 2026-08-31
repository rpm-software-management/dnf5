if(NOT WITH_PYTHON3)
    return()
endif()


find_package(Python3 REQUIRED)

# Generate test config file with CMAKE_INSTALL_PREFIX
configure_file(test_config.py.in test_config.py @ONLY)

add_subdirectory(libdnf5)
add_subdirectory(libdnf5_cli)
