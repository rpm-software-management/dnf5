if(NOT WITH_DNFDAEMON_SERVER)
    return()
endif()

set(DNFDAEMON_SERVER_BIN dnfdaemon-server)
file(GLOB_RECURSE DNFDAEMON_SERVER_SOURCES *.cpp)

pkg_check_modules(SDBUS_CPP REQUIRED sdbus-c++)
find_package(Threads)

add_executable(${DNFDAEMON_SERVER_BIN} ${DNFDAEMON_SERVER_SOURCES})

target_link_libraries(
    ${DNFDAEMON_SERVER_BIN}
    PUBLIC
        libdnf
        libdnf-cli
        ${SDBUS_CPP_LIBRARIES}
        Threads::Threads
)

install(TARGETS ${DNFDAEMON_SERVER_BIN} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

add_subdirectory(polkit)
add_subdirectory(dbus)
