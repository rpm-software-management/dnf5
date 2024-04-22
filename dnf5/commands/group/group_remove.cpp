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

#include "group_remove.hpp"

#include <dnf5/shared_options.hpp>
#include <libdnf5/comps/group/group.hpp>
#include <libdnf5/comps/group/query.hpp>
#include <libdnf5/conf/const.hpp>

namespace dnf5 {

using namespace libdnf5::cli;

void GroupRemoveCommand::set_argument_parser() {
    auto & cmd = *get_argument_parser_command();
    cmd.set_description("Remove comp groups, including their packages");

    no_packages = std::make_unique<GroupNoPackagesOption>(*this);
    group_specs = std::make_unique<GroupSpecArguments>(*this, ArgumentParser::PositionalArg::AT_LEAST_ONE);
    create_offline_option(*this);
    create_store_option(*this);
}

void GroupRemoveCommand::configure() {
    auto & context = get_context();
    context.set_load_system_repo(true);
    context.set_load_available_repos(Context::LoadAvailableRepos::ENABLED);
    context.base.get_config().get_optional_metadata_types_option().add_item(
        libdnf5::Option::Priority::RUNTIME, libdnf5::METADATA_TYPE_COMPS);
}

void GroupRemoveCommand::run() {
    auto & ctx = get_context();
    auto goal = ctx.get_goal();

    libdnf5::GoalJobSettings settings;
    if (no_packages->get_value()) {
        settings.set_group_no_packages(true);
    }
    for (const auto & spec : group_specs->get_value()) {
        goal->add_group_remove(spec, libdnf5::transaction::TransactionItemReason::USER, settings);
    }

    // To enable removal of dependency packages it requires to use allow_erasing
    goal->set_allow_erasing(true);
}

}  // namespace dnf5
