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

#include "../../context.hpp"

#include <libdnf/conf/option_string.hpp>
#include <libdnf/rpm/package.hpp>
#include <libdnf/rpm/package_query.hpp>
#include <libdnf/rpm/package_set.hpp>
#include <libdnf/rpm/transaction.hpp>

#include <filesystem>
#include <iostream>


namespace fs = std::filesystem;


namespace microdnf {


using namespace libdnf::cli;


DownloadCommand::DownloadCommand(Command & parent) : Command(parent, "download") {
    auto & ctx = static_cast<Context &>(get_session());
    auto & parser = ctx.get_argument_parser();
    auto & cmd = *get_argument_parser_command();

    patterns_to_download_options = parser.add_new_values();
    auto keys = parser.add_new_positional_arg(
        "keys_to_match",
        ArgumentParser::PositionalArg::UNLIMITED,
        parser.add_init_value(std::unique_ptr<libdnf::Option>(new libdnf::OptionString(nullptr))),
        patterns_to_download_options);
    keys->set_short_description("List of keys to match");

    cmd.set_short_description("Download software to the current directory");
    cmd.set_parse_hook_func([this, &ctx](
                               [[maybe_unused]] ArgumentParser::Argument * arg,
                               [[maybe_unused]] const char * option,
                               [[maybe_unused]] int argc,
                               [[maybe_unused]] const char * const argv[]) {
        ctx.set_selected_command(this);
        return true;
    });

    cmd.register_positional_arg(keys);
}


void DownloadCommand::run() {
    auto & ctx = static_cast<Context &>(get_session());
    auto package_sack = ctx.base.get_rpm_package_sack();

    // To search in available repositories (available packages)
    libdnf::repo::RepoQuery enabled_repos(ctx.base);
    enabled_repos.filter_enabled(true);
    ctx.load_rpm_repos(enabled_repos);

    std::cout << std::endl;

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


}  // namespace microdnf
