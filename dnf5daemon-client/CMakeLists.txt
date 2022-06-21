if(NOT WITH_DNF5DAEMON_CLIENT)
    return()
endif()

set(DNF5DAEMON_CLIENT_BIN dnf5daemon-client)
file(GLOB_RECURSE DNF5DAEMON_CLIENT_SOURCES *.cpp)

#set gettext domain for translations
add_definitions(-DGETTEXT_DOMAIN=\"dnf5-daemon-client\")

include_directories(.)
# TODO(mblaha) workround for dnf5daemon-client using server's headers, fix
include_directories(..)

pkg_check_modules(SDBUS_CPP REQUIRED sdbus-c++)

add_executable(${DNF5DAEMON_CLIENT_BIN} ${DNF5DAEMON_CLIENT_SOURCES})

target_link_libraries(
    ${DNF5DAEMON_CLIENT_BIN}
    PUBLIC
        common
        libdnf
        libdnf-cli
        ${SDBUS_CPP_LIBRARIES}
        pthread
)

install(TARGETS ${DNF5DAEMON_CLIENT_BIN} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
