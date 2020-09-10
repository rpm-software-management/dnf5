if(NOT WITH_DNFDAEMON_CLIENT)
    return()
endif()

set(DNFDAEMON_CLIENT_BIN dnfdaemon-client)
file(GLOB_RECURSE DNFDAEMON_CLIENT_SOURCES *.cpp)

find_package(sdbus-c++ REQUIRED)

add_executable(${DNFDAEMON_CLIENT_BIN} ${DNFDAEMON_CLIENT_SOURCES})

target_link_libraries(
    ${DNFDAEMON_CLIENT_BIN}
    PUBLIC
        libdnf
        libdnf-cli
        sdbus-c++
        pthread
)

install(TARGETS ${DNFDAEMON_CLIENT_BIN} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
