if(NOT WITH_DNF5)
    return()
endif()


find_package(Threads)

add_definitions(-DDNF_BUILD_APPLICATION)

# set gettext domain for translations
set(GETTEXT_DOMAIN dnf5)
add_definitions(-DGETTEXT_DOMAIN=\"${GETTEXT_DOMAIN}\")

# set LIBDIR, used to find the directory containing the dnf5 plugins
add_definitions(-DLIBDIR=\"${CMAKE_INSTALL_FULL_LIBDIR}\")

# set INSTALL_PREFIX, used to find the directory containing the dnf5 default configuration
add_definitions(-DINSTALL_PREFIX=\"${CMAKE_INSTALL_PREFIX}\")

# set SYSCONFIG_DIR, used to find the directory containing the dnf5 configuration
add_definitions(-DSYSCONFIG_DIR=\"${CMAKE_INSTALL_FULL_SYSCONFDIR}\")

# use sources found under the current directory
file(GLOB_RECURSE DNF5_SOURCES *.cpp)

# exclude module sources if WITH_MODULEMD not defined
if (NOT WITH_MODULEMD)
    file(GLOB_RECURSE DNF5_SOURCES_MODULES commands/module/*.cpp)
    list(REMOVE_ITEM DNF5_SOURCES ${DNF5_SOURCES_MODULES})
endif()

include_directories("${PROJECT_SOURCE_DIR}/dnf5/include/")
include_directories(.)

add_executable(dnf5 ${DNF5_SOURCES})

# Enable symbol export. Needed for loadable modules (dnf5 plugins).
set_property(TARGET dnf5 PROPERTY ENABLE_EXPORTS 1)
# Export only explicitly marked symbols.
set_target_properties(dnf5 PROPERTIES C_VISIBILITY_PRESET hidden CXX_VISIBILITY_PRESET hidden)

target_link_libraries(dnf5 PRIVATE common_obj libdnf5 libdnf5-cli Threads::Threads)
install(TARGETS dnf5 RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

pkg_check_modules(RPM REQUIRED rpm>=4.19.0)

if(WITH_SYSTEMD)
  include(sdbus_cpp)
  pkg_check_modules(LIBSYSTEMD REQUIRED libsystemd)
  add_definitions(-DWITH_SYSTEMD)
  target_link_libraries(dnf5 PRIVATE ${RPM_LIBRARIES} ${SDBUS_CPP_LIBRARIES} ${LIBSYSTEMD_LIBRARIES})
else()
  target_link_libraries(dnf5 PRIVATE ${RPM_LIBRARIES})
endif()

find_package(bash-completion)
if(BASH_COMPLETION_FOUND)
  message(STATUS "Using bash completion dir ${BASH_COMPLETION_COMPLETIONSDIR}")
else()
  set (BASH_COMPLETION_COMPLETIONSDIR "/etc/bash_completion.d")
  message (STATUS "Using fallback bash completion dir ${BASH_COMPLETION_COMPLETIONSDIR}")
endif()

set(ZSH_COMPLETION_COMPLETIONSDIR "${CMAKE_INSTALL_PREFIX}/share/zsh/site-functions")
message(STATUS "Using zsh completion dir ${ZSH_COMPLETION_COMPLETIONSDIR}")

find_package(toml11 REQUIRED)
if (toml11_VERSION VERSION_LESS 4.0.0)
    add_definitions(-DTOML11_COMPAT)
endif()

install(FILES bash-completion/dnf5 DESTINATION ${BASH_COMPLETION_COMPLETIONSDIR})
install(FILES zsh-completion/_dnf5 DESTINATION ${ZSH_COMPLETION_COMPLETIONSDIR})
install(FILES "README.plugins" DESTINATION "${CMAKE_INSTALL_FULL_LIBDIR}/dnf5/plugins" RENAME "README")
install(DIRECTORY "config/usr/" DESTINATION "${CMAKE_INSTALL_PREFIX}" PATTERN ".gitkeep" EXCLUDE)
install(DIRECTORY "config/etc/" DESTINATION "${CMAKE_INSTALL_FULL_SYSCONFDIR}" PATTERN ".gitkeep" EXCLUDE)

if(WITH_SYSTEMD)
  pkg_check_modules(SYSTEMD REQUIRED systemd)
  pkg_get_variable(SYSTEMD_SYSTEM_UNIT_DIR systemd systemdsystemunitdir)
  install(DIRECTORY "config/systemd/system/" DESTINATION "${SYSTEMD_SYSTEM_UNIT_DIR}" PATTERN ".gitkeep" EXCLUDE)
endif()

# Makes an empty directory for dnf5-plugins configuration files
install(DIRECTORY DESTINATION "${CMAKE_INSTALL_FULL_SYSCONFDIR}/dnf/dnf5-plugins")
# Makes an empty directory for distribution dnf5-plugins configuration files
install(DIRECTORY DESTINATION "${CMAKE_INSTALL_PREFIX}/share/dnf5/dnf5-plugins")


add_subdirectory(po)
add_subdirectory("include")
