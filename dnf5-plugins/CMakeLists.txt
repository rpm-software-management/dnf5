if(NOT WITH_DNF5_PLUGINS)
    return()
endif()

include_directories("${PROJECT_SOURCE_DIR}/dnf5/include/")
# These plugins use symbols from dnf5 program, hence the symbold are undefined
# at link time and cannot pass "-z defs" linker check. Disable the check.
set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Wl,-z,undefs")

add_subdirectory("automatic_plugin")
add_subdirectory("builddep_plugin")
add_subdirectory("changelog_plugin")
add_subdirectory("config-manager_plugin")
add_subdirectory("copr_plugin")
add_subdirectory("needs_restarting_plugin")
add_subdirectory("repoclosure_plugin")
