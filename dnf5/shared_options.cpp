/*
Copyright Contributors to the dnf5 project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Dnf5 is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2.1 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/


#include "dnf5/shared_options.hpp"

#include "dnf5/offline.hpp"

#include "libdnf5/utils/bgettext/bgettext-mark-domain.h"

#include <libdnf5/rpm/arch.hpp>

namespace dnf5 {


void create_allow_downgrade_options(dnf5::Command & command) {
    auto * solver_options = command.get_context().get_argument_parser().add_new_group("solver_options");
    solver_options->set_header("Solver Options:");
    auto * parser_command = command.get_argument_parser_command();
    parser_command->register_group(solver_options);

    auto & parser = command.get_context().get_argument_parser();
    auto allow_downgrade = parser.add_new_named_arg("allow-downgrade");
    allow_downgrade->set_long_name("allow-downgrade");
    allow_downgrade->set_description("Allow downgrade of dependencies for resolve of requested operation");
    allow_downgrade->set_const_value("true");
    allow_downgrade->link_value(&command.get_context().get_base().get_config().get_allow_downgrade_option());
    parser_command->register_named_arg(allow_downgrade);

    auto no_allow_downgrade = parser.add_new_named_arg("no-allow-downgrade");
    no_allow_downgrade->set_long_name("no-allow-downgrade");
    no_allow_downgrade->set_description("Disable downgrade of dependencies for resolve of requested operation");
    no_allow_downgrade->set_const_value("false");
    no_allow_downgrade->link_value(&command.get_context().get_base().get_config().get_allow_downgrade_option());
    parser_command->register_named_arg(no_allow_downgrade);

    allow_downgrade->add_conflict_argument(*no_allow_downgrade);
}

void create_destdir_option(dnf5::Command & command) {
    auto & parser = command.get_context().get_argument_parser();
    auto destdir = parser.add_new_named_arg("destdir");
    destdir->set_long_name("destdir");
    destdir->set_description(
        "Set directory used for downloading packages to. Default location is to the current working directory.");
    destdir->set_has_value(true);
    destdir->set_arg_value_help("DESTDIR");
    destdir->link_value(&command.get_context().get_base().get_config().get_destdir_option());
    command.get_argument_parser_command()->register_named_arg(destdir);
}

void create_downloadonly_option(dnf5::Command & command) {
    auto & parser = command.get_context().get_argument_parser();
    auto downloadonly = parser.add_new_named_arg("downloadonly");
    downloadonly->set_long_name("downloadonly");
    downloadonly->set_description("Only download packages for a transaction");
    downloadonly->set_const_value("true");
    downloadonly->link_value(&command.get_context().get_base().get_config().get_downloadonly_option());
    command.get_argument_parser_command()->register_named_arg(downloadonly);
}

void create_offline_option(dnf5::Command & command) {
    auto & ctx = command.get_context();
    auto & parser = command.get_context().get_argument_parser();
    auto offline = parser.add_new_named_arg("offline");
    offline->set_long_name("offline");
    offline->set_description("Store the transaction to be performed offline");
    offline->set_const_value("true");
    offline->set_parse_hook_func([&ctx](
                                     [[maybe_unused]] libdnf5::cli::ArgumentParser::NamedArg * arg,
                                     [[maybe_unused]] const char * option,
                                     [[maybe_unused]] const char * value) {
        // The installroot will be prepended to the cachedir and system_cachedir later in Base.setup()
        const auto & offline_datadir = dnf5::offline::DEFAULT_DATADIR;
        ctx.get_base().get_config().get_system_cachedir_option().set(
            libdnf5::Option::Priority::INSTALLROOT, offline_datadir);
        ctx.get_base().get_config().get_cachedir_option().set(libdnf5::Option::Priority::INSTALLROOT, offline_datadir);
        ctx.set_should_store_offline(true);
        return true;
    });
    command.get_argument_parser_command()->register_named_arg(offline);
}

void create_store_option(dnf5::Command & command) {
    auto & ctx = command.get_context();
    auto & parser = ctx.get_argument_parser();
    auto * store = parser.add_new_named_arg("store");
    store->set_long_name("store");
    store->set_has_value(true);
    store->set_description("Store the current transaction in a directory at the specified path instead of running it.");
    store->set_arg_value_help("STORED_TRANSACTION_PATH");
    command.get_argument_parser_command()->register_named_arg(store);
    store->set_parse_hook_func([&ctx](
                                   [[maybe_unused]] libdnf5::cli::ArgumentParser::NamedArg * arg,
                                   [[maybe_unused]] const char * option,
                                   const char * value) {
        ctx.transaction_store_path = value;
        return true;
    });
}


}  // namespace dnf5
