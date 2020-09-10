if(NOT WITH_DNFDAEMON_SERVER)
    return()
endif()

set(DNFDAEMON_SERVER_BIN dnfdaemon-server)
file(GLOB_RECURSE DNFDAEMON_SERVER_SOURCES *.cpp)

find_package(sdbus-c++ REQUIRED)
find_package(Threads)

add_executable(${DNFDAEMON_SERVER_BIN} ${DNFDAEMON_SERVER_SOURCES})

target_link_libraries(
    ${DNFDAEMON_SERVER_BIN}
    PUBLIC
        libdnf
        libdnf-cli
        sdbus-c++
        Threads::Threads
)

install(TARGETS ${DNFDAEMON_SERVER_BIN} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

add_subdirectory(polkit)
add_subdirectory(dbus)
