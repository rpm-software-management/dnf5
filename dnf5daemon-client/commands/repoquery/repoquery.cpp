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

#include "repoquery.hpp"

#include "context.hpp"
#include "wrappers/dbus_package_wrapper.hpp"

#include "libdnf-cli/output/repoquery.hpp"

#include <dnf5daemon-server/dbus.hpp>
#include <fmt/format.h>
#include <libdnf/conf/option_string.hpp>
#include <libdnf/rpm/package.hpp>
#include <libdnf/rpm/package_query.hpp>
#include <libdnf/rpm/package_set.hpp>

#include <iostream>

namespace dnfdaemon::client {

using namespace libdnf::cli;

RepoqueryCommand::RepoqueryCommand(Command & parent) : DaemonCommand(parent, "repoquery") {
    auto & ctx = static_cast<Context &>(get_session());
    auto & parser = ctx.get_argument_parser();
    auto & cmd = *get_argument_parser_command();

    available_option = dynamic_cast<libdnf::OptionBool *>(
        parser.add_init_value(std::unique_ptr<libdnf::OptionBool>(new libdnf::OptionBool(true))));

    installed_option = dynamic_cast<libdnf::OptionBool *>(
        parser.add_init_value(std::unique_ptr<libdnf::OptionBool>(new libdnf::OptionBool(false))));

    info_option = dynamic_cast<libdnf::OptionBool *>(
        parser.add_init_value(std::unique_ptr<libdnf::OptionBool>(new libdnf::OptionBool(false))));

    auto available = parser.add_new_named_arg("available");
    available->set_long_name("available");
    available->set_description("display available packages (default)");
    available->set_const_value("true");
    available->link_value(available_option);

    auto installed = parser.add_new_named_arg("installed");
    installed->set_long_name("installed");
    installed->set_description("display installed packages");
    installed->set_const_value("true");
    installed->link_value(installed_option);

    auto info = parser.add_new_named_arg("info");
    info->set_long_name("info");
    info->set_description("show detailed information about the packages");
    info->set_const_value("true");
    info->link_value(info_option);

    patterns_options = parser.add_new_values();
    auto keys = parser.add_new_positional_arg(
        "keys_to_match",
        ArgumentParser::PositionalArg::UNLIMITED,
        parser.add_init_value(std::unique_ptr<libdnf::Option>(new libdnf::OptionString(nullptr))),
        patterns_options);
    keys->set_description("List of keys to match");

    cmd.set_description("search for packages matching various criteria");

    cmd.register_named_arg(available);
    cmd.register_named_arg(installed);
    cmd.register_named_arg(info);
    cmd.register_positional_arg(keys);
}

dnfdaemon::KeyValueMap RepoqueryCommand::session_config() {
    dnfdaemon::KeyValueMap cfg = {};
    cfg["load_system_repo"] = installed_option->get_value();
    cfg["load_available_repos"] =
        (available_option->get_priority() >= libdnf::Option::Priority::COMMANDLINE || !installed_option->get_value());
    return cfg;
}

void RepoqueryCommand::run() {
    auto & ctx = static_cast<Context &>(get_session());

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
            "package_attrs",
            {"name",
             "epoch",
             "version",
             "release",
             "arch",
             "repo",
             "install_size",
             "package_size",
             "sourcerpm",
             "is_installed",
             "summary",
             "url",
             "license",
             "description"}));
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
