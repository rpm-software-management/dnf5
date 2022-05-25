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

#include "changelog.hpp"

#include <libdnf/conf/option_string.hpp>
#include <libdnf/rpm/package.hpp>
#include <libdnf/rpm/package_query.hpp>

#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace dnf5 {

using namespace libdnf::cli;

void ChangelogCommand::set_argument_parser() {
    auto & ctx = get_context();
    auto & parser = ctx.get_argument_parser();

    auto & cmd = *get_argument_parser_command();
    cmd.set_short_description("Show package changelogs");

    since_option = dynamic_cast<libdnf::OptionNumber<std::int64_t> *>(
        parser.add_init_value(std::unique_ptr<libdnf::OptionNumber<std::int64_t>>(
            new libdnf::OptionNumber<int64_t>(0, [](const std::string & value) {
                struct tm time_m;
                std::istringstream ss(value);
                ss >> std::get_time(&time_m, "%Y-%m-%d");
                if (ss.fail()) {
                    throw std::runtime_error(fmt::format("Invalid date: {}", value));
                }
                return static_cast<int64_t>(mktime(&time_m));
            }))));

    count_option = dynamic_cast<libdnf::OptionNumber<std::int32_t> *>(
        parser.add_init_value(std::unique_ptr<libdnf::OptionNumber<std::int32_t>>(new libdnf::OptionNumber<int>(0))));

    upgrades_option = dynamic_cast<libdnf::OptionBool *>(
        parser.add_init_value(std::unique_ptr<libdnf::OptionBool>(new libdnf::OptionBool(false))));

    auto since = parser.add_new_named_arg("since");
    since->set_long_name("since");
    since->set_short_description("Show changelog entries since date in the YYYY-MM-DD format");
    since->set_has_value(true);
    since->link_value(since_option);

    auto count = parser.add_new_named_arg("count");
    count->set_long_name("count");
    count->set_short_description("Limit the number of changelog entries shown per package");
    count->set_has_value(true);
    count->link_value(count_option);

    auto upgrades = parser.add_new_named_arg("upgrades");
    upgrades->set_long_name("upgrades");
    upgrades->set_short_description(
        "Show new changelog entries for packages that provide an upgrade for an already installed package");
    upgrades->set_const_value("true");
    upgrades->link_value(upgrades_option);

    pkgs_spec_to_show_options = parser.add_new_values();
    auto keys = parser.add_new_positional_arg(
        "pkg_spec",
        ArgumentParser::PositionalArg::UNLIMITED,
        parser.add_init_value(std::unique_ptr<libdnf::Option>(new libdnf::OptionString(nullptr))),
        pkgs_spec_to_show_options);
    keys->set_short_description("List of packages specifiers");
    keys->set_complete_hook_func([&ctx](const char * arg) { return match_specs(ctx, arg, false, true, false, false); });

    auto conflict_args = parser.add_conflict_args_group(std::unique_ptr<std::vector<ArgumentParser::Argument *>>(
        new std::vector<ArgumentParser::Argument *>{since, count, upgrades}));

    since->set_conflict_arguments(conflict_args);
    count->set_conflict_arguments(conflict_args);
    upgrades->set_conflict_arguments(conflict_args);

    cmd.register_named_arg(since);
    cmd.register_named_arg(count);
    cmd.register_named_arg(upgrades);
    cmd.register_positional_arg(keys);
}

void ChangelogCommand::configure() {
    auto & context = get_context();
    context.set_load_system_repo(true);
    context.set_load_available_repos(Context::LoadAvailableRepos::ENABLED);
    context.set_available_repos_load_flags(libdnf::repo::Repo::LoadFlags::ALL);
}

void ChangelogCommand::run() {
    auto & ctx = get_context();

    auto since = since_option->get_value();
    auto count = count_option->get_value();
    auto upgrades = upgrades_option->get_value();

    if (since > 0) {
        std::cout << "Listing changelogs since " << std::put_time(std::localtime(&since), "%c") << std::endl;
    } else if (count != 0) {
        std::cout << "Listing only latest changelogs" << std::endl;
    } else if (upgrades) {
        std::cout << "Listing only new changelogs since installed version of the package" << std::endl;
    } else {
        std::cout << "Listing all changelogs" << std::endl;
    }

    //query
    libdnf::rpm::PackageQuery query(ctx.base, libdnf::sack::ExcludeFlags::APPLY_EXCLUDES, true);
    libdnf::rpm::PackageQuery full_package_query(ctx.base, libdnf::sack::ExcludeFlags::APPLY_EXCLUDES, false);
    libdnf::ResolveSpecSettings settings{
        .ignore_case = true, .with_nevra = true, .with_provides = false, .with_filenames = false};
    if (pkgs_spec_to_show_options->size() > 0) {
        for (auto & pattern : *pkgs_spec_to_show_options) {
            libdnf::rpm::PackageQuery package_query(full_package_query);
            auto option = dynamic_cast<libdnf::OptionString *>(pattern.get());
            package_query.resolve_pkg_spec(option->get_value(), settings, true);
            package_query.filter_latest_evr();
            query |= package_query;
        }
    } else {
        query = full_package_query;
    }
    if (upgrades) {
        query.filter_upgrades();
    } else {
        query.filter_available();
    }


    // by_srpm
    std::map<std::string, std::vector<libdnf::rpm::Package>> by_srpm;
    for (auto pkg : query) {
        by_srpm[pkg.get_source_name() + '/' + pkg.get_evr()].push_back(pkg);
    }

    libdnf::rpm::PackageQuery installed(full_package_query);
    installed.filter_installed();
    for (auto & [name, packages] : by_srpm) {
        // Print header
        std::cout << "Changelogs for ";
        std::vector<std::string> nevras;
        nevras.reserve(packages.size());
        for (auto & pkg : packages) {
            nevras.push_back(pkg.get_nevra());
        }
        std::sort(nevras.begin(), nevras.end());
        std::cout << nevras[0];
        for (size_t idx = 1; idx < nevras.size(); ++idx) {
            std::cout << ", " << nevras[idx];
        }
        std::cout << std::endl;

        auto changelogs = packages[0].get_changelogs();
        std::sort(
            changelogs.begin(),
            changelogs.end(),
            [](const libdnf::rpm::Changelog & a, const libdnf::rpm::Changelog & b) {
                return a.timestamp > b.timestamp;
            });

        // filter changelog
        if (upgrades) {
            libdnf::rpm::PackageQuery query(installed);
            query.filter_name({packages[0].get_name()});
            time_t newest_timestamp = 0;
            for (auto pkg : query) {
                for (auto & chlog : pkg.get_changelogs()) {
                    if (chlog.timestamp > newest_timestamp) {
                        newest_timestamp = chlog.timestamp;
                    }
                }
            }
            size_t idx;
            for (idx = 0; idx < changelogs.size() && changelogs[idx].timestamp > newest_timestamp; ++idx) {
            }
            changelogs.erase(changelogs.begin() + static_cast<int>(idx), changelogs.end());
        } else if (count != 0) {
            if (count > 0) {
                if (static_cast<size_t>(count) < changelogs.size()) {
                    changelogs.erase(changelogs.begin() + count, changelogs.end());
                }
            } else {
                if (static_cast<size_t>(-count) < changelogs.size()) {
                    changelogs.erase(changelogs.end() + count, changelogs.end());
                } else {
                    changelogs.clear();
                }
            }
        } else if (since > 0) {
            size_t idx;
            for (idx = 0; idx < changelogs.size() && changelogs[idx].timestamp >= since; ++idx) {
            }
            changelogs.erase(changelogs.begin() + static_cast<int>(idx), changelogs.end());
        }

        for (auto & chlog : changelogs) {
            std::cout << std::put_time(std::gmtime(&chlog.timestamp), "* %a %b %d %X %Y ");
            std::cout << chlog.author << "\n";
            std::cout << chlog.text << "\n" << std::endl;
        }
    }
}

}  // namespace dnf5
