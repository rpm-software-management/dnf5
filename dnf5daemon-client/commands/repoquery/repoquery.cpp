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

#include <dnf5daemon-server/dbus.hpp>
#include <fcntl.h>
#include <fmt/format.h>
#include <json-c/json.h>
#include <libdnf5-cli/exception.hpp>
#include <libdnf5-cli/output/adapters/package_tmpl.hpp>
#include <libdnf5-cli/output/packageinfo.hpp>
#include <libdnf5/conf/option_string.hpp>
#include <libdnf5/rpm/package.hpp>
#include <libdnf5/rpm/package_query.hpp>
#include <libdnf5/rpm/package_set.hpp>
#include <libdnf5/utils/bgettext/bgettext-lib.h>
#include <libdnf5/utils/bgettext/bgettext-mark-domain.h>
#include <limits.h>
#include <poll.h>

#include <array>
#include <iostream>
#include <string>

namespace dnfdaemon::client {

using namespace libdnf5::cli;

void RepoqueryCommand::set_parent_command() {
    auto * arg_parser_parent_cmd = get_session().get_argument_parser().get_root_command();
    auto * arg_parser_this_cmd = get_argument_parser_command();
    arg_parser_parent_cmd->register_command(arg_parser_this_cmd);
    arg_parser_parent_cmd->get_group("query_commands").register_argument(arg_parser_this_cmd);
}

void RepoqueryCommand::set_argument_parser() {
    auto & parser = get_context().get_argument_parser();
    auto & cmd = *get_argument_parser_command();

    available_option = dynamic_cast<libdnf5::OptionBool *>(
        parser.add_init_value(std::unique_ptr<libdnf5::OptionBool>(new libdnf5::OptionBool(true))));

    installed_option = dynamic_cast<libdnf5::OptionBool *>(
        parser.add_init_value(std::unique_ptr<libdnf5::OptionBool>(new libdnf5::OptionBool(false))));

    info_option = dynamic_cast<libdnf5::OptionBool *>(
        parser.add_init_value(std::unique_ptr<libdnf5::OptionBool>(new libdnf5::OptionBool(false))));

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
        parser.add_init_value(std::unique_ptr<libdnf5::Option>(new libdnf5::OptionString(nullptr))),
        patterns_options);
    keys->set_description("List of keys to match");

    cmd.set_description(_("search for packages matching various criteria"));

    cmd.register_named_arg(available);
    cmd.register_named_arg(installed);
    cmd.register_named_arg(info);
    cmd.register_positional_arg(keys);
}

dnfdaemon::KeyValueMap RepoqueryCommand::session_config() {
    dnfdaemon::KeyValueMap cfg = {};
    cfg["load_system_repo"] = sdbus::Variant(installed_option->get_value());
    cfg["load_available_repos"] = sdbus::Variant(
        (available_option->get_priority() >= libdnf5::Option::Priority::COMMANDLINE || !installed_option->get_value()));
    return cfg;
}

/// Read package objects from json stream. The `json_stream` string is modified, all
/// successfully parsed json objects are removed from it, possible partial json object
/// is left for next round of parsing after the rest of the string arrives.
std::vector<DbusPackageWrapper> json_to_packages(std::string & json_stream) {
    libdnf_assert(json_stream.size() < INT32_MAX, "can not parse JSON string longer then INT32_MAX");
    std::vector<DbusPackageWrapper> packages;

    json_tokener * tokener;
    tokener = json_tokener_new();
    while (!json_stream.empty()) {
        json_object * json_pkg =
            json_tokener_parse_ex(tokener, json_stream.c_str(), static_cast<int>(json_stream.size()));
        if (json_pkg) {
            dnfdaemon::KeyValueMap dbuspkg;
            json_object_object_foreach(json_pkg, key, val) {
                switch (json_object_get_type(val)) {
                    case json_type_boolean:
                        dbuspkg[key] = sdbus::Variant(static_cast<bool>(json_object_get_boolean(val)));
                        break;
                    case json_type_int:
                        dbuspkg[key] = sdbus::Variant(static_cast<uint64_t>(json_object_get_int64(val)));
                        break;
                    default:
                        dbuspkg[key] = sdbus::Variant(json_object_get_string(val));
                }
            }
            packages.emplace_back(DbusPackageWrapper(dbuspkg));

            auto parse_end = json_tokener_get_parse_end(tokener);
            json_tokener_reset(tokener);
            json_object_put(json_pkg);
            if (parse_end < json_stream.size()) {
                // more json objects on the line, remove parsed part and continue
                json_stream = json_stream.substr(parse_end);
            } else {
                // everything has been parsed
                json_stream.clear();
            }
        } else {
            auto err = json_tokener_get_error(tokener);
            if (err == json_tokener_continue) {
                // only partial json string in the input line, continue on the next one
                break;
            } else {
                // this is unrecoverable, throw error
                json_tokener_free(tokener);
                throw libdnf5::cli::CommandExitError(
                    1,
                    M_("Error parsing JSON object \"{}\": {}"),
                    json_stream,
                    std::string{json_tokener_error_desc(err)});
            }
        }
    }
    json_tokener_free(tokener);
    return packages;
}

void RepoqueryCommand::run() {
    auto & ctx = get_context();

    // query packages
    dnfdaemon::KeyValueMap options = {};

    std::vector<std::string> patterns;
    if (patterns_options->size() > 0) {
        patterns.reserve(patterns_options->size());
        for (auto & pattern : *patterns_options) {
            auto option = dynamic_cast<libdnf5::OptionString *>(pattern.get());
            patterns.emplace_back(option->get_value());
        }
    }
    options["patterns"] = sdbus::Variant(patterns);
    if (info_option->get_value()) {
        options.insert(std::pair<std::string, std::vector<std::string>>(
            "package_attrs",
            {"name",
             "epoch",
             "version",
             "release",
             "arch",
             "repo_id",
             "install_size",
             "download_size",
             "sourcerpm",
             "is_installed",
             "summary",
             "url",
             "license",
             "description",
             "vendor"}));
    } else {
        options.insert(std::pair<std::string, std::vector<std::string>>("package_attrs", {"full_nevra"}));
    }

    std::array<int, 2> pipefd{};  // File descriptors for the pipe
    if (pipe2(pipefd.data(), O_NONBLOCK) == -1) {
        std::cerr << "Error: failed to create a pipe." << std::endl;
        return;
    }

    ctx.session_proxy->callMethod("list_fd")
        .onInterface(dnfdaemon::INTERFACE_RPM)
        .withTimeout(static_cast<uint64_t>(-1))
        .withArguments(options)
        .withArguments(sdbus::UnixFd{pipefd[1]});
    close(pipefd[1]);

    int in_fd = pipefd[0];

    const auto pipe_buffer_size = static_cast<size_t>(fcntl(in_fd, F_GETPIPE_SZ));
    const int timeout = 30000;

    std::array<pollfd, 1> pfds;
    pfds[0].fd = in_fd;
    pfds[0].events = POLLIN;

    std::string message;

    std::vector<char> buffer(pipe_buffer_size);  // Buffer for reading data
    enum class STATUS { PENDING, FINISHED, FAILED } reading_status;
    reading_status = STATUS::PENDING;
    while (reading_status == STATUS::PENDING) {
        switch (poll(pfds.data(), pfds.size(), timeout)) {
            case -1:
                // poll() failed
                reading_status = STATUS::FAILED;
                std::cerr << "Error: poll() failed." << std::endl;
                break;
            case 0:
                // timeout was reached
                reading_status = STATUS::FAILED;
                std::cerr << "Error: timeout while waiting for data." << std::endl;
                break;
            default:
                if ((pfds[0].revents & POLLIN) != 0) {
                    const auto bytes_read = read(in_fd, buffer.data(), pipe_buffer_size);
                    if (bytes_read == -1) {
                        // read error
                        if (errno != EAGAIN && errno != EWOULDBLOCK) {
                            reading_status = STATUS::FAILED;
                            std::cerr << "Error: reading from the pipe failed." << std::endl;
                        }
                    } else if (bytes_read == 0) {
                        // EOF
                        reading_status = STATUS::FINISHED;
                    } else {
                        // data read
                        message.append(buffer.data(), static_cast<size_t>(bytes_read));
                        for (const auto & package : json_to_packages(message)) {
                            if (info_option->get_value()) {
                                libdnf5::cli::output::PackageAdapter cli_pkg(package);
                                libdnf5::cli::output::print_package_info(cli_pkg);
                                std::cout << '\n';
                            } else {
                                std::cout << package.get_full_nevra() << std::endl;
                            }
                        }
                    }
                } else if ((pfds[0].revents & POLLHUP) != 0) {
                    // EOF
                    reading_status = STATUS::FINISHED;
                } else {
                    reading_status = STATUS::FAILED;
                }
        }
    }
    close(in_fd);
}

}  // namespace dnfdaemon::client
