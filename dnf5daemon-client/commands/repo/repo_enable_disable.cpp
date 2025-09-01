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

#include "repo_enable_disable.hpp"

#include <fmt/format.h>

namespace dnfdaemon::client {

using namespace libdnf5::cli;

void RepoEnableDisableCommand::set_argument_parser() {
    auto & parser = get_context().get_argument_parser();
    auto & cmd = *get_argument_parser_command();

    patterns_options = parser.add_new_values();
    auto repos = parser.add_new_positional_arg(
        "repo_ids",
        libdnf5::cli::ArgumentParser::PositionalArg::UNLIMITED,
        parser.add_init_value(std::unique_ptr<libdnf5::Option>(new libdnf5::OptionString(nullptr))),
        patterns_options);

    repos->set_description(fmt::format("List of repos to be {}d", command));
    cmd.set_description(fmt::format("{} given repositories", command));
    cmd.register_positional_arg(repos);
}

void RepoEnableDisableCommand::run() {
    auto & ctx = get_context();

    std::vector<std::string> patterns;
    if (!patterns_options->empty()) {
        patterns.reserve(patterns_options->size());
        for (auto & pattern : *patterns_options) {
            auto option = dynamic_cast<libdnf5::OptionString *>(pattern.get());
            patterns.emplace_back(option->get_value());
        }
    }

    ctx.session_proxy->callMethod(command)
        .onInterface(dnfdaemon::INTERFACE_REPO)
        .withTimeout(static_cast<uint64_t>(-1))
        .withArguments(patterns);
}

}  // namespace dnfdaemon::client
