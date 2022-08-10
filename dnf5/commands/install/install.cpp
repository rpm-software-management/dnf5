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

#include "install.hpp"

namespace dnf5 {

using namespace libdnf::cli;

void InstallCommand::set_argument_parser() {
    auto & ctx = get_context();
    auto & parser = ctx.get_argument_parser();

    auto & cmd = *get_argument_parser_command();
    cmd.set_short_description("Install software");

    auto keys =
        parser.add_new_positional_arg("keys_to_match", ArgumentParser::PositionalArg::UNLIMITED, nullptr, nullptr);
    keys->set_short_description("List of keys to match");
    keys->set_parse_hook_func(
        [this]([[maybe_unused]] ArgumentParser::PositionalArg * arg, int argc, const char * const argv[]) {
            parse_add_specs(argc, argv, pkg_specs, pkg_file_paths);
            return true;
        });
    keys->set_complete_hook_func([&ctx](const char * arg) { return match_specs(ctx, arg, false, true, true, false); });

    advisory_name = std::make_unique<AdvisoryOption>(*this);
    advisory_security = std::make_unique<SecurityOption>(*this);
    advisory_bugfix = std::make_unique<BugfixOption>(*this);
    advisory_enhancement = std::make_unique<EnhancementOption>(*this);
    advisory_newpackage = std::make_unique<NewpackageOption>(*this);
    advisory_severity = std::make_unique<AdvisorySeverityOption>(*this);
    advisory_bz = std::make_unique<BzOption>(*this);
    advisory_cve = std::make_unique<CveOption>(*this);

    cmd.register_positional_arg(keys);
}

void InstallCommand::configure() {
    auto & context = get_context();
    context.update_repo_load_flags_from_specs(pkg_specs);
    context.set_load_system_repo(true);
    bool updateinfo_needed = advisory_name->get_value().empty() || advisory_security->get_value() ||
                             advisory_bugfix->get_value() || advisory_enhancement->get_value() ||
                             advisory_newpackage->get_value() || advisory_severity->get_value().empty() ||
                             advisory_bz->get_value().empty() || advisory_cve->get_value().empty();
    if (updateinfo_needed) {
        context.set_available_repos_load_flags(
            context.get_available_repos_load_flags() | libdnf::repo::LoadFlags::UPDATEINFO);
    }

    context.set_load_available_repos(Context::LoadAvailableRepos::ENABLED);
}

void InstallCommand::load_additional_packages() {
    cmdline_packages = get_context().add_cmdline_packages(pkg_file_paths);
}

void InstallCommand::run() {
    auto & ctx = get_context();
    auto goal = get_context().get_goal();
    auto settings = libdnf::GoalJobSettings();
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
        settings.advisory_filter = advisories;
    }
    for (const auto & pkg : cmdline_packages) {
        goal->add_rpm_install(pkg, settings);
    }
    for (const auto & spec : pkg_specs) {
        goal->add_rpm_install(spec, settings);
    }
}

}  // namespace dnf5
