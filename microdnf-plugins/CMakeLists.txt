if(NOT WITH_MICRODNF_PLUGINS)
    return()
endif()

include_directories("${PROJECT_SOURCE_DIR}/microdnf/include/")

add_subdirectory("changelog_plugin")
