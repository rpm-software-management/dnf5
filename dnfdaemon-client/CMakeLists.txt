if(NOT WITH_DNFDAEMON_CLIENT)
    return()
endif()


file(GLOB_RECURSE DNFDAEMON_CLIENT_SOURCES *.cpp)
add_executable(dnfdaemon-client main.cpp ${DNFDAEMON_CLIENT_SOURCES})
target_link_libraries(dnfdaemon-client PUBLIC libdnf libdnf-cli)
install(TARGETS dnfdaemon-client RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
