if(NOT WITH_PYTHON3)
    return()
endif()


find_package(Python3 REQUIRED)


add_subdirectory(libdnf5)
add_subdirectory(libdnf5_cli)
