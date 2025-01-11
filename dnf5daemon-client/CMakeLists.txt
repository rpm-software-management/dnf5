if(NOT WITH_DNF5DAEMON_CLIENT)
    return()
endif()

set(DNF5DAEMON_CLIENT_BIN dnf5daemon-client)
file(GLOB_RECURSE DNF5DAEMON_CLIENT_SOURCES *.cpp)

#set gettext domain for translations
set(GETTEXT_DOMAIN dnf5daemon-client)
add_definitions(-DGETTEXT_DOMAIN=\"${GETTEXT_DOMAIN}\")

include_directories(.)
# TODO(mblaha) workaround for dnf5daemon-client using server's headers, fix
include_directories(..)

pkg_check_modules(SDBUS_CPP REQUIRED sdbus-c++>=0.9.0)

pkg_check_modules(JSONC REQUIRED json-c)
include_directories(${JSONC_INCLUDE_DIRS})

add_executable(${DNF5DAEMON_CLIENT_BIN} ${DNF5DAEMON_CLIENT_SOURCES})

target_link_libraries(
    ${DNF5DAEMON_CLIENT_BIN}
    PRIVATE
        common_obj
        libdnf5
        libdnf5-cli
        ${SDBUS_CPP_LIBRARIES}
        ${JSONC_LIBRARIES}
        pthread
)

install(TARGETS ${DNF5DAEMON_CLIENT_BIN} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})


add_subdirectory(po)
