// Copyright Contributors to the DNF5 project.
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


#ifndef LIBDNF5_CLI_SESSION_HPP
#define LIBDNF5_CLI_SESSION_HPP


#include "argument_parser.hpp"

#include "libdnf5-cli/defs.h"

#include <libdnf5/common/impl_ptr.hpp>
#include <libdnf5/conf/option_bool.hpp>
#include <libdnf5/conf/option_string.hpp>
#include <libdnf5/conf/option_string_list.hpp>


namespace libdnf5::cli::session {


class Command;


class LIBDNF_CLI_API Session {
public:
    explicit Session();
    ~Session();

    Session(const Session & src) = delete;
    Session(Session && src);

    Session & operator=(const Session & src) = delete;
    Session & operator=(Session && src);

    /// Store the command to the session and initialize it.
    /// @since 5.0
    void add_and_initialize_command(std::unique_ptr<Command> && command);

    /// @return Root command that represents the main program.
    ///         The returned pointer must **not** be freed manually.
    /// @since 5.0
    Command * get_root_command();

    /// Set `command` as the root command that represents the main program.
    /// @since 5.0
    void set_root_command(Command & command);

    /// @return Selected (sub)command that a user specified on the command line.
    ///         The returned pointer must **not** be freed manually.
    /// @since 5.0
    Command * get_selected_command();

    /// Set `command` as the selected (sub)command that a user specified on the command line.
    /// We're only pointing to a command that is owned by the Session already.
    /// @since 5.0
    void set_selected_command(Command * command);

    /// @return The underlying argument parser.
    /// @since 5.0
    libdnf5::cli::ArgumentParser & get_argument_parser();

    /// Remove all commands from the session and argument parser.
    /// @since 5.0
    void clear();

private:
    class LIBDNF_CLI_LOCAL Impl;
    ImplPtr<Impl> p_impl;
};


class LIBDNF_CLI_API Command : public libdnf5::cli::ArgumentParserUserData {
public:
    explicit Command(Session & session, const std::string & name);
    virtual ~Command();

    Command() = delete;
    Command(const Command & src) = delete;
    Command(Command && src) = delete;

    Command & operator=(const Command & src) = delete;
    Command & operator=(Command && src) = delete;

    /// Sets a parent command and group. Can add a new group to the parent command.
    /// @since 5.0
    virtual void set_parent_command();

    /// Set command arguments.
    /// @since 5.0
    virtual void set_argument_parser();

    /// Register subcommands.
    /// @since 5.0
    virtual void register_subcommands();

    /// Adjust configuration.
    /// Called after parsing the command line and but before loading configuration files.
    /// @since 5.0
    virtual void pre_configure();

    /// Adjust configuration.
    /// Called after parsing the command line and loading configuration files.
    /// @since 5.0
    virtual void configure();

    /// Loads additional packages that are not in the repositories.
    /// @since 5.0
    virtual void load_additional_packages();

    /// Run the command.
    /// @since 5.0
    virtual void run();

    /// Called immediately after the goal is resolved.
    /// @since 5.0
    virtual void goal_resolved();

    /// Throw a ArgumentParserMissingCommandError exception with the command name in it
    void throw_missing_command() const;

    /// @return Pointer to the Session.
    ///         The returned pointer must **not** be freed manually.
    /// @since 5.0
    Session & get_session() const noexcept;

    /// @return Pointer to the parent Command. Root command returns null because it has no parent.
    ///         The returned pointer must **not** be freed manually.
    /// @since 5.0
    Command * get_parent_command() const noexcept;

    /// @return Pointer to the underlying argument parser command.
    ///         The returned pointer must **not** be freed manually.
    /// @since 5.0
    libdnf5::cli::ArgumentParser::Command * get_argument_parser_command() const noexcept;

protected:
    /// Register a `subcommand` to the current command.
    /// The command becomes owner of the `subcommand`.
    /// @since 5.0
    void register_subcommand(
        std::unique_ptr<Command> subcommand, libdnf5::cli::ArgumentParser::Group * group = nullptr);

private:
    Session & session;
    libdnf5::cli::ArgumentParser::Command * argument_parser_command;
};


class LIBDNF_CLI_API Option {
public:
    Option(const Option & src) = delete;
    Option(Option && src) = delete;
    ~Option();

    Option & operator=(const Option & src) = delete;
    Option & operator=(Option && src) = delete;

protected:
    Option();
};


class LIBDNF_CLI_API BoolOption : public Option {
public:
    explicit BoolOption(
        libdnf5::cli::session::Command & command,
        const std::string & long_name,
        char short_name,
        const std::string & desc,
        bool default_value,
        libdnf5::OptionBool * linked_option = nullptr);

    ~BoolOption();

    /// @return Parsed value.
    /// @since 5.0
    bool get_value() const;

    /// Set bool value with priority for the option
    /// @param priority Priority
    /// @param value Value
    /// @since 5.0
    void set(libdnf5::Option::Priority priority, bool value);

    libdnf5::cli::ArgumentParser::NamedArg * get_arg();

protected:
    libdnf5::OptionBool * conf{nullptr};
    libdnf5::cli::ArgumentParser::NamedArg * arg{nullptr};
};


/// AppendStringListOption is a wrapper around NamedArg and OptionStringList
/// which allows specifying the argument multiple times and merging their values.
/// E.g. --whatrequires=tree --whatrequires=plant -> option contains: "tree, plant"
class LIBDNF_CLI_API AppendStringListOption : public Option {
public:
    explicit AppendStringListOption(
        libdnf5::cli::session::Command & command,
        const std::string & long_name,
        char short_name,
        const std::string & desc,
        const std::string & help);

    explicit AppendStringListOption(
        libdnf5::cli::session::Command & command,
        const std::string & long_name,
        char short_name,
        const std::string & desc,
        const std::string & help,
        const std::string & allowed_values_regex,
        const bool icase,
        const std::string & delimiters = libdnf5::OptionStringList::get_default_delimiters());

    ~AppendStringListOption();

    /// @return Parsed value.
    /// @since 5.0
    std::vector<std::string> get_value() const;

    libdnf5::cli::ArgumentParser::NamedArg * get_arg();

protected:
    libdnf5::OptionStringList * conf{nullptr};
    libdnf5::cli::ArgumentParser::NamedArg * arg{nullptr};
};


class LIBDNF_CLI_API StringArgumentList : public Option {
public:
    explicit StringArgumentList(
        libdnf5::cli::session::Command & command, const std::string & name, const std::string & desc, int nargs);
    explicit StringArgumentList(
        libdnf5::cli::session::Command & command, const std::string & name, const std::string & desc);

    ~StringArgumentList();

    /// @return Parsed value.
    /// @since 5.0
    std::vector<std::string> get_value() const;

    libdnf5::cli::ArgumentParser::PositionalArg * get_arg();

protected:
    std::vector<std::unique_ptr<libdnf5::Option>> * conf{nullptr};
    libdnf5::cli::ArgumentParser::PositionalArg * arg{nullptr};
};


}  // namespace libdnf5::cli::session


#endif  // LIBDNF5_CLI_SESSION_HPP
