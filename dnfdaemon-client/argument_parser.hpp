/*
Copyright (C) 2020 Red Hat, Inc.

This file is part of dnfdaemon-client: https://github.com/rpm-software-management/libdnf/

Dnfdaemon-client is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Dnfdaemon-client is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with dnfdaemon-client.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef DNFDAEMON_CLIENT_ARGUMENT_PARSER_HPP
#define DNFDAEMON_CLIENT_ARGUMENT_PARSER_HPP

#include <libdnf/conf/option.hpp>

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace dnfdaemon::client {

class ArgumentParser {
public:
    class Argument {
    public:
        explicit Argument(std::string name) : name(std::move(name)) {}
        virtual ~Argument() = default;
        void set_description(const std::string & descr) { description = descr; }
        void set_description(std::string && descr) noexcept { description = std::move(descr); }
        void set_short_description(const std::string & descr) { short_description = descr; }
        void set_short_description(std::string && descr) noexcept { short_description = std::move(descr); }
        void set_conflict_arguments(std::vector<Argument *> * args) noexcept { conflict_args = args; }
        const std::string & get_name() const noexcept { return name; }
        const std::string & get_description() const { return description; }
        const std::string & get_short_description() const { return short_description; }
        int get_parse_count() const noexcept { return parse_count; }
        void reset_parse_count() noexcept { parse_count = 0; }
        Argument * get_conflict_argument() const;
        static std::string get_conflict_arg_msg(Argument * conflict_arg);
        virtual void help() const noexcept {}

    protected:
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

        PositionalArg(const std::string & name, std::vector<std::unique_ptr<libdnf::Option>> * values);
        PositionalArg(
            const std::string & name,
            int nargs,
            libdnf::Option * init_value,
            std::vector<std::unique_ptr<libdnf::Option>> * values);

        void set_store_value(bool enable) noexcept { store_value = enable; }
        bool get_store_value() const noexcept { return store_value; }
        std::vector<std::unique_ptr<libdnf::Option>> * get_linked_values() noexcept { return values; }
        std::function<bool(PositionalArg * arg, int argc, const char * const argv[])> parse_hook;

        // returns number of consumed arguments
        int parse(const char * option, int argc, const char * const argv[]);

    private:
        int nargs;
        libdnf::Option * init_value;
        std::vector<std::unique_ptr<libdnf::Option>> * values;
        bool store_value{true};
    };

    class NamedArg : public Argument {
    public:
        explicit NamedArg(const std::string & name) : Argument(name) {}
        void set_long_name(const std::string & long_name) { this->long_name = long_name; }
        void set_long_name(std::string && long_name) noexcept { this->long_name = std::move(long_name); }
        void set_short_name(char short_name) { this->short_name = short_name; }
        void set_has_arg(bool has_arg) { this->has_arg = has_arg; }
        void link_value(libdnf::Option * value) { this->value = value; }
        void set_const_value(const std::string & const_value) { const_val = const_value; }
        void set_const_value(std::string && const_value) noexcept { const_val = std::move(const_value); }
        const std::string & get_long_name() const noexcept { return long_name; }
        char get_short_name() const noexcept { return short_name; }
        bool get_has_arg() { return has_arg; }
        const std::string & get_const_value() const noexcept { return const_val; }
        libdnf::Option * get_linked_value() noexcept { return value; }
        const libdnf::Option * get_linked_value() const noexcept { return value; }
        void set_store_value(bool enable) noexcept { store_value = enable; }
        bool get_store_value() const noexcept { return store_value; }
        std::function<bool(NamedArg * arg, const char * option, const char * value)> parse_hook;

        // returns number of consumed arguments
        int parse_long(const char * option, int argc, const char * const argv[]);
        // returns number of consumed arguments
        int parse_short(const char * option, int argc, const char * const argv[]);

        std::string arg_value_help;

    private:
        std::string long_name;
        char short_name{'\0'};
        bool has_arg{false};
        std::string const_val;  // used if params == 0
        libdnf::Option * value{nullptr};
        bool store_value{true};
    };

    class Command : public Argument {
    public:
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
        std::function<bool(Command * arg, const char * cmd, int argc, const char * const argv[])> parse_hook;
        void help() const noexcept override;

        std::string commands_help_header;
        std::string named_args_help_header;
        std::string positional_args_help_header;

    private:
        std::vector<Command *> cmds;
        std::vector<NamedArg *> named_args;
        std::vector<PositionalArg *> pos_args;
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

    void set_root_command(Command * command);
    Command * get_root_command();
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
    auto ptr = arg.get();
    cmds.push_back(std::move(arg));
    return ptr;
}

inline ArgumentParser::NamedArg * ArgumentParser::add_new_named_arg(const std::string & name) {
    std::unique_ptr<NamedArg> arg(new NamedArg(name));
    auto ptr = arg.get();
    named_args.push_back(std::move(arg));
    return ptr;
}

inline ArgumentParser::PositionalArg * ArgumentParser::add_new_positional_arg(
    const std::string & name, std::vector<std::unique_ptr<libdnf::Option>> * values) {
    std::unique_ptr<PositionalArg> arg(new PositionalArg(name, values));
    auto ptr = arg.get();
    pos_args.push_back(std::move(arg));
    return ptr;
}

inline ArgumentParser::PositionalArg * ArgumentParser::add_new_positional_arg(
    const std::string & name,
    int nargs,
    libdnf::Option * init_value,
    std::vector<std::unique_ptr<libdnf::Option>> * values) {
    std::unique_ptr<PositionalArg> arg(new PositionalArg(name, nargs, init_value, values));
    auto ptr = arg.get();
    pos_args.push_back(std::move(arg));
    return ptr;
}

inline std::vector<ArgumentParser::Argument *> * ArgumentParser::add_conflict_args_group(
    std::unique_ptr<std::vector<Argument *>> && conflict_args_group) {
    auto ptr = conflict_args_group.get();
    conflict_args_groups.push_back(std::move(conflict_args_group));
    return ptr;
}

inline libdnf::Option * ArgumentParser::add_init_value(std::unique_ptr<libdnf::Option> && src) {
    auto ptr = src.get();
    values_init.push_back(std::move(src));
    return ptr;
}

inline std::vector<std::unique_ptr<libdnf::Option>> * ArgumentParser::add_new_values() {
    std::unique_ptr<std::vector<std::unique_ptr<libdnf::Option>>> tmp(new std::vector<std::unique_ptr<libdnf::Option>>);
    auto ptr = tmp.get();
    values.push_back(std::move(tmp));
    return ptr;
}

inline std::vector<std::unique_ptr<libdnf::Option>> * ArgumentParser::add_values(
    std::unique_ptr<std::vector<std::unique_ptr<libdnf::Option>>> && src) {
    auto ptr = src.get();
    values.push_back(std::move(src));
    return ptr;
}


inline void ArgumentParser::set_root_command(Command * command) {
    root_command = command;
}

inline ArgumentParser::Command * ArgumentParser::get_root_command() {
    return root_command;
}

}  // namespace dnfdaemon::client

#endif
