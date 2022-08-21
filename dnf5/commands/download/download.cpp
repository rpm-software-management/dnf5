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

#include "download.hpp"

#include <libdnf/conf/option_string.hpp>
#include <libdnf/rpm/package.hpp>
#include <libdnf/rpm/package_query.hpp>
#include <libdnf/rpm/package_set.hpp>

namespace dnf5 {

using namespace libdnf::cli;

void DownloadCommand::set_argument_parser() {
    auto & ctx = get_context();
    auto & parser = ctx.get_argument_parser();

    auto & cmd = *get_argument_parser_command();
    cmd.set_description("Download software to the current directory");

    patterns_to_download_options = parser.add_new_values();
    auto keys = parser.add_new_positional_arg(
        "keys_to_match",
        ArgumentParser::PositionalArg::UNLIMITED,
        parser.add_init_value(std::unique_ptr<libdnf::Option>(new libdnf::OptionString(nullptr))),
        patterns_to_download_options);
    keys->set_description("List of keys to match");
    keys->set_complete_hook_func([&ctx](const char * arg) { return match_specs(ctx, arg, false, true, false, false); });
    cmd.register_positional_arg(keys);
}

void DownloadCommand::configure() {
    auto & context = get_context();

    std::vector<std::string> pkg_specs;
    for (auto & pattern : *patterns_to_download_options) {
        auto option = dynamic_cast<libdnf::OptionString *>(pattern.get());
        pkg_specs.push_back(option->get_value());
    }

    context.update_repo_load_flags_from_specs(pkg_specs);
    context.set_load_system_repo(false);
    context.set_load_available_repos(Context::LoadAvailableRepos::ENABLED);
}

void DownloadCommand::run() {
    auto & ctx = get_context();

    libdnf::rpm::PackageSet result_pset(ctx.base);
    libdnf::rpm::PackageQuery full_package_query(ctx.base);
    for (auto & pattern : *patterns_to_download_options) {
        libdnf::rpm::PackageQuery package_query(full_package_query);
        auto option = dynamic_cast<libdnf::OptionString *>(pattern.get());
        package_query.resolve_pkg_spec(option->get_value(), {}, true);
        result_pset |= package_query;
    }

    std::vector<libdnf::rpm::Package> download_pkgs;
    download_pkgs.reserve(result_pset.size());
    for (auto package : result_pset) {
        download_pkgs.push_back(std::move(package));
    }
    download_packages(download_pkgs, ".");
}

}  // namespace dnf5
