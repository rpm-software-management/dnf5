if(NOT WITH_TESTS AND NOT WITH_PERFORMANCE_TESTS)
    return()
endif()


message("Building tests")


if(WITH_SANITIZERS)
    # This fixes the "ASan runtime does not come first in initial library
    # list" message. Note the better solution should be to use
    # LD_PRELOAD=`ls /lib64/libasan.so.?`. With the solution used here the
    # sanitizer library is loaded when the DSO linked against it is loaded. For
    # any calls up to that point the sanitizers aren't active. While hackish,
    # this is convenient because e.g. python is causing some leaks during
    # module loading which this works around.
    set(ASAN_OPTIONS "ASAN_OPTIONS=verify_asan_link_order=0")
endif()


# build test data
add_subdirectory(data)

# libdnf5
add_subdirectory(libdnf5)

# performance tests are available only for libdnf5
if(WITH_PERFORMANCE_TESTS)
    return()
endif()

# libdnf5-cli
add_subdirectory(libdnf5-cli)

# tutorial
add_subdirectory(tutorial)

# bindings
add_subdirectory(go)
add_subdirectory(perl5)
add_subdirectory(python3)
add_subdirectory(ruby)

# components
add_subdirectory(dnf5daemon-server)
add_subdirectory(dnf5-plugins)

# add shared
add_subdirectory(shared)
