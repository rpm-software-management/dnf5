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

#include "list.hpp"

#include "utils/bgettext/bgettext-mark-domain.h"

#include <libdnf-cli/output/package_list_sections.hpp>
#include <libdnf/rpm/package_query.hpp>

#include <iostream>

namespace dnf5 {

using namespace libdnf::cli;


void ListCommand::set_parent_command() {
    auto * arg_parser_parent_cmd = get_session().get_argument_parser().get_root_command();
    auto * arg_parser_this_cmd = get_argument_parser_command();
    arg_parser_parent_cmd->register_command(arg_parser_this_cmd);
    arg_parser_parent_cmd->get_group("query_commands").register_argument(arg_parser_this_cmd);
}

void ListCommand::set_argument_parser() {
    auto & ctx = get_context();
    auto & parser = ctx.get_argument_parser();
    auto & config = ctx.base.get_config();

    auto & cmd = *get_argument_parser_command();
    cmd.set_description("Lists packages depending on the packages' relation to the system");

    auto specs = parser.add_new_positional_arg("specs", ArgumentParser::PositionalArg::UNLIMITED, nullptr, nullptr);
    specs->set_description("List of keys to match");
    specs->set_parse_hook_func(
        [this]([[maybe_unused]] ArgumentParser::PositionalArg * arg, int argc, const char * const argv[]) {
            for (int i = 0; i < argc; ++i) {
                pkg_specs.emplace_back(argv[i]);
            }
            return true;
        });
    specs->set_complete_hook_func([&ctx](const char * arg) { return match_specs(ctx, arg, true, true, false, false); });
    cmd.register_positional_arg(specs);

    auto showduplicates = parser.add_new_named_arg("showduplicates");
    showduplicates->set_long_name("showduplicates");
    showduplicates->set_description("Show all versions of the packages, not only the latest ones.");
    showduplicates->set_const_value("true");
    showduplicates->link_value(&config.get_showdupesfromrepos_option());
    cmd.register_named_arg(showduplicates);

    auto conflicts =
        parser.add_conflict_args_group(std::make_unique<std::vector<libdnf::cli::ArgumentParser::Argument *>>());

    installed =
        std::make_unique<libdnf::cli::session::BoolOption>(*this, "installed", '\0', "List installed packages.", false);
    conflicts->push_back(installed->arg);

    available =
        std::make_unique<libdnf::cli::session::BoolOption>(*this, "available", '\0', "List available packages.", false);
    conflicts->push_back(available->arg);

    extras = std::make_unique<libdnf::cli::session::BoolOption>(
        *this,
        "extras",
        '\0',
        "List extras, that is packages installed on the system that are not available in any known repository.",
        false);
    conflicts->push_back(extras->arg);

    obsoletes = std::make_unique<libdnf::cli::session::BoolOption>(
        *this,
        "obsoletes",
        '\0',
        "List packages installed on the system that are obsoleted by packages in any known repository.",
        false);
    conflicts->push_back(obsoletes->arg);

    recent = std::make_unique<libdnf::cli::session::BoolOption>(
        *this, "recent", '\0', "List packages recently added into the repositories.", false);
    conflicts->push_back(recent->arg);

    upgrades = std::make_unique<libdnf::cli::session::BoolOption>(
        *this, "upgrades", '\0', "List upgrades available for the installed packages.", false);
    conflicts->push_back(upgrades->arg);

    autoremove = std::make_unique<libdnf::cli::session::BoolOption>(
        *this, "autoremove", '\0', "List packages which will be removed by the 'dnf autoremove' command.", false);
    conflicts->push_back(autoremove->arg);

    installed->arg->set_conflict_arguments(conflicts);
    available->arg->set_conflict_arguments(conflicts);
    extras->arg->set_conflict_arguments(conflicts);
    obsoletes->arg->set_conflict_arguments(conflicts);
    recent->arg->set_conflict_arguments(conflicts);
    upgrades->arg->set_conflict_arguments(conflicts);
    autoremove->arg->set_conflict_arguments(conflicts);
}

void ListCommand::configure() {
    // TODO(mblaha): do not force expired metadata sync if not explicitely required
    pkg_narrow = PkgNarrow::ALL;
    Context::LoadAvailableRepos load_available = Context::LoadAvailableRepos::ENABLED;
    bool load_system = true;
    if (available->get_value()) {
        pkg_narrow = PkgNarrow::AVAILABLE;
    } else if (installed->get_value()) {
        load_available = Context::LoadAvailableRepos::NONE;
        pkg_narrow = PkgNarrow::INSTALLED;
    } else if (extras->get_value()) {
        pkg_narrow = PkgNarrow::EXTRAS;
    } else if (obsoletes->get_value()) {
        pkg_narrow = PkgNarrow::OBSOLETES;
    } else if (recent->get_value()) {
        pkg_narrow = PkgNarrow::RECENT;
    } else if (upgrades->get_value()) {
        pkg_narrow = PkgNarrow::UPGRADES;
    } else if (autoremove->get_value()) {
        load_available = Context::LoadAvailableRepos::NONE;
        pkg_narrow = PkgNarrow::AUTOREMOVE;
    }
    auto & context = get_context();
    context.set_load_available_repos(load_available);
    context.set_load_system_repo(load_system);
}

std::unique_ptr<libdnf::cli::output::PackageListSections> ListCommand::create_output() {
    auto out = std::make_unique<libdnf::cli::output::PackageListSections>();
    out->setup_cols();
    return out;
}

void ListCommand::run() {
    auto & ctx = get_context();
    auto & config = ctx.base.get_config();
    auto showduplicates = config.get_showdupesfromrepos_option().get_value();

    libdnf::rpm::PackageQuery full_package_query(ctx.base);
    libdnf::rpm::PackageQuery base_query(ctx.base);

    // pre-select by patterns
    if (!pkg_specs.empty()) {
        base_query = libdnf::rpm::PackageQuery(ctx.base, libdnf::sack::ExcludeFlags::APPLY_EXCLUDES, true);
        libdnf::ResolveSpecSettings settings{.with_nevra = true, .with_provides = false, .with_filenames = false};
        for (const auto & spec : pkg_specs) {
            libdnf::rpm::PackageQuery pkg_query(full_package_query);
            pkg_query.resolve_pkg_spec(spec, settings, true);
            base_query |= pkg_query;
        }
    }

    // did any package match the criteria?
    bool package_matched = false;
    // output table
    auto sections = create_output();

    libdnf::rpm::PackageQuery installed(base_query);
    installed.filter_installed();

    // TODO(mblaha) currently only the installed version and upgrades
    // are highlighted to make the output a bit saner
    auto colorizer = std::make_unique<libdnf::cli::output::PkgColorizer>(
        installed,
        "",  //config.get_color_list_available_install_option().get_value(),
        "",  //config.get_color_list_available_downgrade_option().get_value(),
        config.get_color_list_available_reinstall_option().get_value(),
        config.get_color_list_available_upgrade_option().get_value());

    switch (pkg_narrow) {
        case PkgNarrow::ALL: {
            libdnf::rpm::PackageQuery available(base_query);
            available.filter_available();
            if (!showduplicates) {
                available.filter_priority();
                available.filter_latest_evr();
                // keep only those available packages that are either not installed or
                // available EVR is higher than the installed one
                libdnf::rpm::PackageQuery installed_latest(installed);
                installed_latest.filter_latest_evr();
                available.filter_nevra(installed_latest, libdnf::sack::QueryCmp::NOT | libdnf::sack::QueryCmp::LTE);
            }
            package_matched |= sections->add_section("Installed packages", installed, colorizer);
            package_matched |= sections->add_section("Available packages", available, colorizer);
            break;
        }
        case PkgNarrow::INSTALLED: {
            package_matched |= sections->add_section("Installed packages", installed, colorizer);
            break;
        }
        case PkgNarrow::AVAILABLE: {
            base_query.filter_available();
            if (!showduplicates) {
                base_query.filter_priority();
                base_query.filter_latest_evr();
            }
            package_matched |= sections->add_section("Available packages", base_query, colorizer);
            break;
        }
        case PkgNarrow::UPGRADES:
            base_query.filter_priority();
            base_query.filter_upgrades();
            base_query.filter_arch({"src", "nosrc"}, libdnf::sack::QueryCmp::NEQ);
            base_query.filter_latest_evr();
            package_matched |= sections->add_section("Available upgrades", base_query, colorizer);
            break;
        case PkgNarrow::OBSOLETES: {
            base_query.filter_priority();
            base_query.filter_obsoletes(installed);
            // prepare a map of obsoleted packages {obsoleter_id: [obsoleted_pkgs]}
            std::map<libdnf::rpm::PackageId, std::vector<libdnf::rpm::Package>> obsoletes;
            for (const auto & pkg : base_query) {
                std::vector<libdnf::rpm::Package> obsoleted;
                libdnf::rpm::PackageQuery obs_q(installed);
                obs_q.filter_provides(pkg.get_obsoletes());
                for (const auto & pkg_ob : obs_q) {
                    obsoleted.emplace_back(pkg_ob);
                }
                obsoletes.emplace(pkg.get_id(), obsoleted);
            }
            package_matched |= sections->add_section("Obsoleting packages", base_query, colorizer, obsoletes);
            break;
        }
        case PkgNarrow::AUTOREMOVE:
            installed.filter_unneeded();
            package_matched |= sections->add_section("Autoremove packages", installed, colorizer);
            break;
        case PkgNarrow::EXTRAS:
            base_query.filter_extras();
            package_matched |= sections->add_section("Extra packages", base_query, colorizer);
            break;
        case PkgNarrow::RECENT:
            base_query.filter_available();
            if (!showduplicates) {
                base_query.filter_priority();
                base_query.filter_latest_evr();
            }
            auto recent_limit_days = config.get_recent_option().get_value();
            auto now = time(NULL);
            base_query.filter_recent(now - (recent_limit_days * 86400));
            package_matched |= sections->add_section("Recently added packages", base_query, colorizer);
            break;
    }

    if (!package_matched && !pkg_specs.empty()) {
        throw libdnf::cli::CommandExitError(1, M_("No matching packages to list"));
    } else {
        sections->print();
    }
}


}  // namespace dnf5
