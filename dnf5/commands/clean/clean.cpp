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


#include "clean.hpp"

#include "dnf5/shared_options.hpp"

#include <errno.h>
#include <libdnf5-cli/argument_parser.hpp>
#include <libdnf5-cli/utils/units.hpp>
#include <libdnf5/repo/repo_cache.hpp>
#include <libdnf5/utils/bgettext/bgettext-lib.h>
#include <libdnf5/utils/bgettext/bgettext-mark-domain.h>
#include <libdnf5/utils/format.hpp>

#include <filesystem>
#include <iostream>


namespace fs = std::filesystem;


namespace dnf5 {


namespace {

struct CacheType {
    std::string name;
    std::string description;
    CleanCommand::Actions action;
};

const CacheType CACHE_TYPES[]{
    {"all", "Delete all cached data from the repositories cache", CleanCommand::CLEAN_ALL},
    {"packages", "Delete packages from the repositories cache", CleanCommand::CLEAN_PACKAGES},
    {"metadata",
     "Delete the metadata and dbcache from the repositories cache",
     static_cast<CleanCommand::Actions>(CleanCommand::CLEAN_METADATA | CleanCommand::CLEAN_DBCACHE)},
    {"dbcache", "Delete dbcache from the repositories cache", CleanCommand::CLEAN_DBCACHE},
    {"expire-cache", "Mark the repositories cache as expired", CleanCommand::EXPIRE_CACHE}};

}  // namespace


using namespace libdnf5::cli;

void CleanCommand::set_parent_command() {
    auto * arg_parser_parent_cmd = get_session().get_argument_parser().get_root_command();
    auto * arg_parser_this_cmd = get_argument_parser_command();
    arg_parser_parent_cmd->register_command(arg_parser_this_cmd);
}

void CleanCommand::set_argument_parser() {
    auto & ctx = get_context();
    auto & parser = ctx.get_argument_parser();

    auto & cmd = *get_argument_parser_command();
    cmd.set_description("Remove or expire cached data");

    std::string known_types;
    bool first = true;
    for (const auto & type : CACHE_TYPES) {
        if (!first) {
            known_types.append(", ");
        }
        known_types.append(type.name);
        first = false;
    }

    auto cache_types =
        parser.add_new_positional_arg("cache_types", ArgumentParser::PositionalArg::AT_LEAST_ONE, nullptr, nullptr);
    cache_types->set_description(
        libdnf5::utils::sformat(_("List of cache types to clean up. Supported types: {0}"), known_types));

    cache_types->set_parse_hook_func(
        // Parses arguments and sets the appropriate bits in the required_actions.
        // An exception is thrown if an unknown argument is found.
        [this, known_types]([[maybe_unused]] ArgumentParser::PositionalArg * arg, int argc, const char * const argv[]) {
            for (int i = 0; i < argc; ++i) {
                const std::string_view cache_type{argv[i]};
                bool found{false};
                for (const auto & type : CACHE_TYPES) {
                    if (cache_type == type.name) {
                        required_actions = static_cast<Actions>(required_actions | type.action);
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    throw libdnf5::cli::ArgumentParserUnknownArgumentError(
                        M_("Unknown cache type \"{0}\". Supported types: {1}"), std::string(argv[i]), known_types);
                }
            }
            return true;
        });

    cache_types->set_complete_hook_func([](const char * arg) {
        // Support for action/type_cache autocomplete on the command line.
        const std::string_view to_complete{arg};
        std::string last;
        std::vector<std::string> cache_types;
        for (const auto & type : CACHE_TYPES) {
            if (type.name.compare(0, to_complete.length(), to_complete) == 0) {
                cache_types.emplace_back(fmt::format("{:15} ({})", type.name, type.description));
                last = type.name;
            }
        }
        if (cache_types.size() == 1) {
            cache_types[0] = last + " ";
        }
        return cache_types;
    });

    cmd.register_positional_arg(cache_types);
}

void CleanCommand::run() {
    auto & ctx = get_context();
    fs::path cachedir{ctx.get_base().get_config().get_cachedir_option().get_value()};

    std::error_code ec;
    libdnf5::repo::RepoCache::RemoveStatistics statistics{};
    for (const auto & dir_entry : std::filesystem::directory_iterator(cachedir, ec)) {
        if (!dir_entry.is_directory()) {
            continue;
        }
        libdnf5::repo::RepoCache cache(ctx.get_base().get_weak_ptr(), dir_entry.path());

        try {
            if (required_actions & CLEAN_ALL) {
                statistics += cache.remove_all();
                continue;
            }
            if (required_actions & CLEAN_METADATA) {
                statistics += cache.remove_metadata();
            }
            if (required_actions & CLEAN_PACKAGES) {
                statistics += cache.remove_packages();
            }
            if (required_actions & CLEAN_DBCACHE) {
                statistics += cache.remove_solv_files();
            }
            if (required_actions & EXPIRE_CACHE) {
                cache.write_attribute(libdnf5::repo::RepoCache::ATTRIBUTE_EXPIRED);
            }
        } catch (const std::exception & ex) {
            std::cerr << libdnf5::utils::sformat(
                             _("Failed to cleanup repository cache in path \"{0}\": {1}"),
                             dir_entry.path().native(),
                             ex.what())
                      << std::endl;
        }
    }

    if (ec) {
        if (ec.value() == ENOENT) {
            std::cout << libdnf5::utils::sformat(
                             _("Cache directory \"{}\" does not exist. Nothing to clean."), cachedir.string())
                      << std::endl;
            return;
        }
        throw std::runtime_error(
            libdnf5::utils::sformat(_("Cannot iterate cache directory \"{}\": {}"), cachedir.string(), ec.message()));
    }

    const auto [bytes_removed_value, bytes_removed_unit] =
        libdnf5::cli::utils::units::to_size(static_cast<int64_t>(statistics.get_bytes_removed()));
    std::cout << fmt::format(
                     "Removed {} files, {} directories (total of {:.0f} {:s}). {} errors occurred.",
                     statistics.get_files_removed(),
                     statistics.get_dirs_removed(),
                     bytes_removed_value,
                     bytes_removed_unit,
                     statistics.get_errors())
              << std::endl;
}


}  // namespace dnf5
