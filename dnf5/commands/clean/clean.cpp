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


#include "clean.hpp"

#include <libdnf/repo/repo_cache.hpp>

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


using namespace libdnf::cli;

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

    auto cache_types =
        parser.add_new_positional_arg("cache_types", ArgumentParser::PositionalArg::AT_LEAST_ONE, nullptr, nullptr);
    cache_types->set_description("List of cache types to clean up");

    cache_types->set_parse_hook_func(
        // Parses arguments and sets the appropriate bits in the required_actions.
        // An exception is thrown if an unknown argument is found.
        [this]([[maybe_unused]] ArgumentParser::PositionalArg * arg, int argc, const char * const argv[]) {
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
                    throw std::runtime_error(fmt::format("Unknown cache type \"{}\"", argv[i]));
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
    fs::path cachedir{ctx.base.get_config().get_cachedir_option().get_value()};

    std::error_code ec;
    libdnf::repo::RepoCache::RemoveStatistics statistics{};
    for (const auto & dir_entry : std::filesystem::directory_iterator(cachedir, ec)) {
        libdnf::repo::RepoCache cache(ctx.base.get_weak_ptr(), dir_entry.path());

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
            cache.write_attribute(libdnf::repo::RepoCache::ATTRIBUTE_EXPIRED);
        }
    }

    if (ec) {
        throw std::runtime_error(fmt::format("Cannot iterate the cache directory: \"{}\"", cachedir.string()));
    }

    std::cout << fmt::format(
                     "Removed {} files, {} directories. {} errors occurred.",
                     statistics.files_removed,
                     statistics.dirs_removed,
                     statistics.errors)
              << std::endl;
}


}  // namespace dnf5
