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

#include "group_install.hpp"

#include <dnf5/shared_options.hpp>
#include <libdnf/comps/comps.hpp>
#include <libdnf/comps/group/group.hpp>
#include <libdnf/comps/group/query.hpp>
#include <libdnf/conf/const.hpp>

#include <iostream>

namespace dnf5 {

using namespace libdnf::cli;

void GroupInstallCommand::set_argument_parser() {
    auto & cmd = *get_argument_parser_command();
    cmd.set_description("Install comp groups, including their packages");

    with_optional = std::make_unique<GroupWithOptionalOption>(*this);
    no_packages = std::make_unique<GroupNoPackagesOption>(*this);
    group_specs = std::make_unique<GroupSpecArguments>(*this, ArgumentParser::PositionalArg::AT_LEAST_ONE);

    auto skip_unavailable = std::make_unique<SkipUnavailableOption>(*this);
    auto skip_broken = std::make_unique<SkipBrokenOption>(*this);
    create_allow_downgrade_options(*this);
}

void GroupInstallCommand::configure() {
    auto & context = get_context();
    context.set_load_system_repo(true);
    context.set_load_available_repos(Context::LoadAvailableRepos::ENABLED);
    context.base.get_config().get_optional_metadata_types_option().add_item(
        libdnf::Option::Priority::RUNTIME, libdnf::METADATA_TYPE_COMPS);
}

void GroupInstallCommand::run() {
    auto & ctx = get_context();
    auto goal = ctx.get_goal();

    libdnf::GoalJobSettings settings;
    if (no_packages->get_value()) {
        settings.group_no_packages = true;
    }
    if (with_optional->get_value()) {
        auto group_package_types =
            libdnf::comps::package_type_from_string(ctx.base.get_config().get_group_package_types_option().get_value());
        settings.set_group_package_types(group_package_types | libdnf::comps::PackageType::OPTIONAL);
    }
    for (const auto & spec : group_specs->get_value()) {
        goal->add_group_install(spec, libdnf::transaction::TransactionItemReason::USER, settings);
    }
}

}  // namespace dnf5
