// Copyright Contributors to the DNF5 project
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "repos.hpp"

#include <map>

namespace dnf5 {

using namespace libdnf5;

void RepoCommand::set_argument_parser() {
    auto & ctx = get_context();
    auto & parser = ctx.get_argument_parser();

    auto & cmd = *get_argument_parser_command();
    cmd.set_description(get_command_description());

    auto opts_repos =
        parser.add_new_positional_arg("repos", cli::ArgumentParser::PositionalArg::AT_LEAST_ONE, nullptr, nullptr);
    opts_repos->set_description("List of repos to be adjusted.");
    opts_repos->set_parse_hook_func(
        [this]([[maybe_unused]] cli::ArgumentParser::PositionalArg * arg, int argc, const char * const argv[]) {
            for (int i = 0; i < argc; ++i) {
                repos.emplace(argv[i]);
            }
            return true;
        });
    cmd.register_positional_arg(opts_repos);

    auto create_missing_dirs_opt = parser.add_new_named_arg("create-missing-dir");
    create_missing_dirs_opt->set_long_name("create-missing-dir");
    create_missing_dirs_opt->set_description("Allow to create missing directories");
    create_missing_dirs_opt->set_has_value(false);
    create_missing_dirs_opt->set_parse_hook_func([this](cli::ArgumentParser::NamedArg *, const char *, const char *) {
        create_missing_dirs = true;
        return true;
    });
    cmd.register_named_arg(create_missing_dirs_opt);
}

void RepoCommand::configure() {
    auto & ctx = get_context();
    const auto & config = ctx.get_base().get_config();

    auto repo_ids = load_existing_repo_ids(ctx);

    // Expand the repo ID patterns from the argv and construct the final set
    // of repo IDs to be enabled/disabled. Fail for invalid patterns.
    std::set<std::string> repos_to_adjust;
    for (auto && repo_id_pattern : repos) {
        auto filtered_repo_ids = dnf5::filter_repo_ids(repo_id_pattern, repo_ids);
        if (filtered_repo_ids.empty()) {
            throw ConfigManagerError(M_("No matching repository to modify: {}"), repo_id_pattern);
        }
        repos_to_adjust.merge(filtered_repo_ids);
    }

    // Write out the override config for enabling/disabling
    ConfigParser parser;

    resolve_missing_dir(get_repos_config_override_dir_path(config), create_missing_dirs);

    auto repos_override_file_path = get_config_manager_repos_override_file_path(config);

    const bool exists = std::filesystem::exists(repos_override_file_path);
    if (exists) {
        parser.read(repos_override_file_path);
    }

    parser.get_header() = CFG_MANAGER_REPOS_OVERRIDE_CFG_HEADER;

    std::map<std::string, std::string> opts{{"enabled", enabled_value}};
    for (auto && repo_id : repos_to_adjust) {
        modify_config(parser, repo_id, opts);
    }

    parser.write(repos_override_file_path, false);
    if (!exists) {
        set_file_permissions(repos_override_file_path);
    }
}

}  // namespace dnf5
