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

#ifndef LIBDNF5_CLI_ARGUMENT_PARSER_HPP
#define LIBDNF5_CLI_ARGUMENT_PARSER_HPP

#include "libdnf5-cli/exception.hpp"

#include <libdnf5/conf/option.hpp>

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace libdnf5::cli {

/// Parent for all ArgumentsParser runtime errors.
class ArgumentParserError : public Error {
    using Error::Error;
    const char * get_name() const noexcept override { return "ArgumentParserError"; }
};

/// Exception is thrown when conflicting arguments are used together.
class ArgumentParserConflictingArgumentsError : public ArgumentParserError {
public:
    using ArgumentParserError::ArgumentParserError;
    const char * get_name() const noexcept override { return "ArgumentParserConflictingArgumentsError"; }
};

/// Exception is thrown when no command is found.
class ArgumentParserMissingCommandError : public ArgumentParserError {
    using ArgumentParserError::ArgumentParserError;
    const char * get_name() const noexcept override { return "ArgumentParserMissingCommandError"; }
};

/// Exception is thrown when a command requires a positional argument that was not found.
class ArgumentParserMissingPositionalArgumentError : public ArgumentParserError {
    using ArgumentParserError::ArgumentParserError;
    const char * get_name() const noexcept override { return "ArgumentParserMissingPositionalArgumentError"; }
};

/// Exception is thrown when a positional argument has invalid format.
class ArgumentParserPositionalArgumentFormatError : public ArgumentParserError {
    using ArgumentParserError::ArgumentParserError;
    const char * get_name() const noexcept override { return "ArgumentParserPositionalArgumentFormatError"; }
};

/// Exception is generated in the case of an unexpected argument.
class ArgumentParserUnknownArgumentError : public ArgumentParserError {
private:
    std::string command;
    std::string argument;

public:
    using ArgumentParserError::ArgumentParserError;

    template <typename... Ss>
    ArgumentParserUnknownArgumentError(
        const std::string command, const std::string argument, const BgettextMessage & format, Ss &&... args)
        : ArgumentParserError(format, std::forward<Ss>(args)...),
          command(command),
          argument(argument) {}
    const char * get_name() const noexcept override { return "ArgumentParserUnknownArgumentError"; }
    const std::string & get_command() const { return command; };
    const std::string & get_argument() const { return argument; };
};


/// Exception is thrown when an argument with the same ID is already registered.
class ArgumentParserGroupArgumentIdRegisteredError : public ArgumentParserError {
public:
    using ArgumentParserError::ArgumentParserError;
    const char * get_name() const noexcept override { return "ArgumentParserGroupArgumentIdRegisteredError"; }
};


/// Exception is thrown when the Argument `id` contains not allowed '.' character.
class ArgumentParserArgumentInvalidIdError : public ArgumentParserError {
public:
    using ArgumentParserError::ArgumentParserError;
    const char * get_name() const noexcept override { return "ArgumentParserArgumentInvalidIdError"; }
};


/// Exception is thrown when there are too few values for the positional argument.
class ArgumentParserPositionalArgFewValuesError : public ArgumentParserError {
public:
    using ArgumentParserError::ArgumentParserError;
    const char * get_name() const noexcept override { return "ArgumentParserPositionalArgFewValuesError"; }
};


/// Exception is thrown if the named argument requires a value and the value is missing.
class ArgumentParserNamedArgMissingValueError : public ArgumentParserError {
public:
    using ArgumentParserError::ArgumentParserError;
    const char * get_name() const noexcept override { return "ArgumentParserNamedArgMissingValueError"; }
};

/// Exception is thrown if the named argument is defined without a value and a value is present.
class ArgumentParserNamedArgValueNotExpectedError : public ArgumentParserError {
public:
    using ArgumentParserError::ArgumentParserError;
    const char * get_name() const noexcept override { return "ArgumentParserNamedArgValueNotExpectedError"; }
};


/// Exception is thrown when the command, named_argument, or positional argument was not found.
class ArgumentParserNotFoundError : public ArgumentParserError {
public:
    using ArgumentParserError::ArgumentParserError;
    const char * get_name() const noexcept override { return "ArgumentParserNotFoundError"; }
};

/// Exception is thrown when the command, named argument, positional argument, or group with the same ID is already registered.
class ArgumentParserIdAlreadyRegisteredError : public ArgumentParserError {
public:
    using ArgumentParserError::ArgumentParserError;
    const char * get_name() const noexcept override { return "ArgumentParserIdAlreadyRegisteredError"; }
};

/// Exception is thrown when the command, named argument, positiona argument, or group needs an additional argument.
class ArgumentParserMissingDependentArgumentError : public ArgumentParserError {
public:
    using ArgumentParserError::ArgumentParserError;
    const char * get_name() const noexcept override { return "ArgumentParserMissingDependentArgumentError"; }
};

/// Exception is thrown when the given argument value is not valid.
class ArgumentParserInvalidValueError : public ArgumentParserError {
public:
    using ArgumentParserError::ArgumentParserError;
    const char * get_name() const noexcept override { return "ArgumentParserInvalidValueError"; }
};

/// Base class for user data used in ArgumentParser::Argument
class ArgumentParserUserData {};

class ArgumentParser {
public:
    class Argument;

    class Group {
    public:
        /// Sets group header.
        void set_header(std::string header) noexcept { this->header = std::move(header); }

        /// Gets group id.
        const std::string & get_id() const noexcept { return id; }

        /// Gets group header.
        const std::string & get_header() const noexcept { return header; }

        /// Registers an argument to the group.
        /// An exception is thrown when an argument with the same ID is already registered.
        void register_argument(Argument * arg);

        /// Gets a list of registered arguments.
        const std::vector<Argument *> & get_arguments() const noexcept { return arguments; }

    private:
        friend class ArgumentParser;

        Group(const std::string & id) : id(id) {}

        std::string id;
        std::string header;
        std::vector<Argument *> arguments;
    };

    class Argument {
    public:
        Argument(const Argument &) = delete;
        Argument(Argument &&) = delete;
        Argument & operator=(const Argument &) = delete;
        Argument & operator=(Argument &&) = delete;
        virtual ~Argument() = default;

        /// Sets a long description of the argument.
        void set_long_description(std::string descr) noexcept { long_description = std::move(descr); }

        /// Sets a brief description of the argument.
        void set_description(std::string descr) noexcept { description = std::move(descr); }

        /// Passes a list of arguments that cannot be used with this argument.
        /// Can contain this argument. Groups of conflicting argument can be used.
        void set_conflict_arguments(std::vector<Argument *> * args) noexcept { conflict_args = args; }

        /// Adds a conflicting argument. Also adds reverse conflict.
        void add_conflict_argument(Argument & conflict_arg);

        /// Add conflict arguments from another argument. Also adds reverse conflicts.
        void add_conflict_arguments_from_another(Argument & src_arg);

        /// Gets argument id.
        const std::string & get_id() const noexcept { return id; }

        /// Gets a long description of the argument.
        const std::string & get_long_description() const { return long_description; }

        /// Gets a brief description of the argument.
        const std::string & get_description() const { return description; }

        /// Returns a list of arguments that cannot be used with this argument or nullptr.
        std::vector<Argument *> * get_conflict_arguments() noexcept { return conflict_args; }

        /// Gets the number of times the argument was used during analysis.
        /// Can be used to determine how many times the argument has been used on the command line.
        int get_parse_count() const noexcept { return parse_count; }

        /// Set the number of times the argument was used during analysis count to 0.
        void reset_parse_count() noexcept { parse_count = 0; }

        /// Tests input for arguments that cannot be used with this argument.
        /// Returns the first conflicting argument or nullptr.
        Argument * get_conflict_argument() const noexcept;

        /// Sets whether the argument participates in completion.
        void set_complete(bool complete) noexcept { this->complete = complete; }

        /// Returns whether the argument participates in completion.
        bool get_complete() const noexcept { return complete; }

        virtual void help() const noexcept {}

        // Returns the ArgumentParser instance to which the argument belongs.
        ArgumentParser & get_argument_parser() const noexcept { return owner; }

        /// Sets a pointer to user data in the argument.
        void set_user_data(ArgumentParserUserData * user_data) noexcept { this->user_data = user_data; }

        /// Gets a pointer to the user data attached to the argument.
        ArgumentParserUserData * get_user_data() const noexcept { return user_data; }

    private:
        friend class ArgumentParser;

        Argument(ArgumentParser & owner, std::string id);

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

        /// Gets the number of values required by this argument on the command line.
        /// May return special values: OPTIONAL, UNLIMITED, AT_LEAST_ONE
        int get_nvals() const noexcept { return nvals; }

        /// Enables/disables storing parsed values.
        /// Values can be processed / stored in the user parse hook function.
        /// The "AssertionError" exception is thrown when "true" is requested but the storage array "values" is not set.
        void set_store_value(bool enable);

        /// Returns true if storing the parsed values is enabled.
        bool get_store_value() const noexcept { return store_value; }

        /// Gets list of values.
        /// Parsed values are stored there if if get_store_value() == true.
        std::vector<std::unique_ptr<libdnf5::Option>> * get_linked_values() noexcept { return values; }

        /// Sets the user function for parsing the argument.
        void set_parse_hook_func(ParseHookFunc && func) { parse_hook = std::move(func); }

        /// Gets the user function for parsing the argument.
        const ParseHookFunc & get_parse_hook_func() const noexcept { return parse_hook; }

        /// Sets the user function to complete the argument.
        void set_complete_hook_func(CompleteHookFunc && func) { complete_hook = std::move(func); }

        /// Gets the user function to complete the argument.
        const CompleteHookFunc & get_complete_hook_func() const noexcept { return complete_hook; }

        /// Sets the required number of repetitions of this argument on the command line.
        /// The special values OPTIONAL, UNLIMITED, AT_LEAST_ONE can be used.
        void set_nrepeats(int nrepeats) { this->nrepeats = nrepeats; }

        /// Gets the required number of repetitions of this argument on the command line.
        /// May return special values: OPTIONAL, UNLIMITED, AT_LEAST_ONE
        int get_nrepeats() const noexcept { return nrepeats; }

    private:
        friend class ArgumentParser;

        PositionalArg(
            ArgumentParser & owner, const std::string & id, std::vector<std::unique_ptr<libdnf5::Option>> * values);
        PositionalArg(
            ArgumentParser & owner,
            const std::string & id,
            int nvals,
            libdnf5::Option * init_value,
            std::vector<std::unique_ptr<libdnf5::Option>> * values);

        /// Parses input.
        /// Returns number of consumed arguments from the input.
        int parse(const char * option, int argc, const char * const argv[]);

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

        /// Sets long name of argument. Long name is prefixed with two dashes on the command line (e.g. "--help").
        void set_long_name(std::string long_name) noexcept { this->long_name = std::move(long_name); }

        /// Sets short name of argument. Short name is prefixed with one dash on the command line (e.g. "-h").
        void set_short_name(char short_name) { this->short_name = short_name; }

        /// Does the argument need a value on the command line?
        void set_has_value(bool has_value) { this->has_value = has_value; }

        /// Links the value to the argument.
        /// Parsed value is stored there if if get_store_value() == true.
        void link_value(libdnf5::Option * value) { this->value = value; }

        /// Sets constant argument value.
        /// It is used for argument without value on command line (get_has_value() == false).
        void set_const_value(std::string const_value) noexcept { const_val = std::move(const_value); }

        /// Gets long name of argument. Long name is prefixed with two dashes on the command line (e.g. "--help").
        const std::string & get_long_name() const noexcept { return long_name; }

        /// Gets short name of argument. Short name is prefixed with one dash on the command line (e.g. "-h").
        char get_short_name() const noexcept { return short_name; }

        /// Returns true if the argument need a value on the command line.
        bool get_has_value() const noexcept { return has_value; }

        /// Gets constant argument value.
        /// It is used for argument without value on command line (get_has_value() == false).
        const std::string & get_const_value() const noexcept { return const_val; }

        /// Gets the value.
        /// Parsed value is stored there if if get_store_value() == true.
        libdnf5::Option * get_linked_value() noexcept { return value; }

        /// Gets value.
        /// Parsed value is stored there if if get_store_value() == true.
        const libdnf5::Option * get_linked_value() const noexcept { return value; }

        /// Enables/disables storing parsed values.
        /// Values can be processed / stored in the user parse hook function.
        void set_store_value(bool enable) noexcept { store_value = enable; }

        /// Returns true if storing the parsed values is enabled.
        bool get_store_value() const noexcept { return store_value; }

        /// Sets the user function for parsing the argument.
        void set_parse_hook_func(ParseHookFunc && func) { parse_hook = std::move(func); }

        /// Gets the user function for parsing the argument.
        const ParseHookFunc & get_parse_hook_func() const noexcept { return parse_hook; }

        /// Sets help text for argument value.
        void set_arg_value_help(std::string text) { arg_value_help = std::move(text); }

        /// Gets help text for argument value.
        const std::string & get_arg_value_help() const noexcept { return arg_value_help; }

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

        NamedArg(ArgumentParser & owner, const std::string & id) : Argument(owner, id) {}

        /// Parses long argument.
        /// Returns number of consumed arguments from the input.
        int parse_long(const char * option, int argc, const char * const argv[]);

        /// Parses short argument.
        /// Returns the number of arguments consumed from the input. It may be zero.
        /// Multiple short arguments can be packed in one item. (e.g. "-v -f" -> "-vf").
        int parse_short(const char * option, int argc, const char * const argv[]);

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

        /// Parses input. The input may contain named arguments, (sub)commands and positional arguments.
        virtual void parse(const char * option, int argc, const char * const argv[]) = 0;

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

        /// Prints command help text.
        void help() const noexcept override;

        /// Sets the header of the subcommand table. Used to generate help.
        void set_commands_help_header(std::string text) noexcept { commands_help_header = std::move(text); }

        /// Sets the header of the named arguments table. Used to generate help.
        void set_named_args_help_header(std::string text) noexcept { named_args_help_header = std::move(text); }

        /// Sets the header of the positional arguments table. Used to generate help.
        void set_positional_args_help_header(std::string text) noexcept {
            positional_args_help_header = std::move(text);
        }

        /// Gets the header of the subcommand table. Used to generate help.
        const std::string & get_commands_help_header() const noexcept { return commands_help_header; }

        /// Gets the header of the named arguments table. Used to generate help.
        const std::string & get_named_args_help_header() const noexcept { return named_args_help_header; }

        /// Gets the header of the positional arguments table. Used to generate help.
        const std::string & get_positional_args_help_header() const noexcept { return positional_args_help_header; }

        /// Returns the pointer to the parent Command (as set by register_command()).
        Command * get_parent() const noexcept { return parent; }

        /// Gets the list of command-line arguments needed to invoke the command.
        /// Command aliases are resolved; so `get_invocation` for a
        /// RepoListCommand would return `{"dnf5", "repo", "list"}`
        virtual std::vector<std::string> get_invocation() const noexcept;

    private:
        friend class ArgumentParser;

        Command(ArgumentParser & owner, const std::string & id) : Argument(owner, id) {}

        void print_complete(
            const char * arg, std::vector<ArgumentParser::NamedArg *> named_args, size_t used_positional_arguments);

        Command * parent{nullptr};
        std::string commands_help_header = "Commands:";
        std::string named_args_help_header = "Options:";
        std::string positional_args_help_header = "Arguments:";
    };

    class CommandOrdinary : public Command {
    public:
        /// Parses input. The input may contain named arguments, (sub)commands and positional arguments.
        void parse(const char * option, int argc, const char * const argv[]) override;

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
        const std::vector<Command *> & get_commands() const noexcept override { return cmds; }

        /// Gets a list of registered named arguments.
        const std::vector<NamedArg *> & get_named_args() const noexcept override { return named_args; }

        /// Gets a list of registered positional arguments.
        const std::vector<PositionalArg *> & get_positional_args() const noexcept override { return pos_args; }

        /// Gets a list of registered groups.
        const std::vector<Group *> & get_groups() const noexcept override { return groups; }

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
        void set_parse_hook_func(ParseHookFunc && func) override { parse_hook = std::move(func); }

        /// Gets the user function for parsing the argument.
        const ParseHookFunc & get_parse_hook_func() const noexcept override { return parse_hook; }

    private:
        friend class ArgumentParser;

        CommandOrdinary(ArgumentParser & owner, const std::string & id) : Command(owner, id) {}

        std::vector<Command *> cmds;
        std::vector<NamedArg *> named_args;
        std::vector<PositionalArg *> pos_args;
        std::vector<Group *> groups;
        ParseHookFunc parse_hook;
    };

    class CommandAlias : public Command {
    public:
        /// Parses input. The input may contain named arguments, (sub)commands and positional arguments.
        void parse(const char * option, int argc, const char * const argv[]) override;

        /// Registers (sub)command to the command.
        /// An exception is thrown when a command with the same ID is already registered.
        void register_command(Command * cmd) override { attached_command.register_command(cmd); }

        /// Registers named argument to the command.
        /// An exception is thrown when a named argument with the same ID is already registered.
        void register_named_arg(NamedArg * arg) override { attached_command.register_named_arg(arg); }

        /// Registers positional argument to the command.
        /// An exception is thrown when a positional argument with the same ID is already registered.
        void register_positional_arg(PositionalArg * arg) override { attached_command.register_positional_arg(arg); }

        void register_group(Group * grp) override { attached_command.register_group(grp); }

        std::vector<std::string> get_invocation() const noexcept override { return attached_command.get_invocation(); }

        /// Gets a list of registered commands.
        const std::vector<Command *> & get_commands() const noexcept override {
            return attached_command.get_commands();
        }

        /// Gets a list of registered named arguments.
        const std::vector<NamedArg *> & get_named_args() const noexcept override {
            return attached_command.get_named_args();
        }

        /// Gets a list of registered positional arguments.
        const std::vector<PositionalArg *> & get_positional_args() const noexcept override {
            return attached_command.get_positional_args();
        }

        /// Gets a list of registered groups.
        const std::vector<Group *> & get_groups() const noexcept override { return attached_command.get_groups(); }

        /// Returns (sub)command with given ID.
        /// Exception CommandNotFound is thrown if command is not found.
        Command & get_command(const std::string & id) const override { return attached_command.get_command(id); }

        /// Returns named argument with given ID.
        /// Exception NamedArgNotFound is thrown if argument is not found.
        NamedArg & get_named_arg(const std::string & id) const override { return attached_command.get_named_arg(id); }

        /// Returns positional argument with given ID.
        /// Exception PositionalArgNotFound is thrown if argument is not found.
        PositionalArg & get_positional_arg(const std::string & id) const override {
            return attached_command.get_positional_arg(id);
        }

        /// Returns registered group with given ID.
        /// Exception ArgumentParserNotFoundError is thrown if group is not found.
        Group & get_group(const std::string & id) const override { return attached_command.get_group(id); }

        /// Sets the user function for parsing the argument.
        void set_parse_hook_func(ParseHookFunc && func) override {
            attached_command.set_parse_hook_func(std::move(func));
        }

        /// Gets the user function for parsing the argument.
        const ParseHookFunc & get_parse_hook_func() const noexcept override {
            return attached_command.get_parse_hook_func();
        }

        /// Returns the Command to which the CommandAlias is attached.
        Command & get_attached_command() noexcept { return attached_command; }

        void attach_named_arg(const std::string & id_path, const std::string & value);

    private:
        friend class ArgumentParser;

        CommandAlias(ArgumentParser & owner, const std::string & id, Command & attached_command)
            : Command(owner, id),
              attached_command(attached_command) {}

        Command & attached_command;

        // A command can invoke named arguments - for aliases
        struct AttachedNamedArg {
            std::string id_path;
            std::string value;
        };
        std::vector<AttachedNamedArg> attached_named_args;
    };

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
    void set_root_command(Command * command) noexcept { root_command = command; }

    /// Gets the "root" command.
    /// This is the top-level command in the command hierarchy. It can contain named and positional arguments and subcommands.
    Command * get_root_command() noexcept { return root_command; }

    /// Get the selected command.
    /// Needed for printing help, bash completion etc.
    Command * get_selected_command() noexcept { return selected_command; }

    /// Parses input. The parser from the "root" command is called.
    /// The "AssertionError" exception is thrown when the "root" command is not set.
    void parse(int argc, const char * const argv[]);

    /// Reset parse count in all arguments.
    void reset_parse_count();

    /// Enables/disables the inheritance of named arguments for the parser.
    /// If the parser does not find a named argument definition during subcommand processing and
    /// named arguments inheritance is enabled, parser searches the named arguments of the parent commands.
    /// @param enable  true - enable parent argument inheritance, false - disable
    void set_inherit_named_args(bool enable) noexcept { inherit_named_args = enable; }

    /// Returns true if the inheritance of named arguments is enabled for the parser.
    bool get_inherit_named_args() const noexcept { return inherit_named_args; }

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
    const std::vector<std::unique_ptr<Command>> & get_commands() const noexcept { return cmds; }

    /// Gets a list of all named arguments stored in the argument parser.
    const std::vector<std::unique_ptr<NamedArg>> & get_named_args() const noexcept { return named_args; }

    /// Gets a list of all positional argument stored in the argument parser.
    const std::vector<std::unique_ptr<PositionalArg>> & get_positional_args() const noexcept { return pos_args; }

    /// Prints the completed argument from the `complete_arg_idx` position in the argv array. In case of more than one
    /// potential completion matches, prints a table with the potential matches along with their short help descriptions.
    void complete(int argc, const char * const argv[], int complete_arg_idx);

private:
    void assert_root_command();

    std::vector<std::unique_ptr<Command>> cmds;
    std::vector<std::unique_ptr<NamedArg>> named_args;
    std::vector<std::unique_ptr<PositionalArg>> pos_args;
    std::vector<std::unique_ptr<Group>> groups;
    std::vector<std::unique_ptr<std::vector<Argument *>>> conflict_args_groups;
    std::vector<std::unique_ptr<libdnf5::Option>> values_init;
    std::vector<std::unique_ptr<std::vector<std::unique_ptr<libdnf5::Option>>>> values;
    Command * root_command{nullptr};
    Command * selected_command{nullptr};
    bool inherit_named_args{false};
    const char * const * complete_arg_ptr{nullptr};
};

}  // namespace libdnf5::cli

#endif
