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

#include "dnf5/shared_options.hpp"
#include "utils/bgettext/bgettext-mark-domain.h"

#include <libdnf/conf/option_string.hpp>
#include <libdnf/repo/package_downloader.hpp>
#include <libdnf/rpm/package.hpp>
#include <libdnf/rpm/package_query.hpp>
#include <libdnf/rpm/package_set.hpp>

#include <iostream>
#include <map>


namespace dnf5 {

using namespace libdnf::cli;

void DownloadCommand::set_parent_command() {
    auto * arg_parser_parent_cmd = get_session().get_argument_parser().get_root_command();
    auto * arg_parser_this_cmd = get_argument_parser_command();
    arg_parser_parent_cmd->register_command(arg_parser_this_cmd);
}

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

    resolve_option = dynamic_cast<libdnf::OptionBool *>(
        parser.add_init_value(std::unique_ptr<libdnf::OptionBool>(new libdnf::OptionBool(false))));

    alldeps_option = dynamic_cast<libdnf::OptionBool *>(
        parser.add_init_value(std::unique_ptr<libdnf::OptionBool>(new libdnf::OptionBool(false))));

    auto resolve = parser.add_new_named_arg("resolve");
    resolve->set_long_name("resolve");
    resolve->set_description("Resolve and download needed dependencies");
    resolve->set_const_value("true");
    resolve->link_value(resolve_option);

    auto alldeps = parser.add_new_named_arg("alldeps");
    alldeps->set_long_name("alldeps");
    alldeps->set_description(
        "When running with --resolve, download all dependencies (do not exclude already installed ones)");
    alldeps->set_const_value("true");
    alldeps->link_value(alldeps_option);

    cmd.register_named_arg(alldeps);
    create_destdir_option(*this);
    cmd.register_named_arg(resolve);
    cmd.register_positional_arg(keys);
}

void DownloadCommand::configure() {
    auto & context = get_context();

    std::vector<std::string> pkg_specs;
    for (auto & pattern : *patterns_to_download_options) {
        auto option = dynamic_cast<libdnf::OptionString *>(pattern.get());
        pkg_specs.push_back(option->get_value());
    }

    context.update_repo_metadata_from_specs(pkg_specs);
    if (resolve_option->get_value() && !alldeps_option->get_value()) {
        context.set_load_system_repo(true);
    } else if (!resolve_option->get_value() && alldeps_option->get_value()) {
        throw libdnf::cli::ArgumentParserMissingDependentArgumentError(
            //TODO(jrohel): Add support for requiring an argument by another argument in ArgumentParser?
            M_("Option \"--alldeps\" should be used with \"--resolve\""));
    } else {
        context.set_load_system_repo(false);
    }
    context.set_load_available_repos(Context::LoadAvailableRepos::ENABLED);
    // Default destination for downloaded rpms is the current directory
    context.base.get_config().get_destdir_option().set(libdnf::Option::Priority::PLUGINDEFAULT, ".");
}

void DownloadCommand::run() {
    auto & ctx = get_context();

    auto create_nevra_pkg_pair = [](const libdnf::rpm::Package & pkg) { return std::make_pair(pkg.get_nevra(), pkg); };

    std::map<std::string, libdnf::rpm::Package> download_pkgs;
    libdnf::rpm::PackageQuery full_pkg_query(ctx.base);
    for (auto & pattern : *patterns_to_download_options) {
        libdnf::rpm::PackageQuery pkg_query(full_pkg_query);
        auto option = dynamic_cast<libdnf::OptionString *>(pattern.get());
        pkg_query.resolve_pkg_spec(option->get_value(), {}, true);
        pkg_query.filter_available();
        pkg_query.filter_priority();
        pkg_query.filter_latest_evr();

        for (const auto & pkg : pkg_query) {
            download_pkgs.insert(create_nevra_pkg_pair(pkg));

            if (resolve_option->get_value()) {
                auto goal = std::make_unique<libdnf::Goal>(ctx.base);
                goal->add_rpm_install(pkg, {});

                auto transaction = goal->resolve();
                auto transaction_problems = transaction.get_problems();
                if (transaction_problems != libdnf::GoalProblem::NO_PROBLEM) {
                    if (transaction_problems != libdnf::GoalProblem::NOT_FOUND) {
                        throw GoalResolveError(transaction);
                    }
                }
                for (auto & tspkg : transaction.get_transaction_packages()) {
                    if (transaction_item_action_is_inbound(tspkg.get_action()) &&
                        tspkg.get_package().get_repo()->get_type() != libdnf::repo::Repo::Type::COMMANDLINE) {
                        download_pkgs.insert(create_nevra_pkg_pair(tspkg.get_package()));
                    }
                }
            }
        }
    }

    if (!download_pkgs.empty()) {
        libdnf::repo::PackageDownloader downloader(ctx.base);

        // for download command, we don't want to mark the packages for removal
        downloader.force_keep_packages(true);

        for (auto & [nevra, pkg] : download_pkgs) {
            downloader.add(pkg);
        }

        std::cout << "Downloading Packages:" << std::endl;
        downloader.download(true, true);
        std::cout << std::endl;
    }
}

}  // namespace dnf5
