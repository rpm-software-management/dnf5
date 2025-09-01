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

#include "group_upgrade.hpp"

#include <dnf5/shared_options.hpp>
#include <libdnf5/comps/group/group.hpp>
#include <libdnf5/comps/group/query.hpp>
#include <libdnf5/conf/const.hpp>

#include <iostream>

namespace dnf5 {

using namespace libdnf5::cli;

void GroupUpgradeCommand::set_argument_parser() {
    auto & cmd = *get_argument_parser_command();
    cmd.set_description("Upgrade comp groups, including their packages");

    group_specs = std::make_unique<GroupSpecArguments>(*this, ArgumentParser::PositionalArg::AT_LEAST_ONE);

    allow_erasing = std::make_unique<AllowErasingOption>(*this);
    auto skip_unavailable = std::make_unique<SkipUnavailableOption>(*this);
    create_allow_downgrade_options(*this);
    create_downloadonly_option(*this);
    create_offline_option(*this);
    create_store_option(*this);
}

void GroupUpgradeCommand::configure() {
    auto & context = get_context();
    context.set_load_system_repo(true);
    context.set_load_available_repos(Context::LoadAvailableRepos::ENABLED);
    context.get_base().get_config().get_optional_metadata_types_option().add_item(
        libdnf5::Option::Priority::RUNTIME, libdnf5::METADATA_TYPE_COMPS);
}

void GroupUpgradeCommand::run() {
    auto & ctx = get_context();
    auto goal = ctx.get_goal();
    goal->set_allow_erasing(allow_erasing->get_value());

    libdnf5::GoalJobSettings settings;
    for (const auto & spec : group_specs->get_value()) {
        goal->add_group_upgrade(spec, settings);
    }
}

}  // namespace dnf5
