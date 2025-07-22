/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/


#ifndef DNF5_COMMANDS_CONFIG_MANAGER_CONFIG_MANAGER_ADDREPO_HPP
#define DNF5_COMMANDS_CONFIG_MANAGER_CONFIG_MANAGER_ADDREPO_HPP

#include <dnf5/context.hpp>

#include <filesystem>
#include <map>
#include <string>
#include <vector>

namespace dnf5 {

class ConfigManagerAddRepoCommand : public Command {
public:
    explicit ConfigManagerAddRepoCommand(Context & context) : Command(context, "addrepo") {}
    void set_argument_parser() override;
    void configure() override;

private:
    // Defines what happens when the destination repository configuration file already exists.
    enum class FilePolicy {
        ERROR,          // Throw an error
        OVERWRITE,      // Allow overwriting of existing repository configuration file
        ADD_OR_REPLACE  // Allow adding or replacing a repository in the existing configuration file
    };

    struct SourceRepofile {
        std::string location;
        bool is_local_path;
    };

    /// Copies/downloads the repository configuration file.
    /// Tests the content. And if it's ok, it saves it in `dest_repo_dir`.
    /// @param source_repofile  Location of the source repositories configuration file (URL or local file path).
    /// @param dest_repo_dir  The path to the directory where the configuration file will be stored.
    void add_repos_from_repofile(const SourceRepofile & source_repofile, const std::filesystem::path & dest_repo_dir);

    /// Creates a new repository configuration from repository options.
    /// @param repo_id  The ID of the new repository, if it is empty, is automatically generated from baseurl, mirrorlist, metalink.
    /// @param repo_opts  Options for the new repository.
    /// @param dest_repo_dir  The path to the directory where the configuration file will be stored.
    void create_repo(
        std::string repo_id,
        const std::map<std::string, std::string> & repo_opts,
        const std::filesystem::path & dest_repo_dir);

    /// Tests if the file does not exist.
    /// @param path  Path to check.
    /// @throws ConfigManagerError  Trown if `path` already exist and overwriting is not allowed.
    void test_if_filepath_not_exist(const std::filesystem::path & path, bool show_hint_add_or_replace) const;

    /// Tests if the repositories IDs in the vector do not already exist in the configuration.
    /// @param repo_ids  List of repositories IDs to check.
    /// @param ignore_path  The file in this path will be ignored/skipped.
    /// @throws ConfigManagerError  Trown if an already existent repository ID was found.
    void test_if_ids_not_already_exist(
        const std::vector<std::string> & repo_ids, const std::filesystem::path & ignore_path) const;

    libdnf5::ConfigMain tmp_config;
    libdnf5::repo::ConfigRepo tmp_repo_conf{tmp_config, "temporary_to_check_repository_options"};

    SourceRepofile source_repofile;   // Location of source repository configuration file.
    std::string repo_id;              // The user-defined ID of the newly created repository.
    bool create_missing_dirs{false};  // Allows one to create missing directories.
    FilePolicy file_policy{FilePolicy::ERROR};
    std::string save_filename;                     // User-defined name of newly saved configuration file.
    std::map<std::string, std::string> repo_opts;  // Options for the new repository.
};

}  // namespace dnf5

#endif  // DNF5_COMMANDS_CONFIG_MANAGER_CONFIG_MANAGER_ADDREPO_HPP
