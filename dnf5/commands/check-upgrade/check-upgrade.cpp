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

    libdnf::rpm::PackageQuery full_package_query(ctx.base);
    libdnf::rpm::PackageQuery upgrades_query(ctx.base);

    if (!pkg_specs.empty()) {
        upgrades_query = libdnf::rpm::PackageQuery(ctx.base, libdnf::sack::ExcludeFlags::APPLY_EXCLUDES, true);
        libdnf::ResolveSpecSettings settings{.with_nevra = true, .with_provides = false, .with_filenames = false};
        for (const auto & spec : pkg_specs) {
            libdnf::rpm::PackageQuery package_query(full_package_query);
            package_query.resolve_pkg_spec(spec, settings, true);
            upgrades_query |= package_query;
        }
    }

    upgrades_query.filter_upgrades();

    auto sections = create_output();

    bool package_matched = sections->add_section("", upgrades_query);

    if (package_matched) {
        sections->print();

        if (changelogs->get_value()) {
            libdnf::cli::output::print_changelogs(upgrades_query, full_package_query, true, 0, 0);
        }
        throw libdnf::cli::SilentCommandExitError(100, M_(""));
    }
}

}  // namespace dnf5
