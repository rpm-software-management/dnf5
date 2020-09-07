if(NOT WITH_MICRODNF)
    return()
endif()

find_package(Threads)

# use any sources found under the current directory
file(GLOB_RECURSE MICRODNF_SOURCES *.cpp)

add_executable(microdnf ${MICRODNF_SOURCES})

target_link_libraries(microdnf PUBLIC libdnf libdnf-cli Threads::Threads)
install(TARGETS microdnf RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
