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

#include "list.hpp"

#include <dnf5/shared_options.hpp>
#include <libdnf5-cli/output/package_list_sections.hpp>
#include <libdnf5/rpm/package_query.hpp>
#include <libdnf5/utils/bgettext/bgettext-mark-domain.h>

namespace dnf5 {

using namespace libdnf5::cli;


void ListCommand::set_parent_command() {
    auto * arg_parser_parent_cmd = get_session().get_argument_parser().get_root_command();
    auto * arg_parser_this_cmd = get_argument_parser_command();
    arg_parser_parent_cmd->register_command(arg_parser_this_cmd);
    arg_parser_parent_cmd->get_group("query_commands").register_argument(arg_parser_this_cmd);
}

void ListCommand::set_argument_parser() {
    auto & ctx = get_context();
    auto & parser = ctx.get_argument_parser();

    auto & cmd = *get_argument_parser_command();
    cmd.set_description(_("Lists packages depending on the packages' relation to the system"));

    auto specs =
        parser.add_new_positional_arg("package-spec-NI", ArgumentParser::PositionalArg::UNLIMITED, nullptr, nullptr);
    specs->set_description(_("List of package-spec-NI to match (case insensitively)"));
    specs->set_parse_hook_func(
        [this]([[maybe_unused]] ArgumentParser::PositionalArg * arg, int argc, const char * const argv[]) {
            for (int i = 0; i < argc; ++i) {
                pkg_specs.emplace_back(argv[i]);
            }
            return true;
        });
    specs->set_complete_hook_func([&ctx](const char * arg) { return match_specs(ctx, arg, true, true, false, false); });
    cmd.register_positional_arg(specs);

    show_duplicates = std::make_unique<libdnf5::cli::session::BoolOption>(
        *this, "showduplicates", '\0', _("Show all versions of the packages, not only the latest ones."), false);

    create_installed_from_repo_option(*this, installed_from_repos, true);

    auto conflicts =
        parser.add_conflict_args_group(std::make_unique<std::vector<libdnf5::cli::ArgumentParser::Argument *>>());

    installed = std::make_unique<libdnf5::cli::session::BoolOption>(
        *this, "installed", '\0', _("List installed packages."), false);
    conflicts->push_back(installed->get_arg());

    available = std::make_unique<libdnf5::cli::session::BoolOption>(
        *this, "available", '\0', _("List available packages."), false);
    conflicts->push_back(available->get_arg());

    extras = std::make_unique<libdnf5::cli::session::BoolOption>(
        *this,
        "extras",
        '\0',
        _("List extras, that is packages installed on the system that are not available in any known repository."),
        false);
    conflicts->push_back(extras->get_arg());

    obsoletes = std::make_unique<libdnf5::cli::session::BoolOption>(
        *this,
        "obsoletes",
        '\0',
        _("List packages installed on the system that are obsoleted by packages in any known repository."),
        false);
    conflicts->push_back(obsoletes->get_arg());

    recent = std::make_unique<libdnf5::cli::session::BoolOption>(
        *this, "recent", '\0', _("List packages recently added into the repositories."), false);
    conflicts->push_back(recent->get_arg());

    upgrades = std::make_unique<libdnf5::cli::session::BoolOption>(
        *this, "upgrades", '\0', _("List upgrades available for the installed packages."), false);
    conflicts->push_back(upgrades->get_arg());

    autoremove = std::make_unique<libdnf5::cli::session::BoolOption>(
        *this, "autoremove", '\0', _("List packages which will be removed by the 'dnf autoremove' command."), false);
    conflicts->push_back(autoremove->get_arg());

    installed->get_arg()->set_conflict_arguments(conflicts);
    available->get_arg()->set_conflict_arguments(conflicts);
    extras->get_arg()->set_conflict_arguments(conflicts);
    obsoletes->get_arg()->set_conflict_arguments(conflicts);
    recent->get_arg()->set_conflict_arguments(conflicts);
    upgrades->get_arg()->set_conflict_arguments(conflicts);
    autoremove->get_arg()->set_conflict_arguments(conflicts);
}

void ListCommand::configure() {
    // TODO(mblaha): do not force expired metadata sync if not explicitly required
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

std::unique_ptr<libdnf5::cli::output::PackageListSections> ListCommand::create_output() {
    auto out = std::make_unique<libdnf5::cli::output::PackageListSections>();
    return out;
}

void ListCommand::run() {
    auto & ctx = get_context();
    auto & config = ctx.get_base().get_config();

    libdnf5::rpm::PackageQuery full_package_query(ctx.get_base(), libdnf5::sack::ExcludeFlags::IGNORE_VERSIONLOCK);
    libdnf5::rpm::PackageQuery base_query(ctx.get_base(), libdnf5::sack::ExcludeFlags::IGNORE_VERSIONLOCK);

    // pre-select by patterns
    if (!pkg_specs.empty()) {
        base_query = libdnf5::rpm::PackageQuery(ctx.get_base(), libdnf5::sack::ExcludeFlags::APPLY_EXCLUDES, true);
        libdnf5::ResolveSpecSettings settings;
        settings.set_ignore_case(true);
        settings.set_with_nevra(true);
        settings.set_with_provides(false);
        settings.set_with_filenames(false);
        settings.set_with_binaries(false);
        for (const auto & spec : pkg_specs) {
            libdnf5::rpm::PackageQuery pkg_query(full_package_query);
            pkg_query.resolve_pkg_spec(spec, settings, true);
            base_query |= pkg_query;
        }
    }

    // did any package match the criteria?
    bool package_matched = false;
    // output table
    auto sections = create_output();

    libdnf5::rpm::PackageQuery installed(base_query);
    if (installed_from_repos.empty()) {
        installed.filter_installed();
    } else {
        installed.filter_from_repo_id(installed_from_repos, libdnf5::sack::QueryCmp::GLOB);
    }

    // TODO(mblaha) currently only the installed version and upgrades
    // are highlighted to make the output a bit saner
    auto colorizer = std::make_unique<libdnf5::cli::output::PkgColorizer>(
        installed,
        "",  //config.get_color_list_available_install_option().get_value(),
        "",  //config.get_color_list_available_downgrade_option().get_value(),
        config.get_color_list_available_reinstall_option().get_value(),
        config.get_color_list_available_upgrade_option().get_value());

    switch (pkg_narrow) {
        case PkgNarrow::ALL: {
            libdnf5::rpm::PackageQuery available(base_query);
            available.filter_available();
            if (!show_duplicates->get_value()) {
                available.filter_priority();
                available.filter_latest_evr();
                // keep only those available packages that are either not installed or
                // available EVR is higher than the installed one
                libdnf5::rpm::PackageQuery installed_latest(installed);
                installed_latest.filter_latest_evr();
                available.filter_nevra(installed_latest, libdnf5::sack::QueryCmp::NOT | libdnf5::sack::QueryCmp::LTE);
            }
            package_matched |= sections->add_section(_("Installed packages"), installed);
            package_matched |= sections->add_section(_("Available packages"), available);
            break;
        }
        case PkgNarrow::INSTALLED: {
            package_matched |= sections->add_section(_("Installed packages"), installed);
            break;
        }
        case PkgNarrow::AVAILABLE: {
            base_query.filter_available();
            if (!show_duplicates->get_value()) {
                base_query.filter_priority();
                base_query.filter_latest_evr();
            }
            package_matched |= sections->add_section(_("Available packages"), base_query);
            break;
        }
        case PkgNarrow::UPGRADES:
            base_query.filter_priority();
            base_query.filter_upgrades();
            base_query.filter_arch(std::vector<std::string>{"src", "nosrc"}, libdnf5::sack::QueryCmp::NEQ);
            base_query.filter_latest_evr();
            package_matched |= sections->add_section(_("Available upgrades"), base_query);
            break;
        case PkgNarrow::OBSOLETES: {
            base_query.filter_priority();
            base_query.filter_obsoletes(installed);
            // prepare a map of obsoleted packages {obsoleter_id: [obsoleted_pkgs]}
            std::map<libdnf5::rpm::PackageId, std::vector<libdnf5::rpm::Package>> obsoletes;
            for (const auto & pkg : base_query) {
                std::vector<libdnf5::rpm::Package> obsoleted;
                libdnf5::rpm::PackageQuery obs_q(installed);
                obs_q.filter_provides(pkg.get_obsoletes());
                for (const auto & pkg_ob : obs_q) {
                    obsoleted.emplace_back(pkg_ob);
                }
                obsoletes.emplace(pkg.get_id(), obsoleted);
            }
            package_matched |= sections->add_section(_("Obsoleting packages"), base_query, obsoletes);
            break;
        }
        case PkgNarrow::AUTOREMOVE:
            installed.filter_unneeded();
            package_matched |= sections->add_section(_("Autoremove packages"), installed);
            break;
        case PkgNarrow::EXTRAS:
            base_query.filter_extras();
            package_matched |= sections->add_section(_("Extra packages"), base_query);
            break;
        case PkgNarrow::RECENT:
            base_query.filter_available();
            if (!show_duplicates->get_value()) {
                base_query.filter_priority();
                base_query.filter_latest_evr();
            }
            auto recent_limit_days = config.get_recent_option().get_value();
            auto now = time(NULL);
            base_query.filter_recent(now - (recent_limit_days * 86400));
            package_matched |= sections->add_section(_("Recently added packages"), base_query);
            break;
    }

    if (!package_matched && !pkg_specs.empty()) {
        throw libdnf5::cli::CommandExitError(1, M_("No matching packages to list"));
    } else {
        sections->print(colorizer);
    }
}


}  // namespace dnf5
