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

bool remove_from_config(
    ConfigParser & parser,
    const std::string & section_id,
    const std::set<std::string> & keys,
    std::set<std::string> & used_keys) {
    bool removed = false;
    for (const auto & key : keys) {
        if (parser.remove_option(section_id, key)) {
            removed = true;
            used_keys.insert(key);
        }
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
                            M_("{}: Badly formatted argument value: Last key character cannot be '.': {}"),
                            std::string{"remove-opt"},
                            std::string{value});
                    }

                    // Save the repository option for later processing (solving glob pattern, writing to file).
                    auto repo_id = key.substr(0, dot_pos);
                    if (repo_id.empty()) {
                        throw cli::ArgumentParserError(
                            M_("{}: Empty repository id is not allowed: {}"),
                            std::string{"remove-opt"},
                            std::string{value});
                    }
                    auto repo_key = key.substr(dot_pos + 1);

                    // Test if the repository options are known.
                    try {
                        tmp_repo_conf.opt_binds().at(repo_key);
                    } catch (const OptionBindsOptionNotFoundError & ex) {
                        write_warning(
                            *ctx.get_base().get_logger(),
                            M_("config-manager: Request to remove unsupported repository option: {}"),
                            key);
                    }

                    in_repos_opts_to_remove[repo_id].insert(repo_key);
                } else {
                    // Test if the global option is known.
                    try {
                        tmp_config.opt_binds().at(key);
                    } catch (const OptionBindsOptionNotFoundError & ex) {
                        write_warning(
                            *ctx.get_base().get_logger(),
                            M_("config-manager: Request to remove unsupported main option: {}"),
                            key);
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
    const auto & config = ctx.get_base().get_config();

    // Remove options from main configuration file.
    if (!main_opts_to_remove.empty()) {
        const auto & cfg_filepath = get_config_file_path(ctx.get_base().get_config());
        if (std::filesystem::exists(cfg_filepath)) {
            ConfigParser parser;
            bool changed = false;
            std::set<std::string> used_keys;

            parser.read(cfg_filepath);

            changed |= remove_from_config(parser, "main", main_opts_to_remove, used_keys);

            // Generate warning for unused options
            for (const auto & key : main_opts_to_remove) {
                if (!used_keys.contains(key)) {
                    write_warning(
                        *ctx.get_base().get_logger(),
                        M_("config-manager: Request to remove main option but it is not present in the config file: "
                           "{}"),
                        key);
                }
            }

            if (changed) {
                parser.write(cfg_filepath, false);
            }
        } else {
            write_warning(
                *ctx.get_base().get_logger(),
                M_("config-manager: Request to remove main option but config file not found: {}"),
                cfg_filepath.string());
        }
    }

    // Remove options from repositories overrides configuration file, remove empty sections.
    if (!in_repos_opts_to_remove.empty()) {
        const auto & repos_override_file_path = get_config_manager_repos_override_file_path(config);
        if (std::filesystem::exists(repos_override_file_path)) {
            ConfigParser parser;
            bool changed = false;
            std::map<std::string, std::set<std::string>> used_repos_opts;

            parser.read(repos_override_file_path);

            std::vector<std::string> empty_config_sections;
            for (const auto & [repo_id, setopts] : parser.get_data()) {
                for (const auto & [in_repoid, keys] : in_repos_opts_to_remove) {
                    if (sack::match_string(repo_id, sack::QueryCmp::GLOB, in_repoid)) {
                        auto & used_repoid_opts = used_repos_opts[in_repoid];
                        changed |= remove_from_config(parser, repo_id, keys, used_repoid_opts);
                    }
                }
                if (setopts.empty()) {
                    empty_config_sections.emplace_back(repo_id);
                }
            }

            // Generate warning for unused repoids and options
            for (const auto & [in_repoid, keys] : in_repos_opts_to_remove) {
                if (const auto used_repoid_opts = used_repos_opts.find(in_repoid);
                    used_repoid_opts == used_repos_opts.end()) {
                    write_warning(
                        *ctx.get_base().get_logger(),
                        M_("config-manager: Request to remove repository option but repoid is not present "
                           "in the overrides: {}"),
                        in_repoid);
                } else {
                    for (const auto & key : keys) {
                        if (!used_repoid_opts->second.contains(key))
                            write_warning(
                                *ctx.get_base().get_logger(),
                                M_("config-manager: Request to remove repository option but it is not present "
                                   "in the overrides: {}.{}"),
                                in_repoid,
                                key);
                    }
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
        } else {
            write_warning(
                *ctx.get_base().get_logger(),
                M_("config-manager: Request to remove repository option but file with overrides not found: {}"),
                repos_override_file_path.string());
        }
    }
}

}  // namespace dnf5
