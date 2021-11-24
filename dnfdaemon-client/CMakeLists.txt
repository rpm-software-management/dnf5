if(NOT WITH_DNFDAEMON_CLIENT)
    return()
endif()

set(DNFDAEMON_CLIENT_BIN dnfdaemon-client)
file(GLOB_RECURSE DNFDAEMON_CLIENT_SOURCES *.cpp)

include_directories(.)
# TODO(mblaha) workround for dnfdaemon-client using server's headers, fix
include_directories(..)

pkg_check_modules(SDBUS_CPP REQUIRED sdbus-c++)

add_executable(${DNFDAEMON_CLIENT_BIN} ${DNFDAEMON_CLIENT_SOURCES})

target_link_libraries(
    ${DNFDAEMON_CLIENT_BIN}
    PUBLIC
        common
        libdnf
        libdnf-cli
        ${SDBUS_CPP_LIBRARIES}
        pthread
)

install(TARGETS ${DNFDAEMON_CLIENT_BIN} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
