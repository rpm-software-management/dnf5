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

#include "distro-sync.hpp"

#include <libdnf/conf/option_string.hpp>

namespace dnf5 {

using namespace libdnf::cli;

void DistroSyncCommand::set_argument_parser() {
    auto & ctx = get_context();
    auto & parser = ctx.get_argument_parser();

    auto & cmd = *get_argument_parser_command();
    cmd.set_description("Upgrade or downgrade installed software to the latest available versions");

    patterns_to_distro_sync_options = parser.add_new_values();
    auto patterns_arg = parser.add_new_positional_arg(
        "patterns",
        ArgumentParser::PositionalArg::UNLIMITED,
        parser.add_init_value(std::unique_ptr<libdnf::Option>(new libdnf::OptionString(nullptr))),
        patterns_to_distro_sync_options);
    patterns_arg->set_description("Patterns");
    cmd.register_positional_arg(patterns_arg);
}

void DistroSyncCommand::configure() {
    auto & context = get_context();
    context.set_load_system_repo(true);
    context.set_load_available_repos(Context::LoadAvailableRepos::ENABLED);
}

void DistroSyncCommand::run() {
    auto goal = get_context().get_goal();
    if (patterns_to_distro_sync_options->empty()) {
        goal->add_rpm_distro_sync();
    } else {
        for (auto & pattern : *patterns_to_distro_sync_options) {
            auto option = dynamic_cast<libdnf::OptionString *>(pattern.get());
            goal->add_rpm_distro_sync(option->get_value());
        }
    }
}

}  // namespace dnf5
