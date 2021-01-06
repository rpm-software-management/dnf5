if(NOT WITH_DNFDAEMON_CLIENT)
    return()
endif()

set(DNFDAEMON_CLIENT_BIN dnfdaemon-client)
file(GLOB_RECURSE DNFDAEMON_CLIENT_SOURCES *.cpp)

pkg_check_modules(SDBUS_CPP REQUIRED sdbus-c++)

add_executable(${DNFDAEMON_CLIENT_BIN} ${DNFDAEMON_CLIENT_SOURCES})

target_link_libraries(
    ${DNFDAEMON_CLIENT_BIN}
    PUBLIC
        libdnf
        libdnf-cli
        ${SDBUS_CPP_LIBRARIES}
        pthread
)

install(TARGETS ${DNFDAEMON_CLIENT_BIN} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
