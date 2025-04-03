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

#include "clean.hpp"

#include "commands/shared_options.hpp"
#include "context.hpp"
#include "exception.hpp"
#include "utils/auth.hpp"

#include <dnf5daemon-server/dbus.hpp>
#include <libdnf5/conf/option_string.hpp>
#include <libdnf5/utils/bgettext/bgettext-mark-domain.h>

#include <iostream>
#include <memory>

namespace dnfdaemon::client {

using namespace libdnf5::cli;

void CleanCommand::set_parent_command() {
    auto * arg_parser_parent_cmd = get_session().get_argument_parser().get_root_command();
    auto * arg_parser_this_cmd = get_argument_parser_command();
    arg_parser_parent_cmd->register_command(arg_parser_this_cmd);
    arg_parser_parent_cmd->get_group("software_management_commands").register_argument(arg_parser_this_cmd);
}

void CleanCommand::set_argument_parser() {
    auto & parser = get_context().get_argument_parser();
    auto & cmd = *get_argument_parser_command();

    cmd.set_description("Remove or expire cached data");

    cache_types = parser.add_new_values();
    auto cache_types_arg = parser.add_new_positional_arg(
        "cache_types",
        1,
        parser.add_init_value(std::unique_ptr<libdnf5::Option>(new libdnf5::OptionString(nullptr))),
        cache_types);
    cache_types_arg->set_description("Cache type to clean up");
    cmd.register_positional_arg(cache_types_arg);
}

void CleanCommand::run() {
    auto & ctx = get_context();

    if (!libdnf5::utils::am_i_root()) {
        throw UnprivilegedUserError();
    }

    const std::string cache_type = dynamic_cast<libdnf5::OptionString *>((*cache_types)[0].get())->get_value();
    bool success;
    std::string output_msg;
    ctx.session_proxy->callMethod("clean")
        .onInterface(dnfdaemon::INTERFACE_BASE)
        .withTimeout(static_cast<uint64_t>(-1))
        .withArguments(cache_type)
        .storeResultsTo(success, output_msg);
    // make it compatible with `dnf5 clean metadata` which also cleans solv files
    if (success && (cache_type == "metadata")) {
        ctx.session_proxy->callMethod("clean")
            .onInterface(dnfdaemon::INTERFACE_BASE)
            .withTimeout(static_cast<uint64_t>(-1))
            .withArguments("dbcache")
            .storeResultsTo(success, output_msg);
    }

    if (success) {
        std::cout << output_msg << std::endl;
    } else {
        throw libdnf5::cli::CommandExitError(1, M_("Error cleaning the cache: {}"), output_msg);
    }
}

}  // namespace dnfdaemon::client
