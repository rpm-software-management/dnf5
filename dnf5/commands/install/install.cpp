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

#include "install.hpp"

#include <dnf5/shared_options.hpp>
#include <libdnf5/conf/const.hpp>

namespace dnf5 {

using namespace libdnf5::cli;

void InstallCommand::set_parent_command() {
    auto * arg_parser_parent_cmd = get_session().get_argument_parser().get_root_command();
    auto * arg_parser_this_cmd = get_argument_parser_command();
    arg_parser_parent_cmd->register_command(arg_parser_this_cmd);
    arg_parser_parent_cmd->get_group("software_management_commands").register_argument(arg_parser_this_cmd);
}

void InstallCommand::set_argument_parser() {
    auto & ctx = get_context();
    auto & parser = ctx.get_argument_parser();

    auto & cmd = *get_argument_parser_command();
    cmd.set_description(_("Install software"));

    auto keys = parser.add_new_positional_arg("specs", ArgumentParser::PositionalArg::AT_LEAST_ONE, nullptr, nullptr);
    keys->set_description("List of <package-spec-NPFB>|@<group-spec>|@<environment-spec> to install");
    keys->set_parse_hook_func(
        [this]([[maybe_unused]] ArgumentParser::PositionalArg * arg, int argc, const char * const argv[]) {
            for (int i = 0; i < argc; ++i) {
                pkg_specs.emplace_back(argv[i]);
            }
            return true;
        });
    keys->set_complete_hook_func([&ctx](const char * arg) { return ctx.match_specs(arg, false, true, true, false); });
    cmd.register_positional_arg(keys);

    allow_erasing = std::make_unique<AllowErasingOption>(*this);
    auto skip_broken = std::make_unique<SkipBrokenOption>(*this);
    auto skip_unavailable = std::make_unique<SkipUnavailableOption>(*this);
    create_allow_downgrade_options(*this);
    create_from_repo_option(*this, from_repos, true);
    create_from_vendor_option(*this, from_vendors, true);
    create_downloadonly_option(*this);
    create_offline_option(*this);

    advisory_name = std::make_unique<AdvisoryOption>(*this);
    advisory_severity = std::make_unique<AdvisorySeverityOption>(*this);
    advisory_bz = std::make_unique<BzOption>(*this);
    advisory_cve = std::make_unique<CveOption>(*this);
    advisory_security = std::make_unique<SecurityOption>(*this);
    advisory_bugfix = std::make_unique<BugfixOption>(*this);
    advisory_enhancement = std::make_unique<EnhancementOption>(*this);
    advisory_newpackage = std::make_unique<NewpackageOption>(*this);
    create_store_option(*this);
}

void InstallCommand::configure() {
    auto & context = get_context();
    context.update_repo_metadata_from_specs(pkg_specs);
    context.set_load_system_repo(true);
    context.update_repo_metadata_from_advisory_options(
        advisory_name->get_value(),
        advisory_security->get_value(),
        advisory_bugfix->get_value(),
        advisory_enhancement->get_value(),
        advisory_newpackage->get_value(),
        advisory_severity->get_value(),
        advisory_bz->get_value(),
        advisory_cve->get_value());

    context.set_load_available_repos(Context::LoadAvailableRepos::ENABLED);
}

void InstallCommand::run() {
    auto & ctx = get_context();
    auto goal = get_context().get_goal();
    goal->set_allow_erasing(allow_erasing->get_value());
    auto settings = libdnf5::GoalJobSettings();
    settings.set_to_repo_ids(from_repos);
    settings.set_to_vendors(from_vendors);
    ErrorHandling error_mode =
        determine_error_mode(ctx, !ctx.get_base().get_config().get_skip_unavailable_option().get_value());
    auto advisories = advisory_query_from_cli_input(
        ctx.get_base(),
        advisory_name->get_value(),
        advisory_security->get_value(),
        advisory_bugfix->get_value(),
        advisory_enhancement->get_value(),
        advisory_newpackage->get_value(),
        advisory_severity->get_value(),
        advisory_bz->get_value(),
        advisory_cve->get_value(),
        error_mode);
    if (advisories.has_value()) {
        settings.set_advisory_filter(std::move(advisories.value()));
    }
    for (const auto & spec : pkg_specs) {
        goal->add_install(spec, settings);
    }
}

}  // namespace dnf5
