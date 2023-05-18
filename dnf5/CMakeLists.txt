if(NOT WITH_DNF5)
    return()
endif()

find_package(Threads)

# set gettext domain for translations
set(GETTEXT_DOMAIN dnf5)
add_definitions(-DGETTEXT_DOMAIN=\"${GETTEXT_DOMAIN}\")

# set LIBDIR, used to find the directory containing the dnf5 plugins
add_definitions(-DLIBDIR=\"${CMAKE_INSTALL_FULL_LIBDIR}\")

# set INSTALL_PREFIX, used to find the directory containing the dnf5 default configuration
add_definitions(-DINSTALL_PREFIX=\"${CMAKE_INSTALL_PREFIX}\")

# set SYSCONFIG_DIR, used to find the directory containing the dnf5 configuration
add_definitions(-DSYSCONFIG_DIR=\"${CMAKE_INSTALL_FULL_SYSCONFDIR}\")

# use any sources found under the current directory
file(GLOB_RECURSE DNF5_SOURCES *.cpp)

include_directories("${PROJECT_SOURCE_DIR}/dnf5/include/")
include_directories(.)

add_executable(dnf5 ${DNF5_SOURCES})

# Enable symbol export. Needed for loadable modules (dnf5 plugins).
set_property(TARGET dnf5 PROPERTY ENABLE_EXPORTS 1)

target_link_libraries(dnf5 PUBLIC common libdnf libdnf-cli Threads::Threads)
install(TARGETS dnf5 RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})


find_package(bash-completion)
if(BASH_COMPLETION_FOUND)
  message(STATUS "Using bash completion dir ${BASH_COMPLETION_COMPLETIONSDIR}")
else()
  set (BASH_COMPLETION_COMPLETIONSDIR "/etc/bash_completion.d")
  message (STATUS "Using fallback bash completion dir ${BASH_COMPLETION_COMPLETIONSDIR}")
endif()

install(FILES bash-completion/dnf5 DESTINATION ${BASH_COMPLETION_COMPLETIONSDIR})
if(WITH_DNF5_PLUGINS)
    install(FILES "README.plugins" DESTINATION "${CMAKE_INSTALL_FULL_LIBDIR}/dnf5/plugins" RENAME "README")
endif()
install(DIRECTORY "config/usr/" DESTINATION "${CMAKE_INSTALL_PREFIX}")
install(DIRECTORY "config/etc/" DESTINATION "${CMAKE_INSTALL_FULL_SYSCONFDIR}")


add_subdirectory(po)
add_subdirectory("include")
