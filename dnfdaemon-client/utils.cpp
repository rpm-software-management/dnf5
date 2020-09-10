/*
Copyright (C) 2020 Red Hat, Inc.

This file is part of dnfdaemon-client: https://github.com/rpm-software-management/libdnf/

Dnfdaemon-client is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Dnfdaemon-client is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with dnfdaemon-client.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "utils.hpp"

#include <pwd.h>
#include <rpm/rpmdb.h>
#include <rpm/rpmlib.h>
#include <rpm/rpmts.h>
#include <unistd.h>

#include <cstdlib>
#include <cstring>
#include <stdexcept>

namespace dnfdaemon::client {

bool am_i_root() noexcept {
    return geteuid() == 0;
}

namespace xdg {

std::filesystem::path get_user_home_dir() {
    if (char * dir = std::getenv("HOME")) {
        return std::filesystem::path(dir);
    }
    if (struct passwd * pw = getpwuid(getuid())) {
        return std::filesystem::path(pw->pw_dir);
    }
    throw std::runtime_error("get_home_dir(): Can't determine the user's home directory");
}

std::filesystem::path get_user_cache_dir() {
    if (char * dir = std::getenv("XDG_CACHE_HOME")) {
        return std::filesystem::path(dir);
    }
    return std::filesystem::path(get_user_home_dir()) / ".cache";
}

std::filesystem::path get_user_config_dir() {
    if (char * dir = std::getenv("XDG_CONFIG_HOME")) {
        return std::filesystem::path(dir);
    }
    return std::filesystem::path(get_user_home_dir()) / ".config";
}

std::filesystem::path get_user_data_dir() {
    if (char * dir = std::getenv("XDG_DATA_HOME")) {
        return std::filesystem::path(dir);
    }
    return std::filesystem::path(get_user_home_dir()) / ".local" / "share";
}

std::filesystem::path get_user_runtime_dir() {
    if (char * dir = std::getenv("XDG_RUNTIME_HOME")) {
        return std::filesystem::path(dir);
    }
    return get_user_cache_dir();
}

}  // namespace xdg

// ================================================
// Next utils probably will be move into library

#define MAX_NATIVE_ARCHES 12

// data taken from DNF
static const struct {
    const char * base;
    const char * native[MAX_NATIVE_ARCHES];
} ARCH_MAP[] = {{"aarch64", {"aarch64", nullptr}},
                {"alpha",
                 {"alpha",
                  "alphaev4",
                  "alphaev45",
                  "alphaev5",
                  "alphaev56",
                  "alphaev6",
                  "alphaev67",
                  "alphaev68",
                  "alphaev7",
                  "alphapca56",
                  nullptr}},
                {"arm", {"armv5tejl", "armv5tel", "armv5tl", "armv6l", "armv7l", "armv8l", nullptr}},
                {"armhfp", {"armv7hl", "armv7hnl", "armv8hl", "armv8hnl", "armv8hcnl", nullptr}},
                {"i386", {"i386", "athlon", "geode", "i386", "i486", "i586", "i686", nullptr}},
                {"ia64", {"ia64", nullptr}},
                {"mips", {"mips", nullptr}},
                {"mipsel", {"mipsel", nullptr}},
                {"mips64", {"mips64", nullptr}},
                {"mips64el", {"mips64el", nullptr}},
                {"noarch", {"noarch", nullptr}},
                {"ppc", {"ppc", nullptr}},
                {"ppc64", {"ppc64", "ppc64iseries", "ppc64p7", "ppc64pseries", nullptr}},
                {"ppc64le", {"ppc64le", nullptr}},
                {"riscv32", {"riscv32", nullptr}},
                {"riscv64", {"riscv64", nullptr}},
                {"riscv128", {"riscv128", nullptr}},
                {"s390", {"s390", nullptr}},
                {"s390x", {"s390x", nullptr}},
                {"sh3", {"sh3", nullptr}},
                {"sh4", {"sh4", "sh4a", nullptr}},
                {"sparc", {"sparc", "sparc64", "sparc64v", "sparcv8", "sparcv9", "sparcv9v", nullptr}},
                {"x86_64", {"x86_64", "amd64", "ia32e", nullptr}},
                {nullptr, {nullptr}}};

const char * get_base_arch(const char * arch) {
    for (int i = 0; ARCH_MAP[i].base; ++i) {
        for (int j = 0; ARCH_MAP[i].native[j]; ++j) {
            if (std::strcmp(ARCH_MAP[i].native[j], arch) == 0) {
                return ARCH_MAP[i].base;
            }
        }
    }
    return nullptr;
}

static void init_lib_rpm() {
    static bool lib_rpm_initiated{false};
    if (!lib_rpm_initiated) {
        if (rpmReadConfigFiles(nullptr, nullptr) != 0) {
            throw std::runtime_error("failed to read rpm config files\n");
        }
        lib_rpm_initiated = true;
    }
}

//#define RELEASEVER_PROV "system-release(releasever)"

static constexpr const char * DISTROVERPKGS[] = {"system-release(releasever)",
                                                 "system-release",
                                                 "distribution-release(releasever)",
                                                 "distribution-release",
                                                 "redhat-release",
                                                 "suse-release"};

std::string detect_arch() {
    init_lib_rpm();
    const char * value;
    rpmGetArchInfo(&value, nullptr);
    return value;
}

std::string detect_release(const std::string & install_root_path) {
    init_lib_rpm();
    std::string release_ver;

    bool found_in_rpmdb{false};
    auto ts = rpmtsCreate();
    rpmtsSetRootDir(ts, install_root_path.c_str());
    for (auto distroverpkg : DISTROVERPKGS) {
        auto mi = rpmtsInitIterator(ts, RPMTAG_PROVIDENAME, distroverpkg, 0);
        while (auto hdr = rpmdbNextIterator(mi)) {
            auto version = headerGetString(hdr, RPMTAG_VERSION);
            rpmds ds = rpmdsNew(hdr, RPMTAG_PROVIDENAME, 0);
            while (rpmdsNext(ds) >= 0) {
                if (std::strcmp(rpmdsN(ds), distroverpkg) == 0 && rpmdsFlags(ds) == RPMSENSE_EQUAL) {
                    version = rpmdsEVR(ds);
                }
            }
            release_ver = version;
            found_in_rpmdb = true;
            rpmdsFree(ds);
            break;
        }
        rpmdbFreeIterator(mi);
        if (found_in_rpmdb) {
            break;
        }
    }
    rpmtsFree(ts);
    if (found_in_rpmdb) {
        return release_ver;
    }
    return "";
}

}  // namespace dnfdaemon::client
