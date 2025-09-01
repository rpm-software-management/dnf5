// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.


#ifndef DNF5_COMMANDS_CONFIG_MANAGER_CONFIG_MANAGER_SHARED_HPP
#define DNF5_COMMANDS_CONFIG_MANAGER_CONFIG_MANAGER_SHARED_HPP

#include <libdnf5/conf/config_main.hpp>
#include <libdnf5/conf/const.hpp>
#include <libdnf5/utils/bgettext/bgettext-lib.h>
#include <libdnf5/utils/bgettext/bgettext-mark-domain.h>

#include <filesystem>

namespace dnf5 {

const std::filesystem::path CFG_MANAGER_REPOS_OVERRIDE_FILENAME = "99-config_manager.repo";

struct ConfigManagerError : public libdnf5::Error {
    using Error::Error;
    const char * get_domain_name() const noexcept override { return "dnf5"; }
    const char * get_name() const noexcept override { return "ConfigManagerError"; }
};

// Writes the warning message to the log and to stderr.
// The original message is written to the log and translated version to stderr.
template <typename... Args>
void write_warning(libdnf5::Logger & log, BgettextMessage msg, Args &&... args) {
    log.warning(b_gettextmsg_get_id(msg), args...);
    std::cerr << libdnf5::utils::sformat(TM_(msg, 1), args...) << std::endl;
}

// Checks if the `path` directory exists. If not, according to the `create_missing_dirs` argument,
// the directories (missing paths elements) are created or `ConfigManagerError` exception is thrown.
// The ConfigManagerError exception is also thrown when the path exists but is not a directory
// or a symlink to a directory.
inline void resolve_missing_dir(const std::filesystem::path & path, bool create_missing_dirs) {
    auto status = std::filesystem::status(path);  // symlinks are followed to their targets
    if (std::filesystem::exists(status)) {
        if (!std::filesystem::is_directory(status)) {
            throw ConfigManagerError(
                M_("The path \"{}\" exists, but it is not a directory or a symlink to a directory."), path.string());
        }
    } else {
        if (std::filesystem::is_symlink(path)) {
            throw ConfigManagerError(
                M_("The path \"{}\" exists, but it is a symlink to a non-existent object."), path.string());
        }
        if (create_missing_dirs) {
            std::filesystem::create_directories(path);
        } else {
            throw ConfigManagerError(
                M_("Directory \"{}\" does not exist. Add \"--create-missing-dir\" to create missing directories."),
                path.string());
        }
    }
}

inline void check_variable_name(const std::string & name) {
    if (name.find_first_not_of("ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                               "abcdefghijklmnopqrstuvwxyz"
                               "0123456789"
                               "_") != std::string::npos) {
        throw ConfigManagerError(M_("Variable name can contain only ASCII letters, numbers and '_': {}"), name);
    }
}

// Sets permissions to "rw-r--r--"
inline void set_file_permissions(const std::filesystem::path & path) {
    std::filesystem::permissions(
        path,
        std::filesystem::perms::owner_read | std::filesystem::perms::owner_write | std::filesystem::perms::group_read |
            std::filesystem::perms::others_read);
}

// Returns the actual path to the configuration file. It takes installroot into account.
// TODO(jrohel) It should be provided by libdnf.
inline std::filesystem::path get_config_file_path(const libdnf5::ConfigMain & config) {
    std::filesystem::path conf_path{config.get_config_file_path_option().get_value()};
    const auto & conf_path_priority = config.get_config_file_path_option().get_priority();
    const auto & use_host_config = config.get_use_host_config_option().get_value();
    if (!use_host_config && conf_path_priority < libdnf5::Option::Priority::COMMANDLINE) {
        conf_path = config.get_installroot_option().get_value() / conf_path.relative_path();
    }
    return conf_path;
}

// Returns the actual path to the last variable directory. It takes installroot into account.
// TODO(jrohel) Libdnf should provide paths taking into account installroot.
inline std::filesystem::path get_last_vars_dir_path(const libdnf5::ConfigMain & config) {
    std::filesystem::path vars_dir_path;

    const auto & vars_dirs = config.get_varsdir_option().get_value();
    if (vars_dirs.empty()) {
        return vars_dir_path;
    }
    vars_dir_path = vars_dirs.back();

    const auto & varsdir_path_priority = config.get_varsdir_option().get_priority();
    const auto & use_host_config = config.get_use_host_config_option().get_value();
    if (!use_host_config && varsdir_path_priority < libdnf5::Option::Priority::COMMANDLINE) {
        vars_dir_path = config.get_installroot_option().get_value() / vars_dir_path.relative_path();
    }

    return vars_dir_path;
}

// Returns the actual path to the repository configuration override directory.
// It takes into account the root of the installation.
// TODO(jrohel) It should be provided by libdnf.
inline std::filesystem::path get_repos_config_override_dir_path(const libdnf5::ConfigMain & config) {
    std::filesystem::path repos_override_dir_path{libdnf5::REPOS_OVERRIDE_DIR};
    const auto & use_host_config = config.get_use_host_config_option().get_value();
    if (use_host_config) {
        return repos_override_dir_path;
    }
    return config.get_installroot_option().get_value() / repos_override_dir_path.relative_path();
}

// Returns the actual path to the file with repositories configuration overrides.
// It takes into account the root of the installation.
inline std::filesystem::path get_config_manager_repos_override_file_path(const libdnf5::ConfigMain & config) {
    return get_repos_config_override_dir_path(config) / CFG_MANAGER_REPOS_OVERRIDE_FILENAME;
}

}  // namespace dnf5

#endif  // DNF5_COMMANDS_CONFIG_MANAGER_CONFIG_MANAGER_SHARED_HPP
