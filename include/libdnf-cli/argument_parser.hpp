/*
Copyright (C) 2019-2020 Red Hat, Inc.

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

#include "libdnf/conf/option.hpp"
#include "libdnf/utils/exception.hpp"

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

    /// Exception is generated in the case of an unexpected argument.
    class UnknownArgument : public Exception {
    public:
        using Exception::Exception;
        const char * get_name() const noexcept override { return "UnknownArgument"; }
        const char * get_description() const noexcept override { return "Unknown argument"; }
    };

    class Argument {
    public:
        explicit Argument(std::string name) : name(std::move(name)) {}
        Argument(const Argument &) = delete;
        Argument(Argument &&) = delete;
        Argument & operator=(const Argument &) = delete;
        Argument & operator=(Argument &&) = delete;
        virtual ~Argument() = default;
        void set_description(std::string descr) noexcept { description = std::move(descr); }
        void set_short_description(std::string descr) noexcept { short_description = std::move(descr); }
        void set_conflict_arguments(std::vector<Argument *> * args) noexcept { conflict_args = args; }
        const std::string & get_name() const noexcept { return name; }
        const std::string & get_description() const { return description; }
        const std::string & get_short_description() const { return short_description; }
        int get_parse_count() const noexcept { return parse_count; }
        void reset_parse_count() noexcept { parse_count = 0; }
        Argument * get_conflict_argument() const noexcept;
        static std::string get_conflict_arg_msg(const Argument * conflict_arg);
        virtual void help() const noexcept {}

    private:
        friend class ArgumentParser;
        std::string name;
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

        PositionalArg(const std::string & name, std::vector<std::unique_ptr<libdnf::Option>> * values);
        PositionalArg(
            const std::string & name,
            int nvals,
            libdnf::Option * init_value,
            std::vector<std::unique_ptr<libdnf::Option>> * values);

        void set_store_value(bool enable) noexcept { store_value = enable; }
        bool get_store_value() const noexcept { return store_value; }
        std::vector<std::unique_ptr<libdnf::Option>> * get_linked_values() noexcept { return values; }
        void set_parse_hook_func(ParseHookFunc && func) { parse_hook = std::move(func); }

        // returns number of consumed arguments
        int parse(const char * option, int argc, const char * const argv[]);

    private:
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

        explicit NamedArg(const std::string & name) : Argument(name) {}
        void set_long_name(std::string long_name) noexcept { this->long_name = std::move(long_name); }
        void set_short_name(char short_name) { this->short_name = short_name; }
        void set_has_value(bool has_value) { this->has_value = has_value; }
        void link_value(libdnf::Option * value) { this->value = value; }
        void set_const_value(std::string const_value) noexcept { const_val = std::move(const_value); }
        const std::string & get_long_name() const noexcept { return long_name; }
        char get_short_name() const noexcept { return short_name; }
        bool get_has_value() const noexcept { return has_value; }
        const std::string & get_const_value() const noexcept { return const_val; }
        libdnf::Option * get_linked_value() noexcept { return value; }
        const libdnf::Option * get_linked_value() const noexcept { return value; }
        void set_store_value(bool enable) noexcept { store_value = enable; }
        bool get_store_value() const noexcept { return store_value; }
        void set_parse_hook_func(ParseHookFunc && func) { parse_hook = std::move(func); }

        // returns number of consumed arguments
        int parse_long(const char * option, int argc, const char * const argv[]);
        // returns number of consumed arguments
        int parse_short(const char * option, int argc, const char * const argv[]);

        void set_arg_value_help(std::string text) { arg_value_help = std::move(text); }
        const std::string & get_arg_value_help() const noexcept { return arg_value_help; }

    private:
        friend class ArgumentParser;
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

        using ParseHookFunc = std::function<bool(Command * arg, const char * cmd, int argc, const char * const argv[])>;

        explicit Command(const std::string & name) : Argument(name) {}
        void parse(const char * option, int argc, const char * const argv[]);
        void add_command(Command * arg) { cmds.push_back(arg); }
        void add_named_arg(NamedArg * arg) { named_args.push_back(arg); }
        void add_positional_arg(PositionalArg * arg) { pos_args.push_back(arg); }
        const std::vector<Command *> & get_commands() const noexcept { return cmds; }
        const std::vector<NamedArg *> & get_named_args() const noexcept { return named_args; }
        const std::vector<PositionalArg *> & get_positional_args() const noexcept { return pos_args; }
        Command & get_command(const std::string & name) const;
        NamedArg & get_named_arg(const std::string & name) const;
        PositionalArg & get_positional_arg(const std::string & name) const;
        void set_parse_hook_func(ParseHookFunc && func) { parse_hook = std::move(func); }
        void help() const noexcept override;
        void set_commands_help_header(std::string text) noexcept { commands_help_header = std::move(text); }
        void set_named_args_help_header(std::string text) noexcept { named_args_help_header = std::move(text); }
        void set_positional_args_help_header(std::string text) noexcept {
            positional_args_help_header = std::move(text);
        }
        const std::string & get_commands_help_header() const noexcept { return commands_help_header; }
        const std::string & get_named_args_help_header() const noexcept { return named_args_help_header; }
        const std::string & get_positional_args_help_header() const noexcept { return positional_args_help_header; }

    private:
        friend class ArgumentParser;
        std::vector<Command *> cmds;
        std::vector<NamedArg *> named_args;
        std::vector<PositionalArg *> pos_args;
        ParseHookFunc parse_hook;
        std::string commands_help_header;
        std::string named_args_help_header;
        std::string positional_args_help_header;
    };

    Command * add_new_command(const std::string & name);
    NamedArg * add_new_named_arg(const std::string & name);
    PositionalArg * add_new_positional_arg(
        const std::string & name, std::vector<std::unique_ptr<libdnf::Option>> * values);
    PositionalArg * add_new_positional_arg(
        const std::string & name,
        int nargs,
        libdnf::Option * init_value,
        std::vector<std::unique_ptr<libdnf::Option>> * values);
    std::vector<Argument *> * add_conflict_args_group(std::unique_ptr<std::vector<Argument *>> && conflict_args_group);
    libdnf::Option * add_init_value(std::unique_ptr<libdnf::Option> && src);
    std::vector<std::unique_ptr<libdnf::Option>> * add_new_values();
    std::vector<std::unique_ptr<libdnf::Option>> * add_values(
        std::unique_ptr<std::vector<std::unique_ptr<libdnf::Option>>> && values);

    void set_root_command(Command * command) noexcept { root_command = command; }
    Command * get_root_command() noexcept { return root_command; }
    void parse(int argc, const char * const argv[]);
    void reset_parse_count();

private:
    std::vector<std::unique_ptr<Command>> cmds;
    std::vector<std::unique_ptr<NamedArg>> named_args;
    std::vector<std::unique_ptr<PositionalArg>> pos_args;
    std::vector<std::unique_ptr<std::vector<Argument *>>> conflict_args_groups;
    std::vector<std::unique_ptr<libdnf::Option>> values_init;
    std::vector<std::unique_ptr<std::vector<std::unique_ptr<libdnf::Option>>>> values;
    Command * root_command{nullptr};
};

inline ArgumentParser::Command * ArgumentParser::add_new_command(const std::string & name) {
    std::unique_ptr<Command> arg(new Command(name));
    auto * ptr = arg.get();
    cmds.push_back(std::move(arg));
    return ptr;
}

inline ArgumentParser::NamedArg * ArgumentParser::add_new_named_arg(const std::string & name) {
    std::unique_ptr<NamedArg> arg(new NamedArg(name));
    auto * ptr = arg.get();
    named_args.push_back(std::move(arg));
    return ptr;
}

inline ArgumentParser::PositionalArg * ArgumentParser::add_new_positional_arg(
    const std::string & name, std::vector<std::unique_ptr<libdnf::Option>> * values) {
    std::unique_ptr<PositionalArg> arg(new PositionalArg(name, values));
    auto * ptr = arg.get();
    pos_args.push_back(std::move(arg));
    return ptr;
}

inline ArgumentParser::PositionalArg * ArgumentParser::add_new_positional_arg(
    const std::string & name,
    int nargs,
    libdnf::Option * init_value,
    std::vector<std::unique_ptr<libdnf::Option>> * values) {
    std::unique_ptr<PositionalArg> arg(new PositionalArg(name, nargs, init_value, values));
    auto * ptr = arg.get();
    pos_args.push_back(std::move(arg));
    return ptr;
}

inline std::vector<ArgumentParser::Argument *> * ArgumentParser::add_conflict_args_group(
    std::unique_ptr<std::vector<Argument *>> && conflict_args_group) {
    auto * ptr = conflict_args_group.get();
    conflict_args_groups.push_back(std::move(conflict_args_group));
    return ptr;
}

inline libdnf::Option * ArgumentParser::add_init_value(std::unique_ptr<libdnf::Option> && src) {
    auto * ptr = src.get();
    values_init.push_back(std::move(src));
    return ptr;
}

inline std::vector<std::unique_ptr<libdnf::Option>> * ArgumentParser::add_new_values() {
    std::unique_ptr<std::vector<std::unique_ptr<libdnf::Option>>> tmp(new std::vector<std::unique_ptr<libdnf::Option>>);
    auto * ptr = tmp.get();
    values.push_back(std::move(tmp));
    return ptr;
}

inline std::vector<std::unique_ptr<libdnf::Option>> * ArgumentParser::add_values(
    std::unique_ptr<std::vector<std::unique_ptr<libdnf::Option>>> && values) {
    auto * ptr = values.get();
    this->values.push_back(std::move(values));
    return ptr;
}

}  // namespace libdnf::cli

#endif
