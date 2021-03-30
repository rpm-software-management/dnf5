/*
Copyright (C) 2018-2020 Red Hat, Inc.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "libdnf/conf/vars.hpp"

#include <dirent.h>
#include <sys/types.h>

#include <rpm/rpmdb.h>
#include <rpm/rpmlib.h>
#include <rpm/rpmts.h>

#include <algorithm>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <memory>

#define ASCII_LOWERCASE "abcdefghijklmnopqrstuvwxyz"
#define ASCII_UPPERCASE "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define ASCII_LETTERS   ASCII_LOWERCASE ASCII_UPPERCASE
#define DIGITS          "0123456789"

extern char ** environ;

namespace libdnf {

// ==================================================================
// The following helper functions should be moved e.g. into a library

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
                {"armhfp", {"armv6hl", "armv7hl", "armv7hnl", "armv8hl", "armv8hnl", "armv8hcnl", nullptr}},
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

static const char * get_base_arch(const char * arch) {
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

static constexpr const char * DISTROVERPKGS[] = {"system-release(releasever)",
                                                 "system-release",
                                                 "distribution-release(releasever)",
                                                 "distribution-release",
                                                 "redhat-release",
                                                 "suse-release"};

static std::string detect_arch() {
    init_lib_rpm();
    const char * value;
    rpmGetArchInfo(&value, nullptr);
    return value;
}

static std::string detect_release(const std::string & install_root_path) {
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

// ==================================================================

std::string Vars::substitute(const std::string & text) const {
    std::string res = text;

    if (values.empty()) {
        return res;
    }

    auto start = res.find_first_of('$');
    while (start != std::string::npos) {
        auto variable = start + 1;
        if (variable >= res.length()) {
            break;
        }
        bool bracket;
        if (res[variable] == '{') {
            bracket = true;
            if (++variable >= res.length()) {
                break;
            }
        } else {
            bracket = false;
        }
        auto it = std::find_if_not(
            res.begin() + static_cast<long>(variable), res.end(), [](char c) { return std::isalnum(c) != 0 || c == '_'; });
        if (bracket && it == res.end()) {
            break;
        }
        auto past_variable = static_cast<unsigned long>(std::distance(res.begin(), it));
        if (bracket && *it != '}') {
            start = res.find_first_of('$', past_variable);
            continue;
        }
        auto subst = values.find(res.substr(variable, past_variable - variable));
        if (subst != values.end()) {
            if (bracket) {
                ++past_variable;
            }
            res.replace(start, past_variable - start, subst->second);
            start = res.find_first_of('$', start + subst->second.length());
        } else {
            start = res.find_first_of('$', past_variable);
        }
    }

    return res;
}

void Vars::load(const std::string & installroot, const std::vector<std::string> & directories) {
    detect_vars(installroot);

    load_from_env();

    for (const auto & dir : directories) {
        load_from_dir(std::filesystem::path(installroot) / dir);
    }
}

void Vars::detect_vars(const std::string & installroot) {
    auto arch = detect_arch();
    values["arch"] = arch;
    values["basearch"] = get_base_arch(arch.c_str());
    values["releasever"] = detect_release(installroot);
}

static void dir_close(DIR * d) {
    closedir(d);
}

void Vars::load_from_dir(const std::string & directory) {
    if (DIR * dir = opendir(directory.c_str())) {
        std::unique_ptr<DIR, decltype(&dir_close)> dir_guard(dir, &dir_close);
        while (auto ent = readdir(dir)) {
            auto dname = ent->d_name;
            if (dname[0] == '.' && (dname[1] == '\0' || (dname[1] == '.' && dname[2] == '\0')))
                continue;

            auto full_path = directory;
            if (full_path.back() != '/') {
                full_path += "/";
            }
            full_path += dname;
            std::ifstream in_stream(full_path);
            if (in_stream.fail()) {
                // log.warning()
                continue;
            }
            std::string line;
            std::getline(in_stream, line);
            if (in_stream.fail()) {
                // log.warning()
                continue;
            }
            values[dname] = std::move(line);
        }
    }
}

void Vars::load_from_env() {
    if (!environ) {
        return;
    }

    for (const char * const * var_ptr = environ; *var_ptr; ++var_ptr) {
        auto var = *var_ptr;
        if (auto eql_ptr = strchr(var, '=')) {
            auto eql_idx = eql_ptr - var;
            if (eql_idx == 4 && strncmp("DNF", var, 3) == 0 && isdigit(var[3]) != 0) {
                // DNF[0-9]
                values[std::string(var, static_cast<size_t>(eql_idx))] = eql_ptr + 1;
            } else if (
                // DNF_VAR_[A-Za-z0-9_]+ , DNF_VAR_ prefix is cut off
                eql_idx > 8 && strncmp("DNF_VAR_", var, 8) == 0 &&
                static_cast<int>(strspn(var + 8, ASCII_LETTERS DIGITS "_")) == eql_idx - 8) {
                values[std::string(var + 8, static_cast<size_t>(eql_idx - 8))] = eql_ptr + 1;
            }
        }
    }
}

} // namespace libdnf
