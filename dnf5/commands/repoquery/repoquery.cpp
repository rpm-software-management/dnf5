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

#include "repoquery.hpp"

#include "libdnf-cli/output/repoquery.hpp"

#include <libdnf/advisory/advisory_query.hpp>
#include <libdnf/conf/const.hpp>
#include <libdnf/conf/option_string.hpp>
#include <libdnf/rpm/package.hpp>
#include <libdnf/rpm/package_query.hpp>
#include <libdnf/rpm/package_set.hpp>

#include <iostream>

namespace dnf5 {

using namespace libdnf::cli;

void RepoqueryCommand::set_parent_command() {
    auto * arg_parser_parent_cmd = get_session().get_argument_parser().get_root_command();
    auto * arg_parser_this_cmd = get_argument_parser_command();
    arg_parser_parent_cmd->register_command(arg_parser_this_cmd);
    arg_parser_parent_cmd->get_group("query_commands").register_argument(arg_parser_this_cmd);
}

void RepoqueryCommand::set_argument_parser() {
    auto & ctx = get_context();
    auto & parser = ctx.get_argument_parser();

    auto & cmd = *get_argument_parser_command();
    cmd.set_description("Search for packages matching various criteria");

    available_option = dynamic_cast<libdnf::OptionBool *>(
        parser.add_init_value(std::unique_ptr<libdnf::OptionBool>(new libdnf::OptionBool(true))));

    installed_option = dynamic_cast<libdnf::OptionBool *>(
        parser.add_init_value(std::unique_ptr<libdnf::OptionBool>(new libdnf::OptionBool(false))));

    info_option = dynamic_cast<libdnf::OptionBool *>(
        parser.add_init_value(std::unique_ptr<libdnf::OptionBool>(new libdnf::OptionBool(false))));

    nevra_option = dynamic_cast<libdnf::OptionBool *>(
        parser.add_init_value(std::unique_ptr<libdnf::OptionBool>(new libdnf::OptionBool(true))));

    auto available = parser.add_new_named_arg("available");
    available->set_long_name("available");
    available->set_description("display available packages (default)");
    available->set_const_value("true");
    available->link_value(available_option);

    auto installed = parser.add_new_named_arg("installed");
    installed->set_long_name("installed");
    installed->set_description("display installed packages");
    installed->set_const_value("true");
    installed->link_value(installed_option);

    auto info = parser.add_new_named_arg("info");
    info->set_long_name("info");
    info->set_description("show detailed information about the packages");
    info->set_const_value("true");
    info->link_value(info_option);

    auto nevra = parser.add_new_named_arg("nevra");
    nevra->set_long_name("nevra");
    nevra->set_description("use name-epoch:version-release.architecture format for displaying packages (default)");
    nevra->set_const_value("true");
    nevra->link_value(nevra_option);

    auto keys =
        parser.add_new_positional_arg("keys_to_match", ArgumentParser::PositionalArg::UNLIMITED, nullptr, nullptr);
    keys->set_description("List of keys to match");
    keys->set_parse_hook_func(
        [this]([[maybe_unused]] ArgumentParser::PositionalArg * arg, int argc, const char * const argv[]) {
            parse_add_specs(argc, argv, pkg_specs, pkg_file_paths);
            return true;
        });
    keys->set_complete_hook_func([this, &ctx](const char * arg) {
        if (this->installed_option->get_value()) {
            return match_specs(ctx, arg, true, false, false, true);
        } else {
            return match_specs(ctx, arg, false, true, true, false);
        }
    });

    info->add_conflict_argument(*nevra);

    duplicates = std::make_unique<libdnf::cli::session::BoolOption>(
        *this,
        "duplicates",
        '\0',
        "Limit the resulting set to installed duplicate packages (i.e. more package versions for  the  same  name and "
        "architecture). Installonly packages are excluded from this set.",
        false);

    advisory_name = std::make_unique<AdvisoryOption>(*this);
    advisory_security = std::make_unique<SecurityOption>(*this);
    advisory_bugfix = std::make_unique<BugfixOption>(*this);
    advisory_enhancement = std::make_unique<EnhancementOption>(*this);
    advisory_newpackage = std::make_unique<NewpackageOption>(*this);
    advisory_severity = std::make_unique<AdvisorySeverityOption>(*this);
    advisory_bz = std::make_unique<BzOption>(*this);
    advisory_cve = std::make_unique<CveOption>(*this);

    cmd.register_named_arg(available);
    cmd.register_named_arg(installed);
    cmd.register_named_arg(info);
    cmd.register_named_arg(nevra);
    cmd.register_positional_arg(keys);
}

void RepoqueryCommand::configure() {
    auto & context = get_context();
    context.update_repo_metadata_from_specs(pkg_specs);
    only_system_repo_needed = installed_option->get_value() || duplicates->get_value();
    context.set_load_system_repo(only_system_repo_needed);
    bool updateinfo_needed = advisory_name->get_value().empty() || advisory_security->get_value() ||
                             advisory_bugfix->get_value() || advisory_enhancement->get_value() ||
                             advisory_newpackage->get_value() || advisory_severity->get_value().empty() ||
                             advisory_bz->get_value().empty() || advisory_cve->get_value().empty();
    if (updateinfo_needed) {
        context.base.get_config().optional_metadata_types().add_item(libdnf::METADATA_TYPE_UPDATEINFO);
    }
    context.set_load_available_repos(
        available_option->get_priority() >= libdnf::Option::Priority::COMMANDLINE || !only_system_repo_needed
            ? Context::LoadAvailableRepos::ENABLED
            : Context::LoadAvailableRepos::NONE);
}

void RepoqueryCommand::load_additional_packages() {
    if (available_option->get_priority() >= libdnf::Option::Priority::COMMANDLINE || !only_system_repo_needed) {
        cmdline_packages = get_context().add_cmdline_packages(pkg_file_paths);
    }
}

void RepoqueryCommand::run() {
    auto & ctx = get_context();

    libdnf::rpm::PackageSet result_pset(ctx.base);
    libdnf::rpm::PackageQuery full_package_query(ctx.base);

    auto advisories = advisory_query_from_cli_input(
        ctx.base,
        advisory_name->get_value(),
        advisory_security->get_value(),
        advisory_bugfix->get_value(),
        advisory_enhancement->get_value(),
        advisory_newpackage->get_value(),
        advisory_severity->get_value(),
        advisory_bz->get_value(),
        advisory_cve->get_value());
    if (advisories.has_value()) {
        full_package_query.filter_advisories(advisories.value(), libdnf::sack::QueryCmp::GTE);
    }

    if (duplicates->get_value()) {
        auto & cfg_main = ctx.base.get_config();
        const auto & installonly_packages = cfg_main.installonlypkgs().get_value();
        auto installonly_query = full_package_query;
        installonly_query.filter_provides(installonly_packages, libdnf::sack::QueryCmp::GLOB);
        full_package_query -= installonly_query;
        full_package_query.filter_duplicates();
    }

    if (pkg_specs.empty() && pkg_file_paths.empty()) {
        result_pset |= full_package_query;
    } else {
        for (const auto & pkg : cmdline_packages) {
            if (full_package_query.contains(pkg)) {
                result_pset.add(pkg);
            }
        }

        const libdnf::ResolveSpecSettings settings{.ignore_case = true, .with_provides = false};
        for (const auto & spec : pkg_specs) {
            libdnf::rpm::PackageQuery package_query(full_package_query);
            package_query.resolve_pkg_spec(spec, settings, true);
            result_pset |= package_query;
        }
    }

    if (info_option->get_value()) {
        for (auto package : result_pset) {
            libdnf::cli::output::print_package_info_table(package);
            std::cout << '\n';
        }
    } else {
        for (auto package : result_pset) {
            std::cout << package.get_full_nevra() << '\n';
        }
    }
}

}  // namespace dnf5
