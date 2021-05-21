/*
Copyright (C) 2020 Red Hat, Inc.

This file is part of dnfdaemon-client: https://github.com/rpm-software-management/libdnf/

Dnfdaemon-client is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Dnfdaemon-client is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with dnfdaemon-client.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "repoquery.hpp"

#include "../../context.hpp"
#include "../../wrappers/dbus_package_wrapper.hpp"

#include "libdnf-cli/output/repoquery.hpp"

#include <dnfdaemon-server/dbus.hpp>
#include <fmt/format.h>
#include <libdnf/conf/option_string.hpp>
#include <libdnf/rpm/package.hpp>
#include <libdnf/rpm/package_query.hpp>
#include <libdnf/rpm/package_set.hpp>

#include <iostream>

namespace dnfdaemon::client {

using namespace libdnf::cli;
void CmdRepoquery::set_argument_parser(Context & ctx) {
    available_option = dynamic_cast<libdnf::OptionBool *>(
        ctx.arg_parser.add_init_value(std::unique_ptr<libdnf::OptionBool>(new libdnf::OptionBool(true))));

    installed_option = dynamic_cast<libdnf::OptionBool *>(
        ctx.arg_parser.add_init_value(std::unique_ptr<libdnf::OptionBool>(new libdnf::OptionBool(false))));

    info_option = dynamic_cast<libdnf::OptionBool *>(
        ctx.arg_parser.add_init_value(std::unique_ptr<libdnf::OptionBool>(new libdnf::OptionBool(false))));

    auto available = ctx.arg_parser.add_new_named_arg("available");
    available->set_long_name("available");
    available->set_short_description("display available packages (default)");
    available->set_const_value("true");
    available->link_value(available_option);

    auto installed = ctx.arg_parser.add_new_named_arg("installed");
    installed->set_long_name("installed");
    installed->set_short_description("display installed packages");
    installed->set_const_value("true");
    installed->link_value(installed_option);

    auto info = ctx.arg_parser.add_new_named_arg("info");
    info->set_long_name("info");
    info->set_short_description("show detailed information about the packages");
    info->set_const_value("true");
    info->link_value(info_option);

    patterns_options = ctx.arg_parser.add_new_values();
    auto keys = ctx.arg_parser.add_new_positional_arg(
        "keys_to_match",
        ArgumentParser::PositionalArg::UNLIMITED,
        ctx.arg_parser.add_init_value(std::unique_ptr<libdnf::Option>(new libdnf::OptionString(nullptr))),
        patterns_options);
    keys->set_short_description("List of keys to match");

    auto repoquery = ctx.arg_parser.add_new_command("repoquery");
    repoquery->set_short_description("search for packages matching keyword");
    repoquery->set_description("");
    repoquery->set_named_args_help_header("Optional arguments:");
    repoquery->set_positional_args_help_header("Positional arguments:");
    repoquery->set_parse_hook_func([this, &ctx](
                                       [[maybe_unused]] ArgumentParser::Argument * arg,
                                       [[maybe_unused]] const char * option,
                                       [[maybe_unused]] int argc,
                                       [[maybe_unused]] const char * const argv[]) {
        ctx.select_command(this);
        return true;
    });

    repoquery->register_named_arg(available);
    repoquery->register_named_arg(installed);
    repoquery->register_named_arg(info);
    repoquery->register_positional_arg(keys);

    ctx.arg_parser.get_root_command()->register_command(repoquery);
}

dnfdaemon::KeyValueMap CmdRepoquery::session_config([[maybe_unused]] Context & ctx) {
    dnfdaemon::KeyValueMap cfg = {};
    cfg["load_system_repo"] = installed_option->get_value();
    cfg["load_available_repos"] =
        (available_option->get_priority() >= libdnf::Option::Priority::COMMANDLINE || !installed_option->get_value());
    return cfg;
}

void CmdRepoquery::run(Context & ctx) {
    // query packages
    dnfdaemon::KeyValueMap options = {};

    std::vector<std::string> patterns;
    if (patterns_options->size() > 0) {
        patterns.reserve(patterns_options->size());
        for (auto & pattern : *patterns_options) {
            auto option = dynamic_cast<libdnf::OptionString *>(pattern.get());
            patterns.emplace_back(option->get_value());
        }
    }
    options["patterns"] = patterns;
    if (info_option->get_value()) {
        options.insert(std::pair<std::string, std::vector<std::string>>(
            "package_attrs", {"name", "epoch", "version", "release", "arch", "repo"}));
    } else {
        options.insert(std::pair<std::string, std::vector<std::string>>("package_attrs", {"full_nevra"}));
    }

    dnfdaemon::KeyValueMapList packages;
    ctx.session_proxy->callMethod("list")
        .onInterface(dnfdaemon::INTERFACE_RPM)
        .withTimeout(static_cast<uint64_t>(-1))
        .withArguments(options)
        .storeResultsTo(packages);

    auto num_packages = packages.size();
    for (auto & raw_package : packages) {
        --num_packages;
        DbusPackageWrapper package(raw_package);
        if (info_option->get_value()) {
            // TODO(mblaha) use smartcols for this output
            libdnf::cli::output::print_package_info_table(package);
            if (num_packages) {
                std::cout << std::endl;
            }
        } else {
            std::cout << package.get_full_nevra() << std::endl;
        }
    }
}

}  // namespace dnfdaemon::client
