# find all *.in files, strip the suffix and configure them
file(GLOB_RECURSE in_files RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.in")
foreach(in_file ${in_files})
    string(REGEX REPLACE ".in$" "" out_file "${in_file}")
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/${in_file}" "${CMAKE_CURRENT_BINARY_DIR}/${out_file}" @ONLY)
endforeach()

add_custom_target(build_rpm_and_repos ALL COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/build-rpms-and-repos.sh "${CMAKE_CURRENT_BINARY_DIR}")
