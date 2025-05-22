message("Running CMake on dnf5...")
cmake_minimum_required(VERSION 3.21)


include(VERSION.cmake)
project(dnf5 LANGUAGES CXX C VERSION ${VERSION_PRIME}.${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO})
cmake_policy(VERSION ${CMAKE_VERSION})

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(SYSTEMD_DIR "/usr/lib/systemd/system")

message("Building ${PROJECT_NAME} version ${PROJECT_VERSION}")


if(DEFINED PACKAGE_VERSION)
    if(NOT ${PACKAGE_VERSION} STREQUAL ${PROJECT_VERSION})
        message(FATAL_ERROR "Variables PROJECT_VERSION and PACKAGE_VERSION differ. Make sure the versions in VERSION.cmake and the value provided via -DPACKAGE_VERSION are in sync.")
    endif()
endif()


# build options - components (sub-packages)
option(WITH_DNF5DAEMON_CLIENT "Build command-line interface for Dnf5daemon" ON)
option(WITH_DNF5DAEMON_SERVER "Build package management service with a DBus interface" ON)
option(WITH_LIBDNF5_CLI "Build library for working with a terminal in a command-line package manager" ON)
option(WITH_DNF5 "Build dnf5 command-line package manager" ON)
option(WITH_DNF5_PLUGINS "Build plugins for dnf5 command-line package manager" ON)
option(WITH_DNF5_OBSOLETES_DNF "Build with DNF 5 providing files that may conflict with DNF 4" ON)
option(WITH_PLUGIN_ACTIONS "Build a dnf5 actions plugin" ON)
option(WITH_PLUGIN_APPSTREAM "Build with plugin to install repo's Appstream metadata" ON)
option(WITH_PLUGIN_EXPIRED_PGP_KEYS "Build a libdnf5 expired pgp keys plugin" ON)
option(WITH_PLUGIN_RHSM "Build a libdnf5 rhsm (Red Hat Subscription Manager) plugin" OFF)
option(WITH_PYTHON_PLUGINS_LOADER "Build a special dnf5 plugin that loads Python plugins. Requires WITH_PYTHON3=ON." ON)

# build options - features
option(WITH_COMPS "Build with comps groups and environments support" ON)
option(WITH_MODULEMD "Build with modulemd modules support" ON)
option(WITH_SYSTEMD "Build with systemd and D-Bus features" ON)
option(ENABLE_SOLV_URPMREORDER "Build with support for URPM-like solution reordering?" OFF)
option(ENABLE_SOLV_FOCUSNEW "Build with SOLVER_FLAG_FOCUS_NEW libsolv flag enabled to ensure new dependencies are installed in latests versions?" ON)

# build options - documentation
option(WITH_HTML "Build HTML documentation" ON)
option(WITH_MAN "Build man pages" ON)

# build options - translations
option(WITH_TRANSLATIONS "Build translations" ON)

# build options - tests
option(WITH_TESTS "Build tests" ON)
option(WITH_PERFORMANCE_TESTS "Build performance tests (DEBUG ONLY)" OFF)
option(WITH_DNF5DAEMON_TESTS "Build dnf5daemon tests (requires configured dbus)" OFF)

# build options - debugging
option(WITH_SANITIZERS "Build with address, leak and undefined sanitizers (DEBUG ONLY)" OFF)

# build options - bindings
option(WITH_GO "Build Go bindings" OFF)
option(WITH_PERL5 "Build Perl 5 bindings" ON)
option(WITH_PYTHON3 "Build Python 3 bindings" ON)
option(WITH_RUBY "Build Ruby bindings" ON)


if(WITH_PERFORMANCE_TESTS AND WITH_SANITIZERS)
    message(FATAL_ERROR "Cannot perform performance tests with sanitizers enabled because they influence the results. Disable sanitizers to continue.")
endif()


# includes
include(GNUInstallDirs)
include(CheckLibraryExists)


# common dependencies
find_package(PkgConfig REQUIRED)


# C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Give inline methods hidden visibility by default
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

# remove PROJECT_SOURCE_DIR prefix from __FILE__ macro used in asserts (available since GCC 8 and Clang 10)
add_compile_options("-fmacro-prefix-map=${PROJECT_SOURCE_DIR}/=")

# warnings
add_compile_options(-Wall -Wextra -Werror)
add_compile_options(-Wcast-align -Wformat-nonliteral -Wmissing-format-attribute -Wredundant-decls -Wsign-compare -Wsign-conversion -Wtype-limits -Wuninitialized -Wwrite-strings)
add_compile_options(-Werror=unused-result -Wodr)

# not sure about the conversion warnings being errors; review later
add_compile_options(-Wconversion)

# linker options
if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # clang doesn't support this option
    add_compile_options(-Wl,--as-needed)
endif()

add_definitions(-DPROJECT_BINARY_DIR="${PROJECT_BINARY_DIR}")
add_definitions(-DPROJECT_SOURCE_DIR="${PROJECT_SOURCE_DIR}")

add_definitions(-DPROJECT_VERSION_PRIME=${VERSION_PRIME})
add_definitions(-DPROJECT_VERSION_MAJOR=${VERSION_MAJOR})
add_definitions(-DPROJECT_VERSION_MINOR=${VERSION_MINOR})
add_definitions(-DPROJECT_VERSION_MICRO=${VERSION_MICRO})

# libsolv options
if(ENABLE_SOLV_URPMREORDER)
    add_definitions(-DLIBSOLV_FLAG_URPMREORDER=1)
endif()
if(ENABLE_SOLV_FOCUSNEW)
    add_definitions(-DLIBSOLV_FLAG_FOCUSNEW=1)
endif()

if(WITH_SANITIZERS)
    message(WARNING "Building with sanitizers enabled!")
    add_compile_options(-fsanitize=address -fsanitize=undefined -fsanitize=leak)
    link_libraries(asan ubsan)
endif()

if(WITH_TRANSLATIONS)
    # define common command to generate the pot file from sources
    list(APPEND XGETTEXT_COMMAND xgettext -F --from-code=UTF-8 --keyword=_ --keyword=M_ --keyword=P_:1,2 --keyword=MP_:1,2 --keyword=C_:1c,2 --keyword=MC_:1c,2 --keyword=CP_:1c,2,3 --keyword=MCP_:1c,2,3 -c)
    add_custom_target(gettext-potfiles)
endif()

# When modules are enabled add WITH_MODULEMD definition for all sub-directories (all components).
if (WITH_MODULEMD)
    add_definitions(-DWITH_MODULEMD)
endif()

include_directories("${PROJECT_SOURCE_DIR}/include")
include_directories("${PROJECT_SOURCE_DIR}/common")


# libdnf5
add_subdirectory("common")
add_subdirectory("etc")
add_subdirectory("include")
add_subdirectory("libdnf5")
add_subdirectory("libdnf5-cli")
add_subdirectory("libdnf5-plugins")
add_subdirectory("doc")
add_subdirectory("bindings")

# dnf5daemon
add_subdirectory("dnf5daemon-server")
add_subdirectory("dnf5daemon-client")

# dnf5
add_subdirectory("dnf5")
add_subdirectory("dnf5-plugins")

# tests
enable_testing()
add_subdirectory("test")
