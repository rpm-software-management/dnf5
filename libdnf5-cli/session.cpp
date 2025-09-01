// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 2.1 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.


#include <libdnf5-cli/session.hpp>
#include <libdnf5/utils/bgettext/bgettext-mark-domain.h>


namespace libdnf5::cli::session {

class Session::Impl {
public:
    Impl() : argument_parser(new libdnf5::cli::ArgumentParser) {}

    void add_and_initialize_command(std::unique_ptr<Command> && command);

    Command * get_root_command() {
        auto * arg_parser_root_command = argument_parser->get_root_command();
        return arg_parser_root_command ? static_cast<Command *>(arg_parser_root_command->get_user_data()) : nullptr;
    }

    void set_root_command(Command & command) {
        argument_parser->set_root_command(command.get_argument_parser_command());
    }

    Command * get_selected_command() { return selected_command; }

    void set_selected_command(Command * command) { selected_command = command; }

    libdnf5::cli::ArgumentParser & get_argument_parser() { return *argument_parser; }

    void clear() {
        for (auto & cmd : commands) {
            cmd.reset();
        }
        argument_parser.reset();
    }

private:
    Command * selected_command{nullptr};
    std::vector<std::unique_ptr<Command>> commands;
    std::unique_ptr<libdnf5::cli::ArgumentParser> argument_parser;
};


void Session::Impl::add_and_initialize_command(std::unique_ptr<Command> && command) {
    auto * arg_parser_command = command->get_argument_parser_command();

    // set the command as selected when parsed
    arg_parser_command->set_parse_hook_func([](ArgumentParser::Argument * arg,
                                               [[maybe_unused]] const char * option,
                                               [[maybe_unused]] int argc,
                                               [[maybe_unused]] const char * const argv[]) {
        // set the selected command only if a subcommand hasn't set it already
        auto * command = static_cast<Command *>(arg->get_user_data());
        auto & session = command->get_session();
        if (!session.get_selected_command()) {
            session.set_selected_command(command);
        }
        return true;
    });

    command->set_parent_command();
    command->set_argument_parser();
    command->register_subcommands();
    commands.push_back(std::move(command));
}


Session::Session() : p_impl{new Impl} {}

Session::~Session() = default;

Session::Session(Session && src) = default;

Session & Session::operator=(Session && src) = default;

void Session::add_and_initialize_command(std::unique_ptr<Command> && command) {
    p_impl->add_and_initialize_command(std::move(command));
}

Command * Session::get_root_command() {
    return p_impl->get_root_command();
}

void Session::set_root_command(Command & command) {
    p_impl->set_root_command(command);
}

Command * Session::get_selected_command() {
    return p_impl->get_selected_command();
}

void Session::set_selected_command(Command * command) {
    p_impl->set_selected_command(command);
}

libdnf5::cli::ArgumentParser & Session::get_argument_parser() {
    return p_impl->get_argument_parser();
}

void Session::clear() {
    p_impl->clear();
}


Command::Command(Session & session, const std::string & name) : session{session} {
    // create a new argument parser command (owned by arg_parser)
    argument_parser_command = session.get_argument_parser().add_new_command(name);
    // set a backlink from the argument parser command to this command
    argument_parser_command->set_user_data(this);
}

Command::~Command() = default;

void Command::set_parent_command() {}

void Command::set_argument_parser() {}

void Command::register_subcommands() {}

void Command::pre_configure() {}

void Command::configure() {}

void Command::load_additional_packages() {}

void Command::run() {}

void Command::goal_resolved() {}

void Command::throw_missing_command() const {
    throw ArgumentParserMissingCommandError(M_("Missing command"), get_argument_parser_command()->get_id());
}
Session & Command::get_session() const noexcept {
    return session;
}

Command * Command::get_parent_command() const noexcept {
    auto * parser_parent = argument_parser_command->get_parent();
    if (!parser_parent)
        return nullptr;
    return static_cast<Command *>(parser_parent->get_user_data());
}

libdnf5::cli::ArgumentParser::Command * Command::get_argument_parser_command() const noexcept {
    return argument_parser_command;
}

void Command::register_subcommand(std::unique_ptr<Command> subcommand, libdnf5::cli::ArgumentParser::Group * group) {
    auto * sub_arg_parser_command = subcommand->get_argument_parser_command();
    get_argument_parser_command()->register_command(sub_arg_parser_command);

    if (group) {
        group->register_argument(sub_arg_parser_command);
    }

    session.add_and_initialize_command(std::move(subcommand));
}


Option::Option() = default;

Option::~Option() = default;


BoolOption::BoolOption(
    libdnf5::cli::session::Command & command,
    const std::string & long_name,
    char short_name,
    const std::string & desc,
    bool default_value,
    libdnf5::OptionBool * linked_option) {
    auto & parser = command.get_session().get_argument_parser();
    arg = parser.add_new_named_arg(long_name);

    if (!long_name.empty()) {
        arg->set_long_name(long_name);
    }

    if (short_name) {
        arg->set_short_name(short_name);
    }

    arg->set_description(desc);
    arg->set_const_value(default_value ? "false" : "true");
    if (linked_option) {
        conf = linked_option;
    } else {
        conf = dynamic_cast<libdnf5::OptionBool *>(
            parser.add_init_value(std::make_unique<libdnf5::OptionBool>(default_value)));
    }
    arg->link_value(conf);

    command.get_argument_parser_command()->register_named_arg(arg);
}

BoolOption::~BoolOption() = default;

bool BoolOption::get_value() const {
    return conf->get_value();
}

void BoolOption::set(libdnf5::Option::Priority priority, bool value) {
    return conf->set(priority, value);
}

libdnf5::cli::ArgumentParser::NamedArg * BoolOption::get_arg() {
    return arg;
}


AppendStringListOption::AppendStringListOption(
    libdnf5::cli::session::Command & command,
    const std::string & long_name,
    char short_name,
    const std::string & desc,
    const std::string & help,
    const std::string & allowed_values_regex,
    const bool icase,
    const std::string & delimiters) {
    auto & parser = command.get_session().get_argument_parser();
    conf = dynamic_cast<libdnf5::OptionStringList *>(parser.add_init_value(std::make_unique<libdnf5::OptionStringList>(
        std::vector<std::string>(), allowed_values_regex, icase, delimiters)));
    arg = parser.add_new_named_arg(long_name);

    if (!long_name.empty()) {
        arg->set_long_name(long_name);
    }

    if (short_name) {
        arg->set_short_name(short_name);
    }

    arg->set_description(desc);
    arg->set_arg_value_help(help);
    arg->set_has_value(true);
    arg->set_parse_hook_func(
        [this](
            [[maybe_unused]] ArgumentParser::NamedArg * arg, [[maybe_unused]] const char * option, const char * value) {
            conf->add(libdnf5::Option::Priority::COMMANDLINE, value);
            return true;
        });

    command.get_argument_parser_command()->register_named_arg(arg);
}

AppendStringListOption::AppendStringListOption(
    libdnf5::cli::session::Command & command,
    const std::string & long_name,
    char short_name,
    const std::string & desc,
    const std::string & help)
    : AppendStringListOption(command, long_name, short_name, desc, help, "", false) {}

AppendStringListOption::~AppendStringListOption() = default;

std::vector<std::string> AppendStringListOption::get_value() const {
    return conf->get_value();
}

libdnf5::cli::ArgumentParser::NamedArg * AppendStringListOption::get_arg() {
    return arg;
}

StringArgumentList::StringArgumentList(
    libdnf5::cli::session::Command & command, const std::string & name, const std::string & desc, int nargs) {
    auto & parser = command.get_session().get_argument_parser();

    conf = parser.add_new_values();
    arg = parser.add_new_positional_arg(
        name, nargs, parser.add_init_value(std::make_unique<libdnf5::OptionString>(nullptr)), conf);
    arg->set_description(desc);

    command.get_argument_parser_command()->register_positional_arg(arg);
}

StringArgumentList::StringArgumentList(
    libdnf5::cli::session::Command & command, const std::string & name, const std::string & desc)
    : StringArgumentList(command, name, desc, ArgumentParser::PositionalArg::UNLIMITED) {};

StringArgumentList::~StringArgumentList() = default;

std::vector<std::string> StringArgumentList::get_value() const {
    std::vector<std::string> result;

    for (auto & opt : *conf) {
        auto string_opt = dynamic_cast<libdnf5::OptionString *>(opt.get());
        result.emplace_back(string_opt->get_value());
    }

    return result;
}

libdnf5::cli::ArgumentParser::PositionalArg * StringArgumentList::get_arg() {
    return arg;
}


}  // namespace libdnf5::cli::session
