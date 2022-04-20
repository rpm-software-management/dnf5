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

#include "upgrade.hpp"

namespace dnf5 {

using namespace libdnf::cli;

void UpgradeCommand::set_argument_parser() {
    auto & ctx = get_context();
    auto & parser = ctx.get_argument_parser();

    auto & cmd = *get_argument_parser_command();
    cmd.set_short_description("Upgrade software");

    minimal = dynamic_cast<libdnf::OptionBool *>(
        parser.add_init_value(std::unique_ptr<libdnf::OptionBool>(new libdnf::OptionBool(false))));
    auto minimal_opt = parser.add_new_named_arg("minimal");
    minimal_opt->set_long_name("minimal");
    // TODO(dmach): Explain how this relates to options such as --security, --enhacement etc.
    minimal_opt->set_short_description(
        "Upgrade packages only to the lowest versions of packages that fix the problems affecting the system.");
    minimal_opt->set_const_value("true");
    minimal_opt->link_value(minimal);
    cmd.register_named_arg(minimal_opt);

    auto keys =
        parser.add_new_positional_arg("keys_to_match", ArgumentParser::PositionalArg::UNLIMITED, nullptr, nullptr);
    keys->set_short_description("List of keys to match");
    keys->set_parse_hook_func(
        [this]([[maybe_unused]] ArgumentParser::PositionalArg * arg, int argc, const char * const argv[]) {
            parse_add_specs(argc, argv, pkg_specs, pkg_file_paths);
            return true;
        });
    keys->set_complete_hook_func([&ctx](const char * arg) { return match_specs(ctx, arg, true, false, true, false); });

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

void UpgradeCommand::configure() {
    auto & context = get_context();
    context.set_load_system_repo(true);
    context.set_load_available_repos(Context::LoadAvailableRepos::ENABLED);
}

void UpgradeCommand::load_additional_packages() {
    cmdline_packages = get_context().add_cmdline_packages(pkg_file_paths);
}

void UpgradeCommand::run() {
    auto & ctx = get_context();
    auto goal = get_context().get_goal();

    auto settings = libdnf::GoalJobSettings();

    if (minimal->get_value()) {
        if ((advisory_name->get_value().empty() && !advisory_security->get_value() && !advisory_bugfix->get_value() &&
             !advisory_enhancement->get_value() && !advisory_newpackage->get_value() &&
             advisory_severity->get_value().empty() && advisory_bz->get_value().empty() &&
             advisory_cve->get_value().empty())) {
            advisory_security->set(libdnf::Option::Priority::RUNTIME, true);
            advisory_bugfix->set(libdnf::Option::Priority::RUNTIME, true);
            advisory_enhancement->set(libdnf::Option::Priority::RUNTIME, true);
            advisory_newpackage->set(libdnf::Option::Priority::RUNTIME, true);
        }
    }

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

    if (pkg_specs.empty() && pkg_file_paths.empty()) {
        goal->add_rpm_upgrade(settings, minimal->get_value());
    } else {
        for (const auto & pkg : cmdline_packages) {
            goal->add_rpm_upgrade(pkg, settings, minimal->get_value());
        }
        for (const auto & spec : pkg_specs) {
            goal->add_rpm_upgrade(spec, settings, minimal->get_value());
        }
    }
}

}  // namespace dnf5
