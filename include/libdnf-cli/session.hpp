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


namespace libdnf::cli::session {


class Command;


class Session {
public:
    /// @return Root command that represents the main program.
    ///         The returned pointer must **not** be freed manually.
    /// @since 5.0
    Command * get_root_command() { return root_command.get(); }

    /// Set `command` as the root command that represents the main program.
    /// Session becomes owner of the `command`.
    /// @since 5.0
    void set_root_command(std::unique_ptr<Command> command) { root_command = std::move(command); }

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
    libdnf::cli::ArgumentParser & get_argument_parser() { return argument_parser; }

private:
    std::unique_ptr<Command> root_command;
    Command * selected_command;
    libdnf::cli::ArgumentParser argument_parser;
};


class Command {
public:
    explicit Command(Command & parent, const std::string & name);
    explicit Command(Session & session, const std::string & program_name);
    virtual ~Command() = default;

    /// Run the command. Must be overriden in inherited classes.
    /// @since 5.0
    virtual void run() = 0;

    /// Throw a ArgumentParserMissingCommandError exception with the command name in it
    void throw_missing_command() const { throw ArgumentParserMissingCommandError(get_argument_parser_command()->get_id()); }

    /// @return Pointer to the Session.
    ///         The returned pointer must **not** be freed manually.
    /// @since 5.0
    Session & get_session() const noexcept { return session; }

    /// @return Pointer to the parent Command. Root command returns null because it has no parent.
    ///         The returned pointer must **not** be freed manually.
    /// @since 5.0
    Command * get_parent_command() const noexcept { return parent_command; }

    /// @return Pointer to the underlying argument parser command.
    ///         The returned pointer must **not** be freed manually.
    /// @since 5.0
    libdnf::cli::ArgumentParser::Command * get_argument_parser_command() const noexcept { return argument_parser_command; }

    /// @return List of subcommands owned by the current command.
    /// @since 5.0
    const std::vector<std::unique_ptr<Command>> & get_subcommands() const noexcept { return subcommands; }

    /// Register a `subcommand` to the current command.
    /// The command becomes owner of the `subcommand`.
    /// @since 5.0
    void register_subcommand(std::unique_ptr<Command> subcommand, libdnf::cli::ArgumentParser::Group * group = nullptr);

private:
    Session & session;
    Command * parent_command = nullptr;
    libdnf::cli::ArgumentParser::Command * argument_parser_command;
    std::vector<std::unique_ptr<Command>> subcommands;
};


class Option {
};


class BoolOption : public Option {
public:
    explicit BoolOption(libdnf::cli::session::Command & command, const std::string & long_name, char short_name, const std::string & desc, bool default_value);

    /// @return Parsed value.
    /// @since 5.0
    bool get_value() const { return conf->get_value(); }

// TODO(dmach): `arg` must be public, because it is used to define conflicting args
//protected:
    libdnf::OptionBool * conf{nullptr};
    libdnf::cli::ArgumentParser::NamedArg * arg{nullptr};
};


class StringArgumentList : public Option {
public:
    explicit StringArgumentList(libdnf::cli::session::Command & command, const std::string & name, const std::string & desc);

    /// @return Parsed value.
    /// @since 5.0
    std::vector<std::string> get_value() const;

protected:
    std::vector<std::unique_ptr<libdnf::Option>> * conf{nullptr};
    libdnf::cli::ArgumentParser::PositionalArg * arg{nullptr};
};


}  // libdnf::cli::session


#endif  // LIBDNF_CLI_SESSION_HPP
