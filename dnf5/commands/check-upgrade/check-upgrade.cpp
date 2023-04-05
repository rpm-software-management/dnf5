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

#include "check-upgrade.hpp"

#include "utils/bgettext/bgettext-mark-domain.h"

#include <dnf5/shared_options.hpp>
#include <libdnf-cli/output/changelogs.hpp>
#include <libdnf-cli/output/package_list_sections.hpp>
#include <libdnf/conf/const.hpp>
#include <libdnf/rpm/package_query.hpp>

#include <iostream>

namespace dnf5 {

using namespace libdnf::cli;

void CheckUpgradeCommand::set_parent_command() {
    auto * arg_parser_parent_cmd = get_session().get_argument_parser().get_root_command();
    auto * arg_parser_this_cmd = get_argument_parser_command();
    arg_parser_parent_cmd->register_command(arg_parser_this_cmd);
    arg_parser_parent_cmd->get_group("software_management_commands").register_argument(arg_parser_this_cmd);
}

void CheckUpgradeCommand::set_argument_parser() {
    auto & ctx = get_context();
    auto & parser = ctx.get_argument_parser();

    auto & cmd = *get_argument_parser_command();
    cmd.set_description("Check for available package upgrades");

    changelogs = dynamic_cast<libdnf::OptionBool *>(
        parser.add_init_value(std::unique_ptr<libdnf::OptionBool>(new libdnf::OptionBool(false))));
    auto changelogs_opt = parser.add_new_named_arg("changelogs");
    changelogs_opt->set_long_name("changelogs");
    changelogs_opt->set_description("Show changelogs before update.");
    changelogs_opt->set_const_value("true");
    changelogs_opt->link_value(changelogs);
    cmd.register_named_arg(changelogs_opt);

    auto keys = parser.add_new_positional_arg("specs", ArgumentParser::PositionalArg::UNLIMITED, nullptr, nullptr);
    keys->set_description("List of package specs to check for upgrades");
    keys->set_parse_hook_func(
        [this]([[maybe_unused]] ArgumentParser::PositionalArg * arg, int argc, const char * const argv[]) {
            for (int i = 0; i < argc; ++i) {
                pkg_specs.emplace_back(argv[i]);
            }
            return true;
        });
    keys->set_complete_hook_func([&ctx](const char * arg) { return match_specs(ctx, arg, true, false, true, false); });
    cmd.register_positional_arg(keys);

    advisory_name = std::make_unique<AdvisoryOption>(*this);
    advisory_security = std::make_unique<SecurityOption>(*this);
    advisory_bugfix = std::make_unique<BugfixOption>(*this);
    advisory_enhancement = std::make_unique<EnhancementOption>(*this);
    advisory_newpackage = std::make_unique<NewpackageOption>(*this);
    advisory_severity = std::make_unique<AdvisorySeverityOption>(*this);
    advisory_bz = std::make_unique<BzOption>(*this);
    advisory_cve = std::make_unique<CveOption>(*this);
}

void CheckUpgradeCommand::configure() {
    auto & context = get_context();
    context.update_repo_metadata_from_specs(pkg_specs);
    context.set_load_system_repo(true);
    context.set_load_available_repos(Context::LoadAvailableRepos::ENABLED);
    if (changelogs->get_value()) {
        context.base.get_config().get_optional_metadata_types_option().add(libdnf::OPTIONAL_METADATA_TYPES);
    }
}

std::unique_ptr<libdnf::cli::output::PackageListSections> CheckUpgradeCommand::create_output() {
    auto out = std::make_unique<libdnf::cli::output::PackageListSections>();
    out->setup_cols();
    return out;
}

void CheckUpgradeCommand::run() {
    auto & ctx = get_context();
    auto & config = ctx.base.get_config();

    libdnf::rpm::PackageQuery full_package_query(ctx.base);
    libdnf::rpm::PackageQuery upgrades_query(ctx.base);

    // filter by provided specs, for `check-upgrade <pkg1> <pkg2> ...`
    if (!pkg_specs.empty()) {
        upgrades_query = libdnf::rpm::PackageQuery(ctx.base, libdnf::sack::ExcludeFlags::APPLY_EXCLUDES, true);
        libdnf::ResolveSpecSettings settings{.with_nevra = true, .with_provides = false, .with_filenames = false};
        for (const auto & spec : pkg_specs) {
            libdnf::rpm::PackageQuery package_query(ctx.base);
            package_query.resolve_pkg_spec(spec, settings, true);
            upgrades_query |= package_query;
        }
    }

    // Only consider updates from the highest priority repo. Must be done
    // before `filter_upgrades()` since the filter needs to consider
    // currently-installed packages as well as upgrades.
    upgrades_query.filter_priority();

    upgrades_query.filter_upgrades();

    // Source RPMs should not be reported as upgrades, e.g. when the binary
    // package is marked exclude and only the source package is left in the
    // repo
    upgrades_query.filter_arch({"src", "nosrc"}, libdnf::sack::QueryCmp::NOT_EXACT);

    // filter by advisory flags, e.g. `--security`
    size_t size_before_filter_advisories = upgrades_query.size();
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
        upgrades_query.filter_advisories(std::move(advisories.value()), libdnf::sack::QueryCmp::GTE);
    }

    libdnf::rpm::PackageQuery installed_query(ctx.base);
    installed_query.filter_installed();

    libdnf::rpm::PackageQuery obsoletes_query(upgrades_query);
    obsoletes_query.filter_obsoletes(installed_query);

    // prepare a map of obsoleted packages {obsoleter_id: [obsoleted_pkgs]}
    std::map<libdnf::rpm::PackageId, std::vector<libdnf::rpm::Package>> obsoletes;
    for (const auto & pkg : obsoletes_query) {
        std::vector<libdnf::rpm::Package> obsoleted;
        libdnf::rpm::PackageQuery obs_q(installed_query);
        obs_q.filter_provides(pkg.get_obsoletes());
        for (const auto & pkg_ob : obs_q) {
            obsoleted.emplace_back(pkg_ob);
        }
        obsoletes.emplace(pkg.get_id(), obsoleted);
    }

    auto colorizer = std::make_unique<libdnf::cli::output::PkgColorizer>(
        installed_query,
        "",  //config.get_color_list_available_install_option().get_value(),
        "",  //config.get_color_list_available_downgrade_option().get_value(),
        config.get_color_list_available_reinstall_option().get_value(),
        config.get_color_list_available_upgrade_option().get_value());
    auto sections = create_output();

    bool package_matched = false;
    package_matched |= sections->add_section("", upgrades_query);
    package_matched |= sections->add_section("Obsoleting packages", obsoletes_query, colorizer, obsoletes);

    if (package_matched) {
        // If any upgrades were found, print a table of them, and optionally print changelogs. Return exit code 100.
        sections->print();
        if (changelogs->get_value()) {
            libdnf::cli::output::print_changelogs(upgrades_query, full_package_query, true, 0, 0);
        }
        throw libdnf::cli::SilentCommandExitError(100, M_(""));
    } else if (
        // Otherwise, main() will exit with code 0.
        size_before_filter_advisories > 0 &&
        (!advisory_name->get_value().empty() || advisory_security->get_value() || advisory_bugfix->get_value() ||
         advisory_newpackage->get_value() || !advisory_severity->get_value().empty() ||
         !advisory_bz->get_value().empty() || !advisory_cve->get_value().empty())) {
        std::cout << "No security updates needed, but " << size_before_filter_advisories << " update(s) available"
                  << std::endl;
    }
}

}  // namespace dnf5
