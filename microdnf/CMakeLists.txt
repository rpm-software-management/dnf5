if(NOT WITH_MICRODNF)
    return()
endif()

find_package(Threads)

# set gettext domain for translations
add_definitions(-DGETTEXT_DOMAIN=\"microdnf\")

# use any sources found under the current directory
file(GLOB_RECURSE MICRODNF_SOURCES *.cpp)

include_directories(.)

add_executable(microdnf ${MICRODNF_SOURCES})

target_link_libraries(microdnf PUBLIC common libdnf libdnf-cli Threads::Threads)
install(TARGETS microdnf RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})


find_package(bash-completion)
if(BASH_COMPLETION_FOUND)
  message(STATUS "Using bash completion dir ${BASH_COMPLETION_COMPLETIONSDIR}")
else()
  set (BASH_COMPLETION_COMPLETIONSDIR "/etc/bash_completion.d")
  message (STATUS "Using fallback bash completion dir ${BASH_COMPLETION_COMPLETIONSDIR}")
endif()

install(FILES bash-completion/microdnf DESTINATION ${BASH_COMPLETION_COMPLETIONSDIR})
