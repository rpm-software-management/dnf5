if(NOT WITH_PYTHON3)
    return()
endif()


find_package(Python3 REQUIRED)


add_subdirectory(libdnf)
add_subdirectory(libdnf_cli)
