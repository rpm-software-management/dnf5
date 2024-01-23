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

#include <libdnf5/conf/option_string.hpp>
#include <libdnf5/repo/package_downloader.hpp>
#include <libdnf5/rpm/package.hpp>
#include <libdnf5/rpm/package_query.hpp>
#include <libdnf5/rpm/package_set.hpp>
#include <libdnf5/utils/bgettext/bgettext-mark-domain.h>

#include <algorithm>
#include <format>
#include <iostream>
#include <map>


namespace dnf5 {

using namespace libdnf5::cli;

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
        parser.add_init_value(std::unique_ptr<libdnf5::Option>(new libdnf5::OptionString(nullptr))),
        patterns_to_download_options);
    keys->set_description("List of keys to match");
    keys->set_complete_hook_func([&ctx](const char * arg) { return match_specs(ctx, arg, false, true, false, false); });

    resolve_option = dynamic_cast<libdnf5::OptionBool *>(
        parser.add_init_value(std::unique_ptr<libdnf5::OptionBool>(new libdnf5::OptionBool(false))));

    alldeps_option = dynamic_cast<libdnf5::OptionBool *>(
        parser.add_init_value(std::unique_ptr<libdnf5::OptionBool>(new libdnf5::OptionBool(false))));

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

<<<<<<< HEAD
=======
    auto url = parser.add_new_named_arg("url");
    url->set_long_name("url");
    url->set_description("Print the list of urls where the rpms can be downloaded instead of downloading");
    url->set_const_value("true");
    url->link_value(url_option);


    urlprotocol_valid_options = {"http", "https", "rsync", "ftp"};
    urlprotocol_option = {};
    auto urlprotocol = parser.add_new_named_arg("urlprotocol");
    urlprotocol->set_long_name("urlprotocol");
    urlprotocol->set_description("When running with --url, limit to specific protocols");
    urlprotocol->set_parse_hook_func(
        [&ctx, this](
            [[maybe_unused]] ArgumentParser::NamedArg * arg, [[maybe_unused]] const char * option, const char * value) {
            if (urlprotocol_valid_options.find(value) == urlprotocol_valid_options.end()) {
                throw libdnf5::cli::ArgumentParserInvalidValueError(M_("Invalid urlprotocol option: {}"), value)
            }
            urlprotocol_option.emplace_back(value);
        });

>>>>>>> 31411c22 (Added urlprotocol)
    cmd.register_named_arg(alldeps);
    create_destdir_option(*this);
    cmd.register_named_arg(resolve);
    cmd.register_positional_arg(keys);
<<<<<<< HEAD
=======
    cmd.register_named_arg(url);
    cmd.register_named_arg(urlprotocol);
>>>>>>> 31411c22 (Added urlprotocol)
}

void DownloadCommand::configure() {
    auto & context = get_context();

    std::vector<std::string> pkg_specs;
    for (auto & pattern : *patterns_to_download_options) {
        auto option = dynamic_cast<libdnf5::OptionString *>(pattern.get());
        pkg_specs.push_back(option->get_value());
    }

    context.update_repo_metadata_from_specs(pkg_specs);
    if (resolve_option->get_value() && !alldeps_option->get_value()) {
        context.set_load_system_repo(true);
    } else if (!resolve_option->get_value() && alldeps_option->get_value()) {
        throw libdnf5::cli::ArgumentParserMissingDependentArgumentError(
            //TODO(jrohel): Add support for requiring an argument by another argument in ArgumentParser?
            M_("Option \"--alldeps\" should be used with \"--resolve\""));
    } else {
        context.set_load_system_repo(false);
    }
    context.set_load_available_repos(Context::LoadAvailableRepos::ENABLED);
    // Default destination for downloaded rpms is the current directory
    context.base.get_config().get_destdir_option().set(libdnf5::Option::Priority::PLUGINDEFAULT, ".");
}

void DownloadCommand::run() {
    auto & ctx = get_context();

    auto create_nevra_pkg_pair = [](const libdnf5::rpm::Package & pkg) { return std::make_pair(pkg.get_nevra(), pkg); };

    std::map<std::string, libdnf5::rpm::Package> download_pkgs;
    libdnf5::rpm::PackageQuery full_pkg_query(ctx.base);
    for (auto & pattern : *patterns_to_download_options) {
        libdnf5::rpm::PackageQuery pkg_query(full_pkg_query);
        auto option = dynamic_cast<libdnf5::OptionString *>(pattern.get());
        pkg_query.resolve_pkg_spec(option->get_value(), {}, true);
        pkg_query.filter_available();
        pkg_query.filter_priority();
        pkg_query.filter_latest_evr();

        for (const auto & pkg : pkg_query) {
            download_pkgs.insert(create_nevra_pkg_pair(pkg));

            if (resolve_option->get_value()) {
                auto goal = std::make_unique<libdnf5::Goal>(ctx.base);
                goal->add_rpm_install(pkg, {});

                auto transaction = goal->resolve();
                auto transaction_problems = transaction.get_problems();
                if (transaction_problems != libdnf5::GoalProblem::NO_PROBLEM) {
                    if (transaction_problems != libdnf5::GoalProblem::NOT_FOUND) {
                        throw GoalResolveError(transaction);
                    }
                }
                for (auto & tspkg : transaction.get_transaction_packages()) {
                    if (transaction_item_action_is_inbound(tspkg.get_action()) &&
                        tspkg.get_package().get_repo()->get_type() != libdnf5::repo::Repo::Type::COMMANDLINE) {
                        download_pkgs.insert(create_nevra_pkg_pair(tspkg.get_package()));
                    }
                }
            }
        }
    }

    if (!download_pkgs.empty()) {
        libdnf5::repo::PackageDownloader downloader(ctx.base);

<<<<<<< HEAD
        // for download command, we don't want to mark the packages for removal
        downloader.force_keep_packages(true);

        for (auto & [nevra, pkg] : download_pkgs) {
            downloader.add(pkg);
=======
    if (url_option->get_value()) {
        // If no urlprotocols are specified, then any urlprotocol is acceptable
        if (urlprotocol_option.empty()) {
            urlprotocol_option = urlprotocol_valid_options;
        }
        for (auto & [nerva, pkg] : download_pkgs) {
            auto urls = pkg.get_remote_locations();
            libdnf_assert(!urls.empty(), "Failed to get mirror for package: \"{}\"", pkg.get_name());
            auto valid_url = std::find_if(urls.begin(), urls.end(), [this](std::string url) {
                for (auto protocol : urlprotocol_option) {
                    if (url.starts_with(protocol)) {
                        return true;
                    }
                }
                return false;
            });
            if (valid_url == urls.end()) {
                libdnf_assert(true, "Failed to get mirror for package: \"{}\"", pkg.get_name());
            }
            std::cout << *valid_url << std::endl;
>>>>>>> 31411c22 (Added urlprotocol)
        }

        std::cout << "Downloading Packages:" << std::endl;
        downloader.download();
        std::cout << std::endl;
    }
}

}  // namespace dnf5
