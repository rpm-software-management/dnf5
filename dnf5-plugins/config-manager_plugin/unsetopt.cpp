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

#include "unsetopt.hpp"

#include "shared.hpp"

#include <libdnf5/conf/const.hpp>
#include <libdnf5/utils/bgettext/bgettext-mark-domain.h>

#include <filesystem>

namespace dnf5 {

using namespace libdnf5;

namespace {

bool remove_from_config(ConfigParser & parser, const std::string & section_id, const std::set<std::string> & keys) {
    bool removed = false;
    for (const auto & key : keys) {
        removed |= parser.remove_option(section_id, key);
    }
    return removed;
}

}  // namespace


void ConfigManagerUnsetOptCommand::set_argument_parser() {
    auto & ctx = get_context();
    auto & parser = ctx.get_argument_parser();

    auto & cmd = *get_argument_parser_command();
    cmd.set_description("Unset/remove configuration and repositories options");

    auto opts_vals =
        parser.add_new_positional_arg("options", cli::ArgumentParser::PositionalArg::AT_LEAST_ONE, nullptr, nullptr);
    opts_vals->set_description("List of options to unset");
    opts_vals->set_parse_hook_func(
        [this, &ctx]([[maybe_unused]] cli::ArgumentParser::PositionalArg * arg, int argc, const char * const argv[]) {
            for (int i = 0; i < argc; ++i) {
                auto value = argv[i];
                std::string key{value};
                auto dot_pos = key.rfind('.');
                if (dot_pos != std::string::npos) {
                    if (dot_pos == key.size() - 1) {
                        throw cli::ArgumentParserError(
                            M_("remove-opt: Badly formatted argument value: Last key character cannot be '.': {}"),
                            std::string{value});
                    }

                    // Save the repository option for later processing (solving glob pattern, writing to file).
                    auto repo_id = key.substr(0, dot_pos);
                    if (repo_id.empty()) {
                        throw cli::ArgumentParserError(
                            M_("remove-opt: Empty repository id is not allowed: {}"), std::string{value});
                    }
                    auto repo_key = key.substr(dot_pos + 1);

                    // Test if the repository options are known.
                    try {
                        tmp_repo_conf.opt_binds().at(repo_key);
                    } catch (const OptionBindsOptionNotFoundError & ex) {
                        ctx.base.get_logger()->warning(
                            "config-manager: Request to remove unknown repository option from config file: {}", key);
                    }

                    in_repos_opts_to_remove[repo_id].insert(repo_key);
                } else {
                    // Test if the global option is known.
                    try {
                        tmp_config.opt_binds().at(key);
                    } catch (const OptionBindsOptionNotFoundError & ex) {
                        ctx.base.get_logger()->warning(
                            "config-manager: Request to remove unknown main option from config file: {}", key);
                    }

                    // Save the global option for later removing from the file.
                    main_opts_to_remove.insert(key);
                }
            }
            return true;
        });
    cmd.register_positional_arg(opts_vals);
}


void ConfigManagerUnsetOptCommand::configure() {
    auto & ctx = get_context();
    const auto & config = ctx.base.get_config();

    // Remove options from main configuration file.
    const auto & cfg_filepath = get_config_file_path(ctx.base.get_config());
    if (!main_opts_to_remove.empty() && std::filesystem::exists(cfg_filepath)) {
        ConfigParser parser;
        bool changed = false;

        parser.read(cfg_filepath);

        changed |= remove_from_config(parser, "main", main_opts_to_remove);

        if (changed) {
            parser.write(cfg_filepath, false);
        }
    }

    auto repos_override_file_path = get_config_manager_repos_override_file_path(config);

    // Remove options from repositories overrides configuration file, remove empty sections.
    if (!in_repos_opts_to_remove.empty() && std::filesystem::exists(repos_override_file_path)) {
        ConfigParser parser;
        bool changed = false;
        parser.read(repos_override_file_path);

        std::vector<std::string> empty_config_sections;
        for (const auto & [repo_id, setopts] : parser.get_data()) {
            for (const auto & [in_repoid, keys] : in_repos_opts_to_remove) {
                if (sack::match_string(repo_id, sack::QueryCmp::GLOB, in_repoid)) {
                    changed |= remove_from_config(parser, repo_id, keys);
                }
            }
            if (setopts.empty()) {
                empty_config_sections.emplace_back(repo_id);
            }
        }

        // Clean config - remove empty sections.
        for (const auto & section : empty_config_sections) {
            parser.remove_section(section);
            changed = true;
        }

        if (changed) {
            parser.write(repos_override_file_path, false);
        }
    }
}

}  // namespace dnf5
