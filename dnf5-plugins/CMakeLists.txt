if(NOT WITH_DNF5_PLUGINS)
    return()
endif()

include_directories("${PROJECT_SOURCE_DIR}/dnf5/include/")

add_subdirectory("changelog_plugin")
add_subdirectory("builddep_plugin")
