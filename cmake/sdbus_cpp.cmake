# Attempt to find sdbus-c++ version 2 (preferred)
pkg_check_modules(SDBUS_CPP REQUIRED sdbus-c++>=1)

if(SDBUS_CPP_VERSION LESS 2)
    message(STATUS "Using sdbus-c++ version 1")
else()
    # Define macro for version 2 if found
    add_definitions(-DSDBUS_CPP_VERSION_2)
    message(STATUS "Using sdbus-c++ version 2")
endif()
