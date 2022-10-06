/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
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


#ifndef LIBDNF_CLI_SESSION_HPP
#define LIBDNF_CLI_SESSION_HPP


#include "argument_parser.hpp"

#include <libdnf/conf/option_bool.hpp>
#include <libdnf/conf/option_string.hpp>
#include <libdnf/conf/option_string_list.hpp>


namespace libdnf::cli::session {


class Command;


class Session {
public:
    Session() : argument_parser(new libdnf::cli::ArgumentParser) {}

    /// Store the command to the session and initialize it.
    /// @since 5.0
    void add_and_initialize_command(std::unique_ptr<Command> && command);

    /// @return Root command that represents the main program.
    ///         The returned pointer must **not** be freed manually.
    /// @since 5.0
    Command * get_root_command();

    /// Register `command` as the root command that represents the main program.
    /// Session becomes owner of the `command`.
    /// @since 5.0
    void register_root_command(std::unique_ptr<Command> && command);

    /// @return Selected (sub)command that a user specified on the command line.
    ///         The returned pointer must **not** be freed manually.
    /// @since 5.0
    Command * get_selected_command() { return selected_command; }

    /// Set `command` as the selected (sub)command that a user specified on the command line.
    /// We're only pointing to a command that is owned by the Session already.
    /// @since 5.0
    void set_selected_command(Command * command) { selected_command = command; }

    /// @return The underlying argument parser.
    /// @since 5.0
    libdnf::cli::ArgumentParser & get_argument_parser();

    /// Remove all commands from the session and argument parser.
    /// @since 5.0
    void clear();

private:
    Command * selected_command{nullptr};
    std::vector<std::unique_ptr<Command>> commands;
    std::unique_ptr<libdnf::cli::ArgumentParser> argument_parser;
};


class Command : public libdnf::cli::ArgumentParserUserData {
public:
    explicit Command(Command & parent, const std::string & name);
    explicit Command(Session & session, const std::string & name);
    virtual ~Command() = default;

    /// Set command arguments.
    /// @since 5.0
    virtual void set_argument_parser() {}

    /// Register subcommands.
    /// @since 5.0
    virtual void register_subcommands() {}

    /// Adjust configuration.
    /// Called after parsing the command line and but before loading configuration files.
    /// @since 5.0
    virtual void pre_configure() {}

    /// Adjust configuration.
    /// Called after parsing the command line and loading configuration files.
    /// @since 5.0
    virtual void configure() {}

    /// Loads additional packages that are not in the repositories.
    /// @since 5.0
    virtual void load_additional_packages() {}

    /// Run the command.
    /// @since 5.0
    virtual void run() {}

    /// Called immediately after the goal is resolved.
    /// @since 5.0
    virtual void goal_resolved() {}

    /// Throw a ArgumentParserMissingCommandError exception with the command name in it
    void throw_missing_command() const;

    /// @return Pointer to the Session.
    ///         The returned pointer must **not** be freed manually.
    /// @since 5.0
    Session & get_session() const noexcept { return session; }

    /// @return Pointer to the parent Command. Root command returns null because it has no parent.
    ///         The returned pointer must **not** be freed manually.
    /// @since 5.0
    Command * get_parent_command() const noexcept {
        return static_cast<Command *>(argument_parser_command->get_user_data());
    }

    /// @return Pointer to the underlying argument parser command.
    ///         The returned pointer must **not** be freed manually.
    /// @since 5.0
    libdnf::cli::ArgumentParser::Command * get_argument_parser_command() const noexcept {
        return argument_parser_command;
    }

protected:
    /// Register a `subcommand` to the current command.
    /// The command becomes owner of the `subcommand`.
    /// @since 5.0
    void register_subcommand(std::unique_ptr<Command> subcommand, libdnf::cli::ArgumentParser::Group * group = nullptr);

private:
    Session & session;
    libdnf::cli::ArgumentParser::Command * argument_parser_command;
};


inline Command * Session::get_root_command() {
    auto * arg_parser_root_command = argument_parser->get_root_command();
    return arg_parser_root_command ? static_cast<Command *>(arg_parser_root_command->get_user_data()) : nullptr;
}


class Option {};


class BoolOption : public Option {
public:
    explicit BoolOption(
        libdnf::cli::session::Command & command,
        const std::string & long_name,
        char short_name,
        const std::string & desc,
        bool default_value);

    /// @return Parsed value.
    /// @since 5.0
    bool get_value() const { return conf->get_value(); }

    /// @set bool value with priority for the option
    /// @since 5.0
    void set(libdnf::Option::Priority priority, bool value) { return conf->set(priority, value); }

    // TODO(dmach): `arg` must be public, because it is used to define conflicting args
    //protected:
    libdnf::OptionBool * conf{nullptr};
    libdnf::cli::ArgumentParser::NamedArg * arg{nullptr};
};


class StringListOption : public Option {
public:
    explicit StringListOption(
        libdnf::cli::session::Command & command,
        const std::string & long_name,
        char short_name,
        const std::string & desc,
        const std::string & help);

    explicit StringListOption(
        libdnf::cli::session::Command & command,
        const std::string & long_name,
        char short_name,
        const std::string & desc,
        const std::string & help,
        const std::string & allowed_values_regex,
        const bool icase);

    /// @return Parsed value.
    /// @since 5.0
    std::vector<std::string> get_value() const { return conf->get_value(); }

    libdnf::OptionStringList * conf{nullptr};
    libdnf::cli::ArgumentParser::NamedArg * arg{nullptr};
};


class StringArgumentList : public Option {
public:
    explicit StringArgumentList(
        libdnf::cli::session::Command & command, const std::string & name, const std::string & desc, int nargs);
    StringArgumentList(libdnf::cli::session::Command & command, const std::string & name, const std::string & desc)
        : StringArgumentList(command, name, desc, ArgumentParser::PositionalArg::UNLIMITED){};

    /// @return Parsed value.
    /// @since 5.0
    std::vector<std::string> get_value() const;

protected:
    std::vector<std::unique_ptr<libdnf::Option>> * conf{nullptr};
    libdnf::cli::ArgumentParser::PositionalArg * arg{nullptr};
};


}  // namespace libdnf::cli::session


#endif  // LIBDNF_CLI_SESSION_HPP
