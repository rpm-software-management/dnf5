// Copyright Contributors to the DNF5 project.
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

#include "download.hpp"

#include "dnf5/shared_options.hpp"

#include <libdnf5/conf/option_string.hpp>
#include <libdnf5/repo/package_downloader.hpp>
#include <libdnf5/rpm/arch.hpp>
#include <libdnf5/rpm/package.hpp>
#include <libdnf5/rpm/package_query.hpp>
#include <libdnf5/rpm/package_set.hpp>
#include <libdnf5/utils/bgettext/bgettext-mark-domain.h>

#include <algorithm>
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
        "package-spec-NPFB",
        ArgumentParser::PositionalArg::AT_LEAST_ONE,
        parser.add_init_value(std::unique_ptr<libdnf5::Option>(new libdnf5::OptionString(nullptr))),
        patterns_to_download_options);
    keys->set_description("List of package-spec-NPFB to download");
    keys->set_complete_hook_func([&ctx](const char * arg) { return match_specs(ctx, arg, false, true, false, false); });

    resolve_option = dynamic_cast<libdnf5::OptionBool *>(
        parser.add_init_value(std::unique_ptr<libdnf5::OptionBool>(new libdnf5::OptionBool(false))));

    alldeps_option = dynamic_cast<libdnf5::OptionBool *>(
        parser.add_init_value(std::unique_ptr<libdnf5::OptionBool>(new libdnf5::OptionBool(false))));

    url_option = dynamic_cast<libdnf5::OptionBool *>(
        parser.add_init_value(std::unique_ptr<libdnf5::OptionBool>(new libdnf5::OptionBool(false))));

    allmirrors_option = dynamic_cast<libdnf5::OptionBool *>(
        parser.add_init_value(std::unique_ptr<libdnf5::OptionBool>(new libdnf5::OptionBool(false))));

    srpm_option = dynamic_cast<libdnf5::OptionBool *>(
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

    auto srpm = parser.add_new_named_arg("srpm");
    srpm->set_long_name("srpm");
    srpm->set_description("Download the src.rpm instead");
    srpm->set_const_value("true");
    srpm->link_value(srpm_option);

    auto url = parser.add_new_named_arg("url");
    url->set_long_name("url");
    url->set_description("Print a URL where the rpms can be downloaded instead of downloading");
    url->set_const_value("true");
    url->link_value(url_option);

    auto allmirrors = parser.add_new_named_arg("allmirrors");
    allmirrors->set_long_name("allmirrors");
    allmirrors->set_description(_("When running with --url, prints URLs from all available mirrors"));
    allmirrors->set_const_value("true");
    allmirrors->link_value(allmirrors_option);

    urlprotocol_valid_options = {"http", "https", "ftp", "file"};
    urlprotocol_option = {};
    auto urlprotocol = parser.add_new_named_arg("urlprotocol");
    urlprotocol->set_long_name("urlprotocol");
    urlprotocol->set_description("When running with --url, limit to specific protocols");
    urlprotocol->set_has_value(true);
    urlprotocol->set_arg_value_help("{http|https|ftp|file},...");
    urlprotocol->set_parse_hook_func(
        [this](
            [[maybe_unused]] ArgumentParser::NamedArg * arg, [[maybe_unused]] const char * option, const char * value) {
            if (urlprotocol_valid_options.find(value) == urlprotocol_valid_options.end()) {
                throw libdnf5::cli::ArgumentParserInvalidValueError(
                    M_("Invalid urlprotocol option: {}"), std::string(value));
            }
            urlprotocol_option.emplace(value);
            return true;
        });

    arch_option = {};
    auto arch = parser.add_new_named_arg("arch");
    arch->set_long_name("arch");
    arch->set_description("Limit to packages of given architectures.");
    arch->set_has_value(true);
    arch->set_arg_value_help("ARCH,...");
    arch->set_parse_hook_func(
        [this](
            [[maybe_unused]] ArgumentParser::NamedArg * arg, [[maybe_unused]] const char * option, const char * value) {
            auto supported_arches = libdnf5::rpm::get_supported_arches();
            if (std::find(supported_arches.begin(), supported_arches.end(), value) == supported_arches.end()) {
                std::string available_arches{};
                auto it = supported_arches.begin();
                if (it != supported_arches.end()) {
                    available_arches.append("\"" + *it + "\"");
                    ++it;
                    for (; it != supported_arches.end(); ++it) {
                        available_arches.append(", \"" + *it + "\"");
                    }
                }
                throw libdnf5::cli::ArgumentParserInvalidValueError(
                    M_("Unsupported architecture \"{0}\". Please choose one from {1}"),
                    std::string(value),
                    available_arches);
            }
            arch_option.emplace(value);
            return true;
        });


    cmd.register_named_arg(arch);
    cmd.register_named_arg(resolve);
    cmd.register_named_arg(alldeps);
    create_from_repo_option(*this, from_repos, true);
    create_from_vendor_option(*this, from_vendors, true);
    create_destdir_option(*this);
    auto skip_unavailable = std::make_unique<SkipUnavailableOption>(*this);
    cmd.register_named_arg(srpm);
    cmd.register_named_arg(url);
    cmd.register_named_arg(urlprotocol);
    cmd.register_named_arg(allmirrors);
    cmd.register_positional_arg(keys);
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

    if (srpm_option->get_value()) {
        context.get_base().get_repo_sack()->enable_source_repos();
    }

    context.set_load_available_repos(Context::LoadAvailableRepos::ENABLED);
    // Default destination for downloaded rpms is the current directory
    context.get_base().get_config().get_destdir_option().set(libdnf5::Option::Priority::PLUGINDEFAULT, ".");
}

void DownloadCommand::run() {
    auto & ctx = get_context();

    auto create_nevra_pkg_pair = [](const libdnf5::rpm::Package & pkg) { return std::make_pair(pkg.get_nevra(), pkg); };

    std::map<std::string, libdnf5::rpm::Package> download_pkgs;
    libdnf5::rpm::PackageQuery full_pkg_query(ctx.get_base(), libdnf5::sack::ExcludeFlags::IGNORE_VERSIONLOCK);
    for (auto & pattern : *patterns_to_download_options) {
        libdnf5::rpm::PackageQuery pkg_query(full_pkg_query);
        auto option = dynamic_cast<libdnf5::OptionString *>(pattern.get());
        pkg_query.resolve_pkg_spec(option->get_value(), {}, true);
        if (from_repos.empty()) {
            pkg_query.filter_available();
        } else {
            pkg_query.filter_repo_id(from_repos, libdnf5::sack::QueryCmp::GLOB);
        }
        if (!from_vendors.empty()) {
            pkg_query.filter_vendor(from_vendors, libdnf5::sack::QueryCmp::GLOB);
        }
        pkg_query.filter_priority();
        pkg_query.filter_latest_evr();
        if (!arch_option.empty()) {
            pkg_query.filter_arch(std::vector<std::string>(arch_option.begin(), arch_option.end()));
        }

        if (!pkg_query.size() && !ctx.get_base().get_config().get_skip_unavailable_option().get_value()) {
            // User tried `dnf5 download non-sense` or `dnf download non-sense-wildcard*`
            throw libdnf5::cli::CommandExitError(
                1,
                M_("No package \"{}\" available; You might want to use --skip-unavailable option."),
                option->get_value());
        }

        for (const auto & pkg : pkg_query) {
            download_pkgs.insert(create_nevra_pkg_pair(pkg));

            if (resolve_option->get_value()) {
                auto goal = std::make_unique<libdnf5::Goal>(ctx.get_base());
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

    if (download_pkgs.empty()) {
        return;
    }

    if (srpm_option->get_value()) {
        std::map<std::string, libdnf5::rpm::Package> source_pkgs;

        libdnf5::rpm::PackageQuery source_pkg_query(ctx.get_base());
        source_pkg_query.filter_arch("src");
        source_pkg_query.filter_available();

        for (auto & [nevra, pkg] : download_pkgs) {
            auto sourcerpm = pkg.get_sourcerpm();

            if (!sourcerpm.empty()) {
                libdnf5::rpm::PackageQuery pkg_query(source_pkg_query);
                pkg_query.filter_epoch({pkg.get_epoch()});

                // Remove ".rpm" to get sourcerpm nevra
                sourcerpm.erase(sourcerpm.length() - 4);
                pkg_query.resolve_pkg_spec(sourcerpm, {}, true);

                for (const auto & spkg : pkg_query) {
                    source_pkgs.insert(create_nevra_pkg_pair(spkg));
                }
            } else if (pkg.get_arch() == "src") {
                source_pkgs.insert(create_nevra_pkg_pair(pkg));
            } else {
                ctx.get_base().get_logger()->info("No source rpm defined for package: \"{}\"", pkg.get_name());
                continue;
            }
        }

        download_pkgs = source_pkgs;
    }

    if (url_option->get_value()) {
        // If no urlprotocols are specified, all values within the urlprotocol_valid_options will be used
        if (urlprotocol_option.empty()) {
            urlprotocol_option = urlprotocol_valid_options;
        }
        for (auto & [nerva, pkg] : download_pkgs) {
            auto urls = pkg.get_remote_locations(urlprotocol_option);
            if (urls.empty()) {
                ctx.get_base().get_logger()->warning("Failed to get mirror for package: \"{}\"", pkg.get_name());
                continue;
            }
            std::cout << urls[0];
            if (allmirrors_option->get_value()) {
                for (size_t index = 1; index < urls.size(); ++index) {
                    std::cout << " " << urls[index];
                }
            }
            std::cout << std::endl;
        }
        return;
    }
    libdnf5::repo::PackageDownloader downloader(ctx.get_base());

    // for download command, we don't want to mark the packages for removal
    downloader.force_keep_packages(true);

    for (auto & [nevra, pkg] : download_pkgs) {
        downloader.add(pkg);
    }

    if (!ctx.get_quiet()) {
        std::cout << "Downloading Packages:" << std::endl;
    }
    downloader.download();
}


}  // namespace dnf5
