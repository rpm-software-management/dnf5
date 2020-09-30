if(NOT WITH_TESTS AND NOT WITH_PERFORMANCE_TESTS)
    return()
endif()


message("Building tests")


if(WITH_SANITIZERS)
    set(ASAN_LD_PRELOAD "LD_PRELOAD=${ASAN_LIBRARIES}")
    set(ASAN_OPTIONS "ASAN_OPTIONS=detect_odr_violation=0")
    set(LSAN_OPTIONS "LSAN_OPTIONS=suppressions=${CMAKE_CURRENT_SOURCE_DIR}/lsan-suppresions.txt")
endif()


# libdnf
add_subdirectory(libdnf)

# performance tests are available only for libdnf
if(WITH_PERFORMANCE_TESTS)
    return()
endif()

# libdnf-cli
add_subdirectory(libdnf-cli)

# bindings
add_subdirectory(go)
add_subdirectory(perl5)
add_subdirectory(python3)
add_subdirectory(ruby)

# components
add_subdirectory(dnfdaemon-server)
