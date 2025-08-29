// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later


#ifndef DNF5_COMMANDS_CONFIG_MANAGER_SETOPT_HPP
#define DNF5_COMMANDS_CONFIG_MANAGER_SETOPT_HPP

#include <dnf5/context.hpp>

#include <map>
#include <set>
#include <string>

namespace dnf5 {

class ConfigManagerSetOptCommand : public Command {
public:
    explicit ConfigManagerSetOptCommand(Context & context) : Command(context, "setopt") {}
    void set_argument_parser() override;
    void configure() override;

private:
    std::set<std::string> load_existing_repo_ids() const;

    libdnf5::ConfigMain tmp_config;
    libdnf5::repo::ConfigRepo tmp_repo_conf{tmp_config, "temporary_to_check_repository_options"};
    std::map<std::string, std::string> main_setopts;
    std::map<std::string, std::map<std::string, std::string>> in_repos_setopts;
    std::map<std::string, std::map<std::string, std::string>> matching_repos_setopts;
    bool create_missing_dirs{false};  // Allows one to create missing directories.
};

}  // namespace dnf5

#endif  // DNF5_COMMANDS_CONFIG_MANAGER_SETOPT_HPP
