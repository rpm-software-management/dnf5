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

#include "group_install.hpp"

#include <dnf5/shared_options.hpp>
#include <libdnf5/comps/group/group.hpp>
#include <libdnf5/comps/group/query.hpp>
#include <libdnf5/conf/const.hpp>

#include <iostream>

namespace dnf5 {

using namespace libdnf5::cli;

void GroupInstallCommand::set_argument_parser() {
    auto & cmd = *get_argument_parser_command();
    cmd.set_description("Install comp environments or groups, including their packages");

    with_optional = std::make_unique<GroupWithOptionalOption>(*this);
    no_packages = std::make_unique<GroupNoPackagesOption>(*this);
    group_specs = std::make_unique<CompsSpecArguments>(*this, ArgumentParser::PositionalArg::AT_LEAST_ONE);

    allow_erasing = std::make_unique<AllowErasingOption>(*this);
    auto skip_broken = std::make_unique<SkipBrokenOption>(*this);
    auto skip_unavailable = std::make_unique<SkipUnavailableOption>(*this);
    create_allow_downgrade_options(*this);
    create_downloadonly_option(*this);
    create_offline_option(*this);
    create_store_option(*this);
}

void GroupInstallCommand::configure() {
    auto & context = get_context();
    context.set_load_system_repo(true);
    context.set_load_available_repos(Context::LoadAvailableRepos::ENABLED);
    context.get_base().get_config().get_optional_metadata_types_option().add_item(
        libdnf5::Option::Priority::RUNTIME, libdnf5::METADATA_TYPE_COMPS);
}

void GroupInstallCommand::run() {
    auto & ctx = get_context();
    auto goal = ctx.get_goal();
    goal->set_allow_erasing(allow_erasing->get_value());

    libdnf5::GoalJobSettings settings;
    if (no_packages->get_value()) {
        settings.set_group_no_packages(true);
    }
    if (with_optional->get_value()) {
        auto group_package_types = libdnf5::comps::package_type_from_string(
            ctx.get_base().get_config().get_group_package_types_option().get_value());
        settings.set_group_package_types(group_package_types | libdnf5::comps::PackageType::OPTIONAL);
    }
    settings.set_comps_type_preferred(get_comps_type_preferred());
    for (const auto & spec : group_specs->get_value()) {
        goal->add_group_install(spec, libdnf5::transaction::TransactionItemReason::USER, settings);
    }
}

}  // namespace dnf5
