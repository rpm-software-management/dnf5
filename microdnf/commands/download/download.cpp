/*
Copyright (C) 2020 Red Hat, Inc.

This file is part of microdnf: https://github.com/rpm-software-management/libdnf/

Microdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Microdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with microdnf.  If not, see <https://www.gnu.org/licenses/>.
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

void CmdDownload::set_argument_parser(Context & ctx) {
    patterns_to_download_options = ctx.arg_parser.add_new_values();
    auto keys = ctx.arg_parser.add_new_positional_arg(
        "keys_to_match",
        ArgumentParser::PositionalArg::UNLIMITED,
        ctx.arg_parser.add_init_value(std::unique_ptr<libdnf::Option>(new libdnf::OptionString(nullptr))),
        patterns_to_download_options);
    keys->set_short_description("List of keys to match");

    auto download = ctx.arg_parser.add_new_command("download");
    download->set_short_description("download packages to current directory");
    download->set_description("");
    download->set_named_args_help_header("Optional arguments:");
    download->set_positional_args_help_header("Positional arguments:");
    download->set_parse_hook_func([this, &ctx](
                               [[maybe_unused]] ArgumentParser::Argument * arg,
                               [[maybe_unused]] const char * option,
                               [[maybe_unused]] int argc,
                               [[maybe_unused]] const char * const argv[]) {
        ctx.select_command(this);
        return true;
    });

    download->register_positional_arg(keys);

    ctx.arg_parser.get_root_command()->register_command(download);
}

void CmdDownload::configure([[maybe_unused]] Context & ctx) {}

void CmdDownload::run(Context & ctx) {
    auto package_sack = ctx.base.get_rpm_package_sack();

    // To search in available repositories (available packages)
    libdnf::repo::RepoQuery enabled_repos(ctx.base.get_rpm_repo_sack());
    enabled_repos.filter_enabled(true);
    using LoadFlags = libdnf::rpm::PackageSack::LoadRepoFlags;
    auto flags = LoadFlags::USE_FILELISTS | LoadFlags::USE_PRESTO | LoadFlags::USE_UPDATEINFO | LoadFlags::USE_OTHER;
    ctx.load_rpm_repos(enabled_repos, flags);

    std::cout << std::endl;

    libdnf::rpm::PackageSet result_pset(package_sack);
    libdnf::rpm::PackageQuery full_package_query(package_sack);
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
