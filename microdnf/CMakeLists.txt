if(NOT WITH_MICRODNF)
    return()
endif()


file(GLOB_RECURSE MICRODNF_SOURCES *.cpp)
add_executable(microdnf main.cpp ${DNFDAEMON_CLIENT_SOURCES})
target_link_libraries(microdnf PUBLIC libdnf libdnf-cli)
install(TARGETS microdnf RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
