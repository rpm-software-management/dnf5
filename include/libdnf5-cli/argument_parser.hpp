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

#ifndef LIBDNF5_CLI_ARGUMENT_PARSER_HPP
#define LIBDNF5_CLI_ARGUMENT_PARSER_HPP

#include "argument_parser_errors.hpp"

#include "libdnf5-cli/defs.h"

#include <libdnf5/conf/option.hpp>

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace libdnf5::cli {

/// Base class for user data used in ArgumentParser::Argument
class ArgumentParserUserData {};

class LIBDNF_CLI_API ArgumentParser {
public:
    class Argument;

    class Group {
    public:
        Group() = delete;
        Group(const Group &) = delete;
        Group(Group &&) = delete;
        Group & operator=(const Group &) = delete;
        Group & operator=(Group &&) = delete;

        ~Group();

        /// Sets group header.
        void set_header(std::string header) noexcept;

        /// Gets group id.
        const std::string & get_id() const noexcept;

        /// Gets group header.
        const std::string & get_header() const noexcept;

        /// Registers an argument to the group.
        /// An exception is thrown when an argument with the same ID is already registered.
        void register_argument(Argument * arg);

        /// Gets a list of registered arguments.
        const std::vector<Argument *> & get_arguments() const noexcept;

    private:
        friend class ArgumentParser;

        LIBDNF_CLI_LOCAL Group(const std::string & id);

        std::string id;
        std::string header;
        std::vector<Argument *> arguments;
    };

    class Argument {
    public:
        Argument() = delete;
        Argument(const Argument &) = delete;
        Argument(Argument &&) = delete;
        Argument & operator=(const Argument &) = delete;
        Argument & operator=(Argument &&) = delete;

        virtual ~Argument();

        /// Sets a long description of the argument.
        void set_long_description(std::string descr) noexcept;

        /// Sets a brief description of the argument.
        void set_description(std::string descr) noexcept;

        /// Passes a list of arguments that cannot be used with this argument.
        /// Can contain this argument. Groups of conflicting argument can be used.
        void set_conflict_arguments(std::vector<Argument *> * args) noexcept;

        /// Adds a conflicting argument. Also adds reverse conflict.
        void add_conflict_argument(Argument & conflict_arg);

        /// Add conflict arguments from another argument. Also adds reverse conflicts.
        void add_conflict_arguments_from_another(Argument & src_arg);

        /// Gets argument id.
        const std::string & get_id() const noexcept;

        /// Gets a long description of the argument.
        const std::string & get_long_description() const;

        /// Gets a brief description of the argument.
        const std::string & get_description() const;

        /// Returns a list of arguments that cannot be used with this argument or nullptr.
        std::vector<Argument *> * get_conflict_arguments() noexcept;

        /// Gets the number of times the argument was used during analysis.
        /// Can be used to determine how many times the argument has been used on the command line.
        int get_parse_count() const noexcept;

        /// Set the number of times the argument was used during analysis count to 0.
        void reset_parse_count() noexcept;

        /// Tests input for arguments that cannot be used with this argument.
        /// Returns the first conflicting argument or nullptr.
        Argument * get_conflict_argument() const noexcept;

        /// Sets whether the argument participates in completion.
        void set_complete(bool complete) noexcept;

        /// Returns whether the argument participates in completion.
        bool get_complete() const noexcept;

        virtual void help() const noexcept;

        // Returns the ArgumentParser instance to which the argument belongs.
        ArgumentParser & get_argument_parser() const noexcept;

        /// Sets a pointer to user data in the argument.
        void set_user_data(ArgumentParserUserData * user_data) noexcept;

        /// Gets a pointer to the user data attached to the argument.
        ArgumentParserUserData * get_user_data() const noexcept;

    private:
        friend class ArgumentParser;

        LIBDNF_CLI_LOCAL Argument(ArgumentParser & owner, std::string id);

        ArgumentParser & owner;
        std::string id;
        std::string long_description;
        std::string description;
        std::vector<Argument *> * conflict_args{nullptr};
        bool complete{true};
        int parse_count{0};
        ArgumentParserUserData * user_data{nullptr};
    };

    class PositionalArg : public Argument {
    public:
        constexpr static int OPTIONAL{0};
        constexpr static int UNLIMITED{-1};
        constexpr static int AT_LEAST_ONE{-2};

        using ParseHookFunc = std::function<bool(PositionalArg * arg, int argc, const char * const argv[])>;
        using CompleteHookFunc = std::function<std::vector<std::string>(const char * arg_to_complete)>;

        PositionalArg() = delete;
        PositionalArg(const PositionalArg &) = delete;
        PositionalArg(PositionalArg &&) = delete;
        PositionalArg & operator=(const PositionalArg &) = delete;
        PositionalArg & operator=(PositionalArg &&) = delete;

        ~PositionalArg() override;

        /// Gets the number of values required by this argument on the command line.
        /// May return special values: OPTIONAL, UNLIMITED, AT_LEAST_ONE
        int get_nvals() const noexcept;

        /// Enables/disables storing parsed values.
        /// Values can be processed / stored in the user parse hook function.
        /// The "AssertionError" exception is thrown when "true" is requested but the storage array "values" is not set.
        void set_store_value(bool enable);

        /// Returns true if storing the parsed values is enabled.
        bool get_store_value() const noexcept;

        /// Gets list of values.
        /// Parsed values are stored there if if get_store_value() == true.
        std::vector<std::unique_ptr<libdnf5::Option>> * get_linked_values() noexcept;

        /// Sets the user function for parsing the argument.
        void set_parse_hook_func(ParseHookFunc && func);

        /// Gets the user function for parsing the argument.
        const ParseHookFunc & get_parse_hook_func() const noexcept;

        /// Sets the user function to complete the argument.
        void set_complete_hook_func(CompleteHookFunc && func);

        /// Gets the user function to complete the argument.
        const CompleteHookFunc & get_complete_hook_func() const noexcept;

        /// Sets the required number of repetitions of this argument on the command line.
        /// The special values OPTIONAL, UNLIMITED, AT_LEAST_ONE can be used.
        void set_nrepeats(int nrepeats);

        /// Gets the required number of repetitions of this argument on the command line.
        /// May return special values: OPTIONAL, UNLIMITED, AT_LEAST_ONE
        int get_nrepeats() const noexcept;

    private:
        friend class ArgumentParser;

        LIBDNF_CLI_LOCAL PositionalArg(
            ArgumentParser & owner, const std::string & id, std::vector<std::unique_ptr<libdnf5::Option>> * values);
        LIBDNF_CLI_LOCAL PositionalArg(
            ArgumentParser & owner,
            const std::string & id,
            int nvals,
            libdnf5::Option * init_value,
            std::vector<std::unique_ptr<libdnf5::Option>> * values);

        /// Parses input.
        /// Returns number of consumed arguments from the input.
        LIBDNF_CLI_LOCAL int parse(const char * option, int argc, const char * const argv[]);

        int nvals;  // Number of values required by this positional argument on the command line
        libdnf5::Option * init_value;
        std::vector<std::unique_ptr<libdnf5::Option>> * values;
        bool store_value{true};
        ParseHookFunc parse_hook;
        CompleteHookFunc complete_hook;
        int nrepeats{1};  // The required number of repetitions of this positional argument on the command line
    };

    class NamedArg : public Argument {
    public:
        using ParseHookFunc = std::function<bool(NamedArg * arg, const char * option, const char * value)>;

        NamedArg() = delete;
        NamedArg(const NamedArg &) = delete;
        NamedArg(NamedArg &&) = delete;
        NamedArg & operator=(const NamedArg &) = delete;
        NamedArg & operator=(NamedArg &&) = delete;

        ~NamedArg() override;

        /// Sets long name of argument. Long name is prefixed with two dashes on the command line (e.g. "--help").
        void set_long_name(std::string long_name) noexcept;

        /// Sets short name of argument. Short name is prefixed with one dash on the command line (e.g. "-h").
        void set_short_name(char short_name);

        /// Does the argument need a value on the command line?
        void set_has_value(bool has_value);

        /// Links the value to the argument.
        /// Parsed value is stored there if if get_store_value() == true.
        void link_value(libdnf5::Option * value);

        /// Sets constant argument value.
        /// It is used for argument without value on command line (get_has_value() == false).
        void set_const_value(std::string const_value) noexcept;

        /// Gets long name of argument. Long name is prefixed with two dashes on the command line (e.g. "--help").
        const std::string & get_long_name() const noexcept;

        /// Gets short name of argument. Short name is prefixed with one dash on the command line (e.g. "-h").
        char get_short_name() const noexcept;

        /// Returns true if the argument need a value on the command line.
        bool get_has_value() const noexcept;

        /// Gets constant argument value.
        /// It is used for argument without value on command line (get_has_value() == false).
        const std::string & get_const_value() const noexcept;

        /// Gets the value.
        /// Parsed value is stored there if if get_store_value() == true.
        libdnf5::Option * get_linked_value() noexcept;

        /// Gets value.
        /// Parsed value is stored there if if get_store_value() == true.
        const libdnf5::Option * get_linked_value() const noexcept;

        /// Enables/disables storing parsed values.
        /// Values can be processed / stored in the user parse hook function.
        void set_store_value(bool enable) noexcept;

        /// Returns true if storing the parsed values is enabled.
        bool get_store_value() const noexcept;

        /// Sets the user function for parsing the argument.
        void set_parse_hook_func(ParseHookFunc && func);

        /// Gets the user function for parsing the argument.
        const ParseHookFunc & get_parse_hook_func() const noexcept;

        /// Sets help text for argument value.
        void set_arg_value_help(std::string text);

        /// Gets help text for argument value.
        const std::string & get_arg_value_help() const noexcept;

        /// Create alias for this named arg. The alias is not shown in completion.
        /// The conflicting args of the alias are copied to match the current conflicting args of this named arg.
        libdnf5::cli::ArgumentParser::NamedArg * add_alias(
            const std::string & id,
            const std::string & long_name,
            char short_name,
            libdnf5::cli::ArgumentParser::Group * group);

        void attach_named_arg(const std::string & id_path, const std::string & value);

    private:
        friend class ArgumentParser;

        LIBDNF_CLI_LOCAL NamedArg(ArgumentParser & owner, const std::string & id);

        /// Parses long argument.
        /// Returns number of consumed arguments from the input.
        LIBDNF_CLI_LOCAL int parse_long(const char * option, int argc, const char * const argv[]);

        /// Parses short argument.
        /// Returns the number of arguments consumed from the input. It may be zero.
        /// Multiple short arguments can be packed in one item. (e.g. "-v -f" -> "-vf").
        LIBDNF_CLI_LOCAL int parse_short(const char * option, int argc, const char * const argv[]);

        std::string long_name;
        char short_name{'\0'};
        bool has_value{false};
        std::string const_val;  // used if params == 0
        libdnf5::Option * value{nullptr};
        bool store_value{true};
        ParseHookFunc parse_hook;
        std::string arg_value_help;

        // A named argument can invoke other named arguments - for aliases
        struct AttachedNamedArg {
            std::string id_path;
            std::string value;
        };
        std::vector<AttachedNamedArg> attached_named_args;
    };

    class Command : public Argument {
    public:
        using ParseHookFunc = std::function<bool(Command * arg, const char * cmd, int argc, const char * const argv[])>;

        Command() = delete;
        Command(const Command &) = delete;
        Command(Command &&) = delete;
        Command & operator=(const Command &) = delete;
        Command & operator=(Command &&) = delete;

        ~Command() override;

        /// Registers (sub)command to the command.
        /// An exception is thrown when a command with the same ID is already registered.
        virtual void register_command(Command * cmd) = 0;

        /// Registers named argument to the command.
        /// An exception is thrown when a named argument with the same ID is already registered.
        virtual void register_named_arg(NamedArg * arg) = 0;

        /// Registers positional argument to the command.
        /// An exception is thrown when a positional argument with the same ID is already registered.
        virtual void register_positional_arg(PositionalArg * arg) = 0;

        virtual void register_group(Group * grp) = 0;

        /// Gets a list of registered commands.
        virtual const std::vector<Command *> & get_commands() const noexcept = 0;

        /// Gets a list of registered named arguments.
        virtual const std::vector<NamedArg *> & get_named_args() const noexcept = 0;

        /// Gets a list of registered positional arguments.
        virtual const std::vector<PositionalArg *> & get_positional_args() const noexcept = 0;

        /// Gets a list of registered groups.
        virtual const std::vector<Group *> & get_groups() const noexcept = 0;

        /// Returns (sub)command with given ID.
        /// Exception CommandNotFound is thrown if command is not found.
        virtual Command & get_command(const std::string & id) const = 0;

        /// Returns named argument with given ID.
        /// Exception NamedArgNotFound is thrown if argument is not found.
        virtual NamedArg & get_named_arg(const std::string & id) const = 0;

        /// Returns positional argument with given ID.
        /// Exception PositionalArgNotFound is thrown if argument is not found.
        virtual PositionalArg & get_positional_arg(const std::string & id) const = 0;

        /// Returns registered group with given ID.
        /// Exception ArgumentParserNotFoundError is thrown if group is not found.
        virtual Group & get_group(const std::string & id) const = 0;

        /// Sets the user function for parsing the argument.
        virtual void set_parse_hook_func(ParseHookFunc && func) = 0;

        /// Gets the user function for parsing the argument.
        virtual const ParseHookFunc & get_parse_hook_func() const noexcept = 0;

        /// Parses input. The input may contain named arguments, (sub)commands and positional arguments.
        virtual void parse(const char * option, int argc, const char * const argv[]) = 0;

        /// Prints command help text.
        void help() const noexcept override;

        /// Sets the header of the subcommand table. Used to generate help.
        void set_commands_help_header(std::string text) noexcept;

        /// Sets the header of the named arguments table. Used to generate help.
        void set_named_args_help_header(std::string text) noexcept;

        /// Sets the header of the positional arguments table. Used to generate help.
        void set_positional_args_help_header(std::string text) noexcept;

        /// Gets the header of the subcommand table. Used to generate help.
        const std::string & get_commands_help_header() const noexcept;

        /// Gets the header of the named arguments table. Used to generate help.
        const std::string & get_named_args_help_header() const noexcept;

        /// Gets the header of the positional arguments table. Used to generate help.
        const std::string & get_positional_args_help_header() const noexcept;

        /// Returns the pointer to the parent Command (as set by register_command()).
        Command * get_parent() const noexcept;

        /// Gets the list of command-line arguments needed to invoke the command.
        /// Command aliases are resolved; so `get_invocation` for a
        /// RepoListCommand would return `{"dnf5", "repo", "list"}`
        virtual std::vector<std::string> get_invocation() const noexcept;

    private:
        friend class ArgumentParser;

        LIBDNF_CLI_LOCAL Command(ArgumentParser & owner, const std::string & id);

        // Prints a completed argument `arg` or a table with suggestions and help to complete
        // if there is more than one solution.
        LIBDNF_CLI_LOCAL void print_complete(
            const char * arg, std::vector<ArgumentParser::NamedArg *> named_args, size_t used_positional_arguments);

        Command * parent{nullptr};
        std::string commands_help_header = "Commands:";
        std::string named_args_help_header = "Options:";
        std::string positional_args_help_header = "Arguments:";
    };

    class CommandOrdinary : public Command {
    public:
        CommandOrdinary() = delete;
        CommandOrdinary(const CommandOrdinary &) = delete;
        CommandOrdinary(CommandOrdinary &&) = delete;
        CommandOrdinary & operator=(const CommandOrdinary &) = delete;
        CommandOrdinary & operator=(CommandOrdinary &&) = delete;

        ~CommandOrdinary() override;

        /// Registers (sub)command to the command.
        /// An exception is thrown when a command with the same ID is already registered.
        void register_command(Command * cmd) override;

        /// Registers named argument to the command.
        /// An exception is thrown when a named argument with the same ID is already registered.
        void register_named_arg(NamedArg * arg) override;

        /// Registers positional argument to the command.
        /// An exception is thrown when a positional argument with the same ID is already registered.
        void register_positional_arg(PositionalArg * arg) override;

        void register_group(Group * grp) override;

        /// Gets a list of registered commands.
        const std::vector<Command *> & get_commands() const noexcept override;

        /// Gets a list of registered named arguments.
        const std::vector<NamedArg *> & get_named_args() const noexcept override;

        /// Gets a list of registered positional arguments.
        const std::vector<PositionalArg *> & get_positional_args() const noexcept override;

        /// Gets a list of registered groups.
        const std::vector<Group *> & get_groups() const noexcept override;

        /// Returns (sub)command with given ID.
        /// Exception CommandNotFound is thrown if command is not found.
        Command & get_command(const std::string & id) const override;

        /// Returns named argument with given ID.
        /// Exception NamedArgNotFound is thrown if argument is not found.
        NamedArg & get_named_arg(const std::string & id) const override;

        /// Returns positional argument with given ID.
        /// Exception PositionalArgNotFound is thrown if argument is not found.
        PositionalArg & get_positional_arg(const std::string & id) const override;

        /// Returns registered group with given ID.
        /// Exception ArgumentParserNotFoundError is thrown if group is not found.
        Group & get_group(const std::string & id) const override;

        /// Sets the user function for parsing the argument.
        void set_parse_hook_func(ParseHookFunc && func) override;

        /// Gets the user function for parsing the argument.
        const ParseHookFunc & get_parse_hook_func() const noexcept override;

        /// Parses input. The input may contain named arguments, (sub)commands and positional arguments.
        void parse(const char * option, int argc, const char * const argv[]) override;

    private:
        friend class ArgumentParser;

        LIBDNF_CLI_LOCAL CommandOrdinary(ArgumentParser & owner, const std::string & id);

        std::vector<Command *> cmds;
        std::vector<NamedArg *> named_args;
        std::vector<PositionalArg *> pos_args;
        std::vector<Group *> groups;
        ParseHookFunc parse_hook;
    };

    class CommandAlias : public Command {
    public:
        CommandAlias() = delete;
        CommandAlias(const CommandAlias &) = delete;
        CommandAlias(CommandAlias &&) = delete;
        CommandAlias & operator=(const CommandAlias &) = delete;
        CommandAlias & operator=(CommandAlias &&) = delete;

        ~CommandAlias() override;

        /// Registers (sub)command to the command.
        /// An exception is thrown when a command with the same ID is already registered.
        void register_command(Command * cmd) override;

        /// Registers named argument to the command.
        /// An exception is thrown when a named argument with the same ID is already registered.
        void register_named_arg(NamedArg * arg) override;

        /// Registers positional argument to the command.
        /// An exception is thrown when a positional argument with the same ID is already registered.
        void register_positional_arg(PositionalArg * arg) override;

        void register_group(Group * grp) override;

        std::vector<std::string> get_invocation() const noexcept override;

        /// Gets a list of registered commands.
        const std::vector<Command *> & get_commands() const noexcept override;

        /// Gets a list of registered named arguments.
        const std::vector<NamedArg *> & get_named_args() const noexcept override;

        /// Gets a list of registered positional arguments.
        const std::vector<PositionalArg *> & get_positional_args() const noexcept override;

        /// Gets a list of registered groups.
        const std::vector<Group *> & get_groups() const noexcept override;

        /// Returns (sub)command with given ID.
        /// Exception CommandNotFound is thrown if command is not found.
        Command & get_command(const std::string & id) const override;

        /// Returns named argument with given ID.
        /// Exception NamedArgNotFound is thrown if argument is not found.
        NamedArg & get_named_arg(const std::string & id) const override;

        /// Returns positional argument with given ID.
        /// Exception PositionalArgNotFound is thrown if argument is not found.
        PositionalArg & get_positional_arg(const std::string & id) const override;

        /// Returns registered group with given ID.
        /// Exception ArgumentParserNotFoundError is thrown if group is not found.
        Group & get_group(const std::string & id) const override;

        /// Sets the user function for parsing the argument.
        void set_parse_hook_func(ParseHookFunc && func) override;

        /// Gets the user function for parsing the argument.
        const ParseHookFunc & get_parse_hook_func() const noexcept override;

        /// Parses input. The input may contain named arguments, (sub)commands and positional arguments.
        void parse(const char * option, int argc, const char * const argv[]) override;

        /// Returns the Command to which the CommandAlias is attached.
        Command & get_attached_command() noexcept;

        void attach_named_arg(const std::string & id_path, const std::string & value);

        /// Add required value (positional argument) consumed by the command alias.
        /// These arguments can be used as values for attached named arguments.
        void add_required_value(const std::string & value_help, const std::string & descr);

    private:
        friend class ArgumentParser;

        LIBDNF_CLI_LOCAL CommandAlias(ArgumentParser & owner, const std::string & id, Command & attached_command);

        Command & attached_command;

        // A command can have values (positional arguments) that can be passed to the attached named arguments.
        struct RequiredValue {
            std::string value_help;
            std::string descr;
        };
        std::vector<RequiredValue> required_values;

        // A command can invoke named arguments - for aliases
        struct AttachedNamedArg {
            std::string id_path;
            std::string value;
        };
        std::vector<AttachedNamedArg> attached_named_args;
    };

    ArgumentParser(const ArgumentParser &) = delete;
    ArgumentParser(ArgumentParser &&) = delete;
    ArgumentParser & operator=(const ArgumentParser &) = delete;
    ArgumentParser & operator=(ArgumentParser &&) = delete;

    ArgumentParser();
    ~ArgumentParser();

    /// Constructs a new command and stores it to the argument parser.
    /// Returns a pointer to the newly created command.
    CommandOrdinary * add_new_command(const std::string & id);

    /// Constructs a new command alias and stores it to the argument parser.
    /// Returns a pointer to the newly created command alias.
    CommandAlias * add_new_command_alias(const std::string & id, Command & attached_command);

    /// Constructs a new named argument and stores it to the argument parser.
    /// Returns a pointer to the newly created named argument.
    NamedArg * add_new_named_arg(const std::string & id);

    /// Constructs a new positional argument and stores it to the argument parser.
    /// Returns a pointer to the newly created positional argument.
    PositionalArg * add_new_positional_arg(
        const std::string & id, std::vector<std::unique_ptr<libdnf5::Option>> * values);

    /// Constructs a new positional argument and stores it to the argument parser.
    /// Returns a pointer to the newly created positional argument.
    PositionalArg * add_new_positional_arg(
        const std::string & id,
        int nargs,
        libdnf5::Option * init_value,
        std::vector<std::unique_ptr<libdnf5::Option>> * values);

    /// Constructs a new group and stores it to the argument parser.
    /// Returns a pointer to the newly created group.
    Group * add_new_group(const std::string & id);

    /// Moves a list of conflicting argument to the parser.
    /// Returns a pointer to the list.
    std::vector<Argument *> * add_conflict_args_group(std::unique_ptr<std::vector<Argument *>> && conflict_args_group);

    /// Moves the option with the initial value to the argument parser.
    /// Returns a pointer to the option.
    libdnf5::Option * add_init_value(std::unique_ptr<libdnf5::Option> && src);

    /// Constructs a new empty list of values and stores it to the argument parser.
    /// Returns a pointer to the newly created list.
    std::vector<std::unique_ptr<libdnf5::Option>> * add_new_values();

    /// Moves an existing list of values to the argument parser.
    /// Returns a pointer to the list.
    std::vector<std::unique_ptr<libdnf5::Option>> * add_values(
        std::unique_ptr<std::vector<std::unique_ptr<libdnf5::Option>>> && values);

    /// Sets the "root" command.
    /// This is the top-level command in the command hierarchy. It can contain named and positional arguments and subcommands.
    void set_root_command(Command * command) noexcept;

    /// Gets the "root" command.
    /// This is the top-level command in the command hierarchy. It can contain named and positional arguments and subcommands.
    Command * get_root_command() noexcept;

    /// Get the selected command.
    /// Needed for printing help, bash completion etc.
    Command * get_selected_command() noexcept;

    /// Parses input. The parser from the "root" command is called.
    /// The "AssertionError" exception is thrown when the "root" command is not set.
    void parse(int argc, const char * const argv[]);

    /// Reset parse count in all arguments.
    void reset_parse_count();

    /// Enables/disables the inheritance of named arguments for the parser.
    /// If the parser does not find a named argument definition during subcommand processing and
    /// named arguments inheritance is enabled, parser searches the named arguments of the parent commands.
    /// @param enable  true - enable parent argument inheritance, false - disable
    void set_inherit_named_args(bool enable) noexcept;

    /// Returns true if the inheritance of named arguments is enabled for the parser.
    bool get_inherit_named_args() const noexcept;

    /// Returns (sub)command with given ID path.
    /// The root command ID must be omitted. It is added automatically.
    /// @param id_path  command ID path, e.g. "module.list"; "" - returns root_command
    /// @exception AssertionError  if root command is not set
    /// @exception ArgumentParser::Command::CommandNotFound  if command is not found.
    Command & get_command(const std::string & id_path);

    /// Returns named argument with given full ID path.
    /// The root command ID must be omitted. It is added automatically.
    /// If the named argument is not found in the path and search in the parent command is enabled,
    /// it will be searched in the parent command. E.g. "installroot" is global option -> instead
    /// of "repoquery.installroot" returns "installroot".
    /// @param id_path  named argument ID path, e.g. "installroot", "repoquery.installed"
    /// @param search_in_parent  true - enable search in parent command, false - disable
    /// @exception AssertionError  if root command is not set
    /// @exception ArgumentParser::Command::CommandNotFound  if command is not found.
    /// @exception ArgumentParser::Command::PositionalArgNotFound  if argument is not found.
    NamedArg & get_named_arg(const std::string & id_path, bool search_in_parent);

    /// Returns positional argument with given full ID path.
    /// The root command ID must be omitted. It is added automatically.
    /// If the positional argument is not found in the path and search in the parent command is enabled,
    /// it will be searched in the parent command. E.g. "installroot" is global option -> instead
    /// of "repoquery.installroot" returns "installroot".
    /// @param id_path  positional argument ID path, e.g. "repoquery.keys"
    /// @param search_in_parent  true - enable search in parent command, false - disable
    /// @exception AssertionError  if root command is not set
    /// @exception ArgumentParser::Command::CommandNotFound  if command is not found.
    /// @exception ArgumentParser::Command::NamedArgNotFound  if argument is not found.
    PositionalArg & get_positional_arg(const std::string & id_path, bool search_in_parent);

    /// Gets a list of all commands stored in the argument parser.
    const std::vector<std::unique_ptr<Command>> & get_commands() const noexcept;

    /// Gets a list of all named arguments stored in the argument parser.
    const std::vector<std::unique_ptr<NamedArg>> & get_named_args() const noexcept;

    /// Gets a list of all positional argument stored in the argument parser.
    const std::vector<std::unique_ptr<PositionalArg>> & get_positional_args() const noexcept;

    /// Prints the completed argument from the `complete_arg_idx` position in the argv array. In case of more than one
    /// potential completion matches, prints a table with the potential matches along with their short help descriptions.
    void complete(int argc, const char * const argv[], int complete_arg_idx);

    /// Sets whether the description is added to the suggested arguments on completion.
    void set_complete_add_description(bool enable) noexcept;

    /// Returns whether the description is added to the suggested arguments on completion.
    bool get_complete_add_description() noexcept;

private:
    class LIBDNF_CLI_LOCAL ArgumentParserImpl;
    const std::unique_ptr<ArgumentParserImpl> p_impl;
};

}  // namespace libdnf5::cli

#endif
