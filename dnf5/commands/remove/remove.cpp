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

#include "remove.hpp"

#include <dnf5/shared_options.hpp>
#include <libdnf5-cli/argument_parser_errors.hpp>
#include <libdnf5/conf/option_number.hpp>
#include <libdnf5/rpm/package_query.hpp>
#include <libdnf5/rpm/package_sack.hpp>
#include <libdnf5/utils/bgettext/bgettext-mark.h>
#include <libdnf5/utils/format.hpp>

#include <map>

namespace dnf5 {

using namespace libdnf5::cli;

void RemoveCommand::set_parent_command() {
    auto * arg_parser_parent_cmd = get_session().get_argument_parser().get_root_command();
    auto * arg_parser_this_cmd = get_argument_parser_command();
    arg_parser_parent_cmd->register_command(arg_parser_this_cmd);
    arg_parser_parent_cmd->get_group("software_management_commands").register_argument(arg_parser_this_cmd);
}

void RemoveCommand::set_argument_parser() {
    auto & ctx = get_context();
    auto & parser = ctx.get_argument_parser();

    auto & cmd = *get_argument_parser_command();
    cmd.set_description(_("Remove (uninstall) software"));

    create_installed_from_repo_option(*this, installed_from_repos, true);

    auto noautoremove = parser.add_new_named_arg("no-autoremove");
    noautoremove->set_long_name("no-autoremove");
    noautoremove->set_description("Disable removal of dependencies that are no longer used");
    noautoremove->set_const_value("false");
    noautoremove->link_value(&ctx.get_base().get_config().get_clean_requirements_on_remove_option());
    cmd.register_named_arg(noautoremove);

    oldinstallonly = dynamic_cast<libdnf5::OptionBool *>(
        parser.add_init_value(std::unique_ptr<libdnf5::OptionBool>(new libdnf5::OptionBool(false))));
    auto oldinstallonly_opt = parser.add_new_named_arg("oldinstallonly");
    oldinstallonly_opt->set_long_name("oldinstallonly");
    oldinstallonly_opt->set_description("Remove old installonly packages");
    oldinstallonly_opt->set_const_value("true");
    oldinstallonly_opt->link_value(oldinstallonly);
    cmd.register_named_arg(oldinstallonly_opt);

    oldinstallonly_limit = dynamic_cast<libdnf5::OptionNumber<std::int32_t> *>(parser.add_init_value(
        std::unique_ptr<libdnf5::OptionNumber<std::int32_t>>(new libdnf5::OptionNumber<std::int32_t>(0))));
    auto limit_opt = parser.add_new_named_arg("limit");
    limit_opt->set_long_name("limit");
    limit_opt->set_has_value(true);
    limit_opt->set_arg_value_help("LIMIT");
    limit_opt->set_description(
        "Limit the number of installonly package versions to keep (must be >=1, used with --oldinstallonly)");
    limit_opt->link_value(oldinstallonly_limit);
    cmd.register_named_arg(limit_opt);

    auto keys = parser.add_new_positional_arg("specs", ArgumentParser::PositionalArg::UNLIMITED, nullptr, nullptr);
    keys->set_description("List of <package-spec-NF>|@<group-spec>|@<environment-spec> to remove");
    keys->set_parse_hook_func(
        [this]([[maybe_unused]] ArgumentParser::PositionalArg * arg, int argc, const char * const argv[]) {
            for (int i = 0; i < argc; ++i) {
                pkg_specs.emplace_back(argv[i]);
            }
            return true;
        });
    keys->set_complete_hook_func([&ctx](const char * arg) { return ctx.match_specs(arg, true, false, false, false); });
    cmd.register_positional_arg(keys);

    create_offline_option(*this);
    create_store_option(*this);
}

void RemoveCommand::configure() {
    auto & context = get_context();
    context.set_load_system_repo(true);
    context.set_load_available_repos(Context::LoadAvailableRepos::NONE);
}

void RemoveCommand::run() {
    auto & ctx = get_context();
    auto goal = ctx.get_goal();

    if (oldinstallonly->get_value()) {
        auto & base = ctx.get_base();
        auto & cfg_main = base.get_config();
        std::int32_t installonly_limit;

        if (oldinstallonly_limit->get_priority() >= libdnf5::Option::Priority::COMMANDLINE) {
            installonly_limit = oldinstallonly_limit->get_value();
            if (installonly_limit < 1) {
                throw libdnf5::cli::ArgumentParserInvalidValueError(
                    M_("Option '--limit' must be equal or greater than 1 (to keep at least the newest installed "
                       "package)."));
            }
        } else {
            installonly_limit = static_cast<std::int32_t>(cfg_main.get_installonly_limit_option().get_value());
        }

        libdnf5::rpm::PackageQuery installed_installonly(base);
        installed_installonly.filter_installed();
        installed_installonly.filter_installonly();

        // If package specs provided, filter to only those packages
        if (!pkg_specs.empty()) {
            libdnf5::rpm::PackageQuery filtered_query(base, libdnf5::sack::ExcludeFlags::APPLY_EXCLUDES, true);
            for (const auto & spec : pkg_specs) {
                libdnf5::rpm::PackageQuery spec_query(installed_installonly);
                spec_query.filter_name({spec}, libdnf5::sack::QueryCmp::GLOB);
                filtered_query |= spec_query;
            }
            installed_installonly = filtered_query;
        }

        // Detect the running kernel so we can skip it instead of causing an error.
        auto running_kernel = base.get_rpm_package_sack()->get_running_kernel();
        auto running_kernel_id = running_kernel.get_id();
        std::string running_kernel_epoch;
        std::string running_kernel_version;
        std::string running_kernel_release;
        std::string running_kernel_arch;
        if (running_kernel_id.id > 0) {
            running_kernel_epoch = running_kernel.get_epoch();
            running_kernel_version = running_kernel.get_version();
            running_kernel_release = running_kernel.get_release();
            running_kernel_arch = running_kernel.get_arch();
            running_kernel_nevra = running_kernel_version + "-" + running_kernel_release + "." + running_kernel_arch;
        }

        std::map<std::string, std::vector<libdnf5::rpm::Package>> packages_by_name;
        for (const auto & pkg : installed_installonly) {
            packages_by_name[pkg.get_name()].push_back(pkg);
        }

        for (auto & [name, packages] : packages_by_name) {
            if (installonly_limit > 0 && packages.size() > static_cast<size_t>(installonly_limit)) {
                // Sort packages newest first by inverting the comparator
                // cmp_nevra(a, b) returns true if a < b (ascending/oldest first)
                // So we swap arguments to get descending/newest first
                std::sort(packages.begin(), packages.end(), [](const auto & a, const auto & b) {
                    return libdnf5::rpm::cmp_nevra(b, a);
                });

                // Keep the first installonly_limit packages (newest), remove the rest (oldest)
                for (size_t i = static_cast<size_t>(installonly_limit); i < packages.size(); ++i) {
                    if (!running_kernel_version.empty() && packages[i].get_epoch() == running_kernel_epoch &&
                        packages[i].get_version() == running_kernel_version &&
                        packages[i].get_release() == running_kernel_release &&
                        packages[i].get_arch() == running_kernel_arch) {
                        running_kernel_skipped = true;
                        continue;
                    }
                    goal->add_rpm_remove(packages[i]);
                }
            }
        }
    } else {
        // Validate that --limit requires --oldinstallonly
        if (oldinstallonly_limit->get_value() > 0) {
            throw libdnf5::cli::ArgumentParserMissingDependentArgumentError(
                M_("Option '--limit' can only be used with '--oldinstallonly'."));
        }

        if (pkg_specs.empty()) {
            throw libdnf5::cli::ArgumentParserMissingCommandError(
                M_("Missing positional arguments for \"remove\" command. "
                   "Add \"--help\" for more information about the arguments."));
        }

        // Limit remove spec capabity to prevent multiple matches. Remove command should not match anything after performing
        // a remove action with the same spec. NEVRA and filenames are the only types that have no overlaps.
        libdnf5::GoalJobSettings settings;
        settings.set_from_repo_ids(installed_from_repos);
        settings.set_with_nevra(true);
        settings.set_with_provides(false);
        settings.set_with_filenames(true);
        settings.set_with_binaries(false);
        for (const auto & spec : pkg_specs) {
            goal->add_remove(spec, settings);
        }
    }

    // To enable removal of dependency packages it requires to use allow_erasing
    goal->set_allow_erasing(true);
}

void RemoveCommand::goal_resolved() {
    Command::goal_resolved();

    if (running_kernel_skipped) {
        auto & ctx = get_context();
        ctx.print_info(libdnf5::utils::sformat(
            _("Warning: The currently running kernel '{}' is not the latest installed version and was not removed. "
              "Please reboot into the latest kernel and run this command again to complete the cleanup."),
            running_kernel_nevra));
    }
}

}  // namespace dnf5
