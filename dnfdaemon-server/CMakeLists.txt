if(NOT WITH_DNFDAEMON_SERVER)
    return()
endif()


file(GLOB_RECURSE DNFDAEMON_SERVER_SOURCES *.cpp)
add_executable(dnfdaemon-server main.cpp ${DNFDAEMON_SERVER_SOURCES})
target_link_libraries(dnfdaemon-server PUBLIC libdnf libdnf-cli)
install(TARGETS dnfdaemon-server RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
