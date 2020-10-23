/*
Copyright (C) 2020 Red Hat, Inc.

This file is part of microdnf: https://github.com/rpm-software-management/libdnf/

Microdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Microdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with microdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "utils.hpp"

#include <fcntl.h>
#include <libsmartcols/libsmartcols.h>
#include <pwd.h>
#include <sys/stat.h>
#include <unistd.h>

#include <fmt/format.h>

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <stdexcept>

namespace microdnf {

bool am_i_root() noexcept {
    return geteuid() == 0;
}

static constexpr uid_t INVALID_UID = static_cast<uid_t>(-1);

/// Read the login uid from the "/proc/self/loginuid".
static uid_t read_login_uid_from_proc() noexcept {
    auto in = open("/proc/self/loginuid", O_RDONLY | O_NOFOLLOW | O_CLOEXEC);
    if (in == -1) {
        return INVALID_UID;
    }

    ssize_t len;
    char buf[16];
    do {
        errno = 0;
        len = read(in, buf, sizeof(buf) - 1);
    } while (len < 0 && errno == EINTR);

    close(in);

    if (len <= 0) {
        return INVALID_UID;
    }

    buf[len] = '\0';
    char * endptr;
    errno = 0;
    auto uid = static_cast<uid_t>(strtol(buf, &endptr, 10));
    if (buf == endptr || errno != 0) {
        return INVALID_UID;
    }

    return uid;
}

uid_t get_login_uid() noexcept {
    static uid_t cached_uid = INVALID_UID;
    if (cached_uid == INVALID_UID) {
        cached_uid = read_login_uid_from_proc();
        if (cached_uid == INVALID_UID) {
            cached_uid = getuid();
        }
    }
    return cached_uid;
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

// transaction details columns
enum { COL_NEVRA, COL_REPO, COL_SIZE };

static void add_transaction_packages(struct libscols_table *tb, struct libscols_line *parent, std::vector<libdnf::rpm::Package> & pkgs) {
    // TODO(jrohel): Print relations with obsoleted packages, sorting
    for (auto & pkg : pkgs) {
        struct libscols_line *ln = scols_table_new_line(tb, parent);
        scols_line_set_data(ln, COL_NEVRA, pkg.get_full_nevra().c_str());
        scols_line_set_data(ln, COL_REPO, pkg.get_repo()->get_id().c_str());
        scols_line_set_data(ln, COL_SIZE, std::to_string(pkg.get_size()).c_str());
    }
}

bool print_goal(libdnf::Goal & goal) {
    auto installs_pkgs = goal.list_rpm_installs();
    auto reinstalls_pkgs = goal.list_rpm_reinstalls();
    auto upgrades_pkgs = goal.list_rpm_upgrades();
    auto downgrades_pkgs = goal.list_rpm_downgrades();
    auto removes_pkgs = goal.list_rpm_removes();
    auto obsoleded_pkgs = goal.list_rpm_obsoleted();

    if (installs_pkgs.empty() && reinstalls_pkgs.empty() && upgrades_pkgs.empty() &&
        downgrades_pkgs.empty() && removes_pkgs.empty() && obsoleded_pkgs.empty()) {
        std::cout << "Nothing to do." << std::endl;
        return false;
    }

    struct libscols_line *ln;

    struct libscols_table *tb = scols_new_table();
    scols_table_new_column(tb, "Package",    0.7, SCOLS_FL_TREE);
    scols_table_new_column(tb, "Repository", 0.2, SCOLS_FL_TRUNC);
    scols_table_new_column(tb, "Size",       0.1, SCOLS_FL_RIGHT);
    scols_table_enable_maxout(tb, 1);
    struct libscols_symbols *sb = scols_new_symbols();
    scols_symbols_set_branch(sb, " ");
    scols_symbols_set_right(sb, " ");
    scols_symbols_set_vertical(sb, " ");
    scols_table_set_symbols(tb, sb);

    if (!installs_pkgs.empty()) {
        ln = scols_table_new_line(tb, NULL);
        scols_line_set_data(ln, COL_NEVRA, "Installing:");
        add_transaction_packages(tb, ln, installs_pkgs);
    }

    if (!reinstalls_pkgs.empty()) {
        ln = scols_table_new_line(tb, NULL);
        scols_line_set_data(ln, COL_NEVRA, "Reinstalling:");
        add_transaction_packages(tb, ln, reinstalls_pkgs);
    }

    if (!upgrades_pkgs.empty()) {
        ln = scols_table_new_line(tb, NULL);
        scols_line_set_data(ln, COL_NEVRA, "Upgrading:");
        add_transaction_packages(tb, ln, upgrades_pkgs);
    }

    if (!removes_pkgs.empty()) {
        ln = scols_table_new_line(tb, NULL);
        scols_line_set_data(ln, COL_NEVRA, "Removing:");
        add_transaction_packages(tb, ln, removes_pkgs);
    }

    if (!downgrades_pkgs.empty()) {
        ln = scols_table_new_line(tb, NULL);
        scols_line_set_data(ln, COL_NEVRA, "Downgrading:");
        add_transaction_packages(tb, ln, downgrades_pkgs);
    }

    scols_print_table(tb);
    scols_unref_symbols(sb);
    scols_unref_table(tb);

    std::cout << "Transaction Summary:\n";
    std::cout << fmt::format(" {:15} {:4} packages\n", "Installing:", installs_pkgs.size());
    std::cout << fmt::format(" {:15} {:4} packages\n", "Reinstalling:", reinstalls_pkgs.size());
    std::cout << fmt::format(" {:15} {:4} packages\n", "Upgrading:", upgrades_pkgs.size());
    std::cout << fmt::format(" {:15} {:4} packages\n", "Obsoleting:", obsoleded_pkgs.size());
    std::cout << fmt::format(" {:15} {:4} packages\n", "Removing:", removes_pkgs.size());
    std::cout << fmt::format(" {:15} {:4} packages\n", "Downgrading:", downgrades_pkgs.size());
    std::cout << std::endl;

    return true;
}

}  // namespace microdnf
