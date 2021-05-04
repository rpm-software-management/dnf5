/*
Copyright (C) 2019-2021 Red Hat, Inc.

This file is part of microdnf: https://github.com/rpm-software-management/libdnf/

Microdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Microdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with microdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef LIBDNF_CLI_ARGUMENT_PARSER_HPP
#define LIBDNF_CLI_ARGUMENT_PARSER_HPP

#include "libdnf/common/exception.hpp"
#include "libdnf/conf/option.hpp"

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace libdnf::cli {

class ArgumentParser {
public:
    /// It reports errors that are a consequence of faulty logic within the program.
    class LogicError : public libdnf::LogicError {
        using libdnf::LogicError::LogicError;
        const char * get_domain_name() const noexcept override { return "libdnf::cli::ArgumentParser"; }
    };

    /// Parent for all ArgumentsParser runtime errors.
    class Exception : public RuntimeError {
        using RuntimeError::RuntimeError;
        const char * get_domain_name() const noexcept override { return "libdnf::cli::ArgumentParser"; }
        const char * get_name() const noexcept override { return "Exception"; }
        const char * get_description() const noexcept override { return "ArgumentParser exception"; }
    };

    /// Exception is generated when conflicting arguments are used together.
    class Conflict : public Exception {
    public:
        using Exception::Exception;
        const char * get_name() const noexcept override { return "Conflict"; }
        const char * get_description() const noexcept override { return "Conflicting arguments"; }
    };

    /// Exception is thrown when a command requires a positional argument that was not found.
    class MissingPositionalArgument : public Exception {
        using Exception::Exception;
        const char * get_name() const noexcept override { return "MissingPositionalArgument"; }
        const char * get_description() const noexcept override { return "Missing positional argument"; }
    };

    /// Exception is generated in the case of an unexpected argument.
    class UnknownArgument : public Exception {
    public:
        using Exception::Exception;
        const char * get_name() const noexcept override { return "UnknownArgument"; }
        const char * get_description() const noexcept override { return "Unknown argument"; }
    };

    class Argument {
    public:
        Argument(const Argument &) = delete;
        Argument(Argument &&) = delete;
        Argument & operator=(const Argument &) = delete;
        Argument & operator=(Argument &&) = delete;
        virtual ~Argument() = default;

        /// Sets a description of the argument.
        void set_description(std::string descr) noexcept { description = std::move(descr); }

        /// Sets a brief description of the argument.
        void set_short_description(std::string descr) noexcept { short_description = std::move(descr); }

        /// Passes a list of arguments that cannot be used with this argument.
        /// Can contain this argument. Groups of conflicting argument can be used.
        void set_conflict_arguments(std::vector<Argument *> * args) noexcept { conflict_args = args; }

        /// Gets argument id.
        const std::string & get_id() const noexcept { return id; }

        /// Gets a description of the argument.
        const std::string & get_description() const { return description; }

        /// Gets a brief description of the argument.
        const std::string & get_short_description() const { return short_description; }

        /// Gets the number of times the argument was used during analysis.
        /// Can be used to determine how many times the argument has been used on the command line.
        int get_parse_count() const noexcept { return parse_count; }

        /// Set the number of times the argument was used during analysis count to 0.
        void reset_parse_count() noexcept { parse_count = 0; }

        /// Tests input for arguments that cannot be used with this argument.
        /// Returns the first conflicting argument.
        Argument * get_conflict_argument() const noexcept;

        virtual void help() const noexcept {}

    private:
        friend class ArgumentParser;

        Argument(ArgumentParser & owner, std::string id) : owner(owner), id(std::move(id)) {}
        static std::string get_conflict_arg_msg(const Argument * conflict_arg);
        ArgumentParser & get_owner() const noexcept { return owner; }

        ArgumentParser & owner;
        std::string id;
        std::string description;
        std::string short_description;
        std::vector<Argument *> * conflict_args{nullptr};
        int parse_count{0};
    };

    class PositionalArg : public Argument {
    public:
        constexpr static int OPTIONAL{0};
        constexpr static int UNLIMITED{-1};
        constexpr static int UNLIMITED_BUT_ONE{-2};

        /// Exception is generated when there are insufficient values for the positional argument.
        class FewValues : public Exception {
        public:
            using Exception::Exception;
            const char * get_domain_name() const noexcept override {
                return "libdnf::cli::ArgumentParser::PositionalArg";
            }
            const char * get_name() const noexcept override { return "FewValues"; }
            const char * get_description() const noexcept override { return "Few values"; }
        };

        using ParseHookFunc = std::function<bool(PositionalArg * arg, int argc, const char * const argv[])>;

        /// Gets the number of values required by this argument on the command line.
        /// May return special values: OPTIONAL, UNLIMITED, UNLIMITED_BUT_ONE
        int get_nvals() const noexcept { return nvals; }

        /// Enables/disables storing parsed values.
        /// Values can be processed / stored in the user parse hook function.
        /// The "LogicError" exception is thrown when "true" is requested but the storage array "values" is not set.
        void set_store_value(bool enable);

        /// Returns true if storing the parsed values is enabled.
        bool get_store_value() const noexcept { return store_value; }

        /// Gets list of values.
        /// Parsed values are stored there if if get_store_value() == true.
        std::vector<std::unique_ptr<libdnf::Option>> * get_linked_values() noexcept { return values; }

        /// Sets the user function for parsing the argument.
        void set_parse_hook_func(ParseHookFunc && func) { parse_hook = std::move(func); }

    private:
        friend class ArgumentParser;

        PositionalArg(
            ArgumentParser & owner, const std::string & id, std::vector<std::unique_ptr<libdnf::Option>> * values);
        PositionalArg(
            ArgumentParser & owner,
            const std::string & id,
            int nvals,
            libdnf::Option * init_value,
            std::vector<std::unique_ptr<libdnf::Option>> * values);

        /// Parses input.
        /// Returns number of consumed arguments from the input.
        int parse(const char * option, int argc, const char * const argv[]);

        int nvals;
        libdnf::Option * init_value;
        std::vector<std::unique_ptr<libdnf::Option>> * values;
        bool store_value{true};
        ParseHookFunc parse_hook;
    };

    class NamedArg : public Argument {
    public:
        /// Exception is generated if the argument requires a value and the value is missing.
        class MissingValue : public Exception {
        public:
            using Exception::Exception;
            const char * get_domain_name() const noexcept override { return "libdnf::cli::ArgumentParser::NamedArg"; }
            const char * get_name() const noexcept override { return "MissingValue"; }
            const char * get_description() const noexcept override { return "Missing argument value"; }
        };

        /// Exception is generated if the argument is defined without a value and a value is present.
        class UnexpectedValue : public Exception {
        public:
            using Exception::Exception;
            const char * get_domain_name() const noexcept override { return "libdnf::cli::ArgumentParser::NamedArg"; }
            const char * get_name() const noexcept override { return "UnexpectedValue"; }
            const char * get_description() const noexcept override { return "Unexpected argument value"; }
        };

        using ParseHookFunc = std::function<bool(NamedArg * arg, const char * option, const char * value)>;

        /// Sets long name of argument. Long name is prefixed with two dashes on the command line (e.g. "--help").
        void set_long_name(std::string long_name) noexcept { this->long_name = std::move(long_name); }

        /// Sets short name of argument. Short name is prefixed with one dash on the command line (e.g. "-h").
        void set_short_name(char short_name) { this->short_name = short_name; }

        /// Does the argument need a value on the command line?
        void set_has_value(bool has_value) { this->has_value = has_value; }

        /// Links the value to the argument.
        /// Parsed value is stored there if if get_store_value() == true.
        void link_value(libdnf::Option * value) { this->value = value; }

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
        libdnf::Option * get_linked_value() noexcept { return value; }

        /// Gets value.
        /// Parsed value is stored there if if get_store_value() == true.
        const libdnf::Option * get_linked_value() const noexcept { return value; }

        /// Enables/disables storing parsed values.
        /// Values can be processed / stored in the user parse hook function.
        void set_store_value(bool enable) noexcept { store_value = enable; }

        /// Returns true if storing the parsed values is enabled.
        bool get_store_value() const noexcept { return store_value; }

        /// Sets the user function for parsing the argument.
        void set_parse_hook_func(ParseHookFunc && func) { parse_hook = std::move(func); }

        /// Sets help text for argument value.
        void set_arg_value_help(std::string text) { arg_value_help = std::move(text); }

        /// Gets help text for argument value.
        const std::string & get_arg_value_help() const noexcept { return arg_value_help; }

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
        libdnf::Option * value{nullptr};
        bool store_value{true};
        ParseHookFunc parse_hook;
        std::string arg_value_help;
    };

    class Command : public Argument {
    public:
        /// Exception is generated when a command was not found.
        class CommandNotFound : public Exception {
        public:
            using Exception::Exception;
            const char * get_domain_name() const noexcept override { return "libdnf::cli::ArgumentParser::Command"; }
            const char * get_name() const noexcept override { return "CommandNotFound"; }
            const char * get_description() const noexcept override { return "Commnand not found"; }
        };

        /// Exception is generated when a named argument was not found.
        class NamedArgNotFound : public Exception {
        public:
            using Exception::Exception;
            const char * get_domain_name() const noexcept override { return "libdnf::cli::ArgumentParser::Command"; }
            const char * get_name() const noexcept override { return "NamedArgNotFound"; }
            const char * get_description() const noexcept override { return "Named argument not found"; }
        };

        /// Exception is generated when a positional argument was not found.
        class PositionalArgNotFound : public Exception {
        public:
            using Exception::Exception;
            const char * get_domain_name() const noexcept override { return "libdnf::cli::ArgumentParser::Command"; }
            const char * get_name() const noexcept override { return "PositionalArgNotFound"; }
            const char * get_description() const noexcept override { return "Positional argument not found"; }
        };

        /// Exception is thrown when a command with the same ID is already registered.
        class CommandIdExists : public Exception {
        public:
            using Exception::Exception;
            const char * get_domain_name() const noexcept override { return "libdnf::cli::ArgumentParser::Command"; }
            const char * get_name() const noexcept override { return "CommandIdExists"; }
            const char * get_description() const noexcept override {
                return "Command with the same ID is already registered";
            }
        };

        /// Exception is thrown when a named argument with the same ID is already registered.
        class NamedArgIdExists : public Exception {
        public:
            using Exception::Exception;
            const char * get_domain_name() const noexcept override { return "libdnf::cli::ArgumentParser::Command"; }
            const char * get_name() const noexcept override { return "NamedArgIdExists"; }
            const char * get_description() const noexcept override {
                return "Named argument with the same ID is already registered";
            }
        };

        /// Exception is thrown when a positional argument with the same ID is already registered.
        class PositionalArgIdExists : public Exception {
        public:
            using Exception::Exception;
            const char * get_domain_name() const noexcept override { return "libdnf::cli::ArgumentParser::Command"; }
            const char * get_name() const noexcept override { return "PositionalArgIdExists"; }
            const char * get_description() const noexcept override {
                return "Positional argument with the same ID is already registered";
            }
        };

        using ParseHookFunc = std::function<bool(Command * arg, const char * cmd, int argc, const char * const argv[])>;

        /// Parses input. The input may contain named arguments, (sub)commands and positional arguments.
        void parse(const char * option, int argc, const char * const argv[]);

        /// Registers (sub)command to the command.
        /// An exception is thrown when a command with the same ID is already registered.
        void register_command(Command * cmd);

        /// Registers named argument to the command.
        /// An exception is thrown when a named argument with the same ID is already registered.
        void register_named_arg(NamedArg * arg);

        /// Registers positional argument to the command.
        /// An exception is thrown when a positional argument with the same ID is already registered.
        void register_positional_arg(PositionalArg * arg);

        /// Gets a list of registered commands.
        const std::vector<Command *> & get_commands() const noexcept { return cmds; }

        /// Gets a list of registered named arguments.
        const std::vector<NamedArg *> & get_named_args() const noexcept { return named_args; }

        /// Gets a list of registered positional arguments.
        const std::vector<PositionalArg *> & get_positional_args() const noexcept { return pos_args; }

        /// Returns (sub)command with given ID.
        /// Exception CommandNotFound is thrown if command is not found.
        Command & get_command(const std::string & id) const;

        /// Returns named argument with given ID.
        /// Exception NamedArgNotFound is thrown if argument is not found.
        NamedArg & get_named_arg(const std::string & id) const;

        /// Returns positional argument with given ID.
        /// Exception PositionalArgNotFound is thrown if argument is not found.
        PositionalArg & get_positional_arg(const std::string & id) const;

        /// Sets the user function for parsing the argument.
        void set_parse_hook_func(ParseHookFunc && func) { parse_hook = std::move(func); }

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

    private:
        /// Parses input. The input may contain named arguments, (sub)commands and positional arguments.
        void parse(
            const char * option,
            int argc,
            const char * const argv[],
            const std::vector<NamedArg *> * additional_named_args);

        friend class ArgumentParser;

        Command(ArgumentParser & owner, const std::string & id) : Argument(owner, id) {}

        std::vector<Command *> cmds;
        std::vector<NamedArg *> named_args;
        std::vector<PositionalArg *> pos_args;
        ParseHookFunc parse_hook;
        std::string commands_help_header;
        std::string named_args_help_header;
        std::string positional_args_help_header;
    };

    /// Constructs a new command and stores it to the argument parser.
    /// Returns a pointer to the newly created command.
    Command * add_new_command(const std::string & id);

    /// Constructs a new named argument and stores it to the argument parser.
    /// Returns a pointer to the newly created named argument.
    NamedArg * add_new_named_arg(const std::string & id);

    /// Constructs a new positional argument and stores it to the argument parser.
    /// Returns a pointer to the newly created positional argument.
    PositionalArg * add_new_positional_arg(
        const std::string & id, std::vector<std::unique_ptr<libdnf::Option>> * values);

    /// Constructs a new positional argument and stores it to the argument parser.
    /// Returns a pointer to the newly created positional argument.
    PositionalArg * add_new_positional_arg(
        const std::string & id,
        int nargs,
        libdnf::Option * init_value,
        std::vector<std::unique_ptr<libdnf::Option>> * values);

    /// Moves a list of conflicting argument to the parser.
    /// Returns a pointer to the list.
    std::vector<Argument *> * add_conflict_args_group(std::unique_ptr<std::vector<Argument *>> && conflict_args_group);

    /// Moves the option with the initial value to the argument parser.
    /// Returns a pointer to the option.
    libdnf::Option * add_init_value(std::unique_ptr<libdnf::Option> && src);

    /// Constructs a new empty list of values and stores it to the argument parser.
    /// Returns a pointer to the newly created list.
    std::vector<std::unique_ptr<libdnf::Option>> * add_new_values();

    /// Moves an existing list of values to the argument parser.
    /// Returns a pointer to the list.
    std::vector<std::unique_ptr<libdnf::Option>> * add_values(
        std::unique_ptr<std::vector<std::unique_ptr<libdnf::Option>>> && values);

    /// Sets the "root" command.
    /// This is the top-level command in the command hierarchy. It can contain named and positional arguments and subcommands.
    void set_root_command(Command * command) noexcept { root_command = command; }

    /// Gets the "root" command.
    /// This is the top-level command in the command hierarchy. It can contain named and positional arguments and subcommands.
    Command * get_root_command() noexcept { return root_command; }

    /// Parses input. The parser from the "root" command is called.
    /// The "LogicError" exception is thrown when the "root" command is not set.
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

private:
    std::vector<std::unique_ptr<Command>> cmds;
    std::vector<std::unique_ptr<NamedArg>> named_args;
    std::vector<std::unique_ptr<PositionalArg>> pos_args;
    std::vector<std::unique_ptr<std::vector<Argument *>>> conflict_args_groups;
    std::vector<std::unique_ptr<libdnf::Option>> values_init;
    std::vector<std::unique_ptr<std::vector<std::unique_ptr<libdnf::Option>>>> values;
    Command * root_command{nullptr};
    bool inherit_named_args{false};
};

}  // namespace libdnf::cli

#endif
