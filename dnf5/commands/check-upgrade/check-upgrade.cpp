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

#include <dnf5/shared_options.hpp>
#include <libdnf5-cli/output/changelogs.hpp>
#include <libdnf5-cli/output/package_list_sections.hpp>
#include <libdnf5/conf/const.hpp>
#include <libdnf5/rpm/package_query.hpp>
#include <libdnf5/utils/bgettext/bgettext-mark-domain.h>

#include <iostream>

namespace dnf5 {

using namespace libdnf5::cli;

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
    cmd.set_description(_("Check for available package upgrades"));

    minimal = dynamic_cast<libdnf5::OptionBool *>(
        parser.add_init_value(std::unique_ptr<libdnf5::OptionBool>(new libdnf5::OptionBool(false))));
    auto minimal_opt = parser.add_new_named_arg("minimal");
    minimal_opt->set_long_name("minimal");
    minimal_opt->set_description(
        _("Reports the lowest versions of packages that fix advisories of type bugfix, enhancement, security, or "
          "newpackage. In case that any option limiting advisories is used it reports the lowest versions of packages "
          "that fix advisories matching selected advisory properties"));
    minimal_opt->set_const_value("true");
    minimal_opt->link_value(minimal);
    cmd.register_named_arg(minimal_opt);

    changelogs = dynamic_cast<libdnf5::OptionBool *>(
        parser.add_init_value(std::unique_ptr<libdnf5::OptionBool>(new libdnf5::OptionBool(false))));
    auto changelogs_opt = parser.add_new_named_arg("changelogs");
    changelogs_opt->set_long_name("changelogs");
    changelogs_opt->set_description("Show changelogs before update.");
    changelogs_opt->set_const_value("true");
    changelogs_opt->link_value(changelogs);
    cmd.register_named_arg(changelogs_opt);

    auto keys =
        parser.add_new_positional_arg("package-spec-N>", ArgumentParser::PositionalArg::UNLIMITED, nullptr, nullptr);
    keys->set_description("List of package-spec-N to check for upgrades");
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
    advisory_severity = std::make_unique<AdvisorySeverityOption>(*this);
    advisory_bz = std::make_unique<BzOption>(*this);
    advisory_cve = std::make_unique<CveOption>(*this);
    advisory_security = std::make_unique<SecurityOption>(*this);
    advisory_bugfix = std::make_unique<BugfixOption>(*this);
    advisory_enhancement = std::make_unique<EnhancementOption>(*this);
    advisory_newpackage = std::make_unique<NewpackageOption>(*this);
}

void CheckUpgradeCommand::configure() {
    auto & context = get_context();
    context.update_repo_metadata_from_specs(pkg_specs);
    context.set_load_system_repo(true);
    context.set_load_available_repos(Context::LoadAvailableRepos::ENABLED);
    if (changelogs->get_value()) {
        context.get_base().get_config().get_optional_metadata_types_option().add(
            libdnf5::Option::Priority::RUNTIME, libdnf5::METADATA_TYPE_OTHER);
    }
    context.update_repo_metadata_from_advisory_options(
        advisory_name->get_value(),
        advisory_security->get_value(),
        advisory_bugfix->get_value(),
        advisory_enhancement->get_value(),
        advisory_newpackage->get_value(),
        advisory_severity->get_value(),
        advisory_bz->get_value(),
        advisory_cve->get_value());
}

std::unique_ptr<libdnf5::cli::output::PackageListSections> CheckUpgradeCommand::create_output() {
    auto out = std::make_unique<libdnf5::cli::output::PackageListSections>();
    return out;
}

void CheckUpgradeCommand::run() {
    auto & ctx = get_context();
    auto & config = ctx.get_base().get_config();

    libdnf5::rpm::PackageQuery full_package_query(ctx.get_base());
    libdnf5::rpm::PackageQuery upgrades_query(ctx.get_base());

    // filter by provided specs, for `check-upgrade <pkg1> <pkg2> ...`
    if (!pkg_specs.empty()) {
        upgrades_query = libdnf5::rpm::PackageQuery(ctx.get_base(), libdnf5::sack::ExcludeFlags::APPLY_EXCLUDES, true);
        libdnf5::ResolveSpecSettings settings;
        settings.set_with_nevra(true);
        settings.set_with_provides(false);
        settings.set_with_filenames(false);
        settings.set_with_binaries(false);
        for (const auto & spec : pkg_specs) {
            libdnf5::rpm::PackageQuery package_query(ctx.get_base());
            package_query.resolve_pkg_spec(spec, settings, true);
            upgrades_query |= package_query;
        }
    }

    // Only consider updates from the highest priority repo. Must be done
    // before `filter_upgrades()` since the filter needs to consider
    // currently-installed packages as well as upgrades.
    upgrades_query.filter_priority();

    upgrades_query.filter_upgrades();

    upgrades_query.filter_arch(std::vector<std::string>{"src", "nosrc"}, libdnf5::sack::QueryCmp::NOT_EXACT);

    libdnf5::rpm::PackageQuery installed_query(ctx.get_base());
    installed_query.filter_installed();

    if (minimal->get_value()) {
        if ((advisory_name->get_value().empty() && !advisory_security->get_value() && !advisory_bugfix->get_value() &&
             !advisory_enhancement->get_value() && !advisory_newpackage->get_value() &&
             advisory_severity->get_value().empty() && advisory_bz->get_value().empty() &&
             advisory_cve->get_value().empty())) {
            advisory_security->set(libdnf5::Option::Priority::RUNTIME, true);
            advisory_bugfix->set(libdnf5::Option::Priority::RUNTIME, true);
            advisory_enhancement->set(libdnf5::Option::Priority::RUNTIME, true);
            advisory_newpackage->set(libdnf5::Option::Priority::RUNTIME, true);
        }
    }

    // filter by advisory flags, e.g. `--security`
    size_t size_before_filter_advisories = upgrades_query.size();
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
        false);
    if (advisories.has_value()) {
        upgrades_query.filter_latest_unresolved_advisories(
            std::move(advisories.value()), installed_query, libdnf5::sack::QueryCmp::GTE);
    }

    if (minimal->get_value()) {
        // when minimal is requested, keep only the earliest version
        upgrades_query.filter_earliest_evr();
    } else {
        // Last, only include latest upgrades
        upgrades_query.filter_latest_evr();
    }

    libdnf5::rpm::PackageQuery obsoletes_query(upgrades_query);
    obsoletes_query.filter_obsoletes(installed_query);

    // prepare a map of obsoleted packages {obsoleter_id: [obsoleted_pkgs]}
    std::map<libdnf5::rpm::PackageId, std::vector<libdnf5::rpm::Package>> obsoletes;
    for (const auto & pkg : obsoletes_query) {
        std::vector<libdnf5::rpm::Package> obsoleted;
        libdnf5::rpm::PackageQuery obs_q(installed_query);
        obs_q.filter_provides(pkg.get_obsoletes());
        for (const auto & pkg_ob : obs_q) {
            obsoleted.emplace_back(pkg_ob);
        }
        obsoletes.emplace(pkg.get_id(), obsoleted);
    }

    auto sections = create_output();

    bool package_matched = false;
    package_matched |= sections->add_section("", upgrades_query);
    package_matched |= sections->add_section("Obsoleting packages", obsoletes_query, obsoletes);

    if (package_matched) {
        // If any upgrades were found, print a table of them, and optionally print changelogs. Return exit code 100.
        auto colorizer = std::make_unique<libdnf5::cli::output::PkgColorizer>(
            installed_query,
            "",  //config.get_color_list_available_install_option().get_value(),
            "",  //config.get_color_list_available_downgrade_option().get_value(),
            config.get_color_list_available_reinstall_option().get_value(),
            config.get_color_list_available_upgrade_option().get_value());
        sections->print(colorizer);
        if (changelogs->get_value()) {
            libdnf5::cli::output::print_changelogs(
                upgrades_query, {libdnf5::cli::output::ChangelogFilterType::UPGRADES, full_package_query});
        }
        throw libdnf5::cli::SilentCommandExitError(100);
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
