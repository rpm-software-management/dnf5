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

#include "libdnf-cli/argument_parser.hpp"
#include "libdnf-cli/output/argument_parser.hpp"

#include <fmt/format.h>

#include <cstring>
#include <iomanip>
#include <iostream>

namespace libdnf::cli {

ArgumentParser::Argument * ArgumentParser::Argument::get_conflict_argument() const noexcept {
    if (conflict_args) {
        for (auto * arg : *conflict_args) {
            if (arg != this && arg->get_parse_count() > 0) {
                return arg;
            }
        }
    }
    return nullptr;
}

std::string ArgumentParser::Argument::get_conflict_arg_msg(const Argument * conflict_arg) {
    std::string msg;
    if (const auto * named_arg = dynamic_cast<const NamedArg *>(conflict_arg)) {
        std::string conflict;
        if (!named_arg->get_long_name().empty()) {
            conflict = "\"--" + named_arg->get_long_name() + "\"";
        }
        if (!named_arg->get_long_name().empty() && named_arg->get_short_name() != '\0') {
            conflict += "/";
        }
        if (named_arg->get_short_name() != '\0') {
            conflict = std::string("\"-") + named_arg->get_short_name() + "\"";
        }
        msg = fmt::format("not allowed with argument {}", conflict);
    } else if (dynamic_cast<const Command *>(conflict_arg)) {
        msg = fmt::format("not allowed with command {}", conflict_arg->name);
    } else {
        msg = fmt::format("not allowed with positional argument {}", conflict_arg->name);
    }
    return msg;
}

ArgumentParser::PositionalArg::PositionalArg(
    ArgumentParser & owner, const std::string & name, std::vector<std::unique_ptr<libdnf::Option>> * values)
    : Argument(owner, name)
    , nvals(static_cast<int>(values->size()))
    , init_value(nullptr)
    , values(values) {
    if (!values || values->empty()) {
        throw LogicError("PositionalArg: \"values\" constructor parameter can't be nullptr or empty vector");
    }
}

ArgumentParser::PositionalArg::PositionalArg(
    ArgumentParser & owner,
    const std::string & name,
    int nvals,
    libdnf::Option * init_value,
    std::vector<std::unique_ptr<libdnf::Option>> * values)
    : Argument(owner, name)
    , nvals(nvals)
    , init_value(init_value)
    , values(values) {
    if (!values) {
        throw LogicError("PositionalArg: \"values\" constructor parameter can't be nullptr");
    }
}

int ArgumentParser::PositionalArg::parse(const char * option, int argc, const char * const argv[]) {
    if (const auto * arg = get_conflict_argument()) {
        auto conflict = get_conflict_arg_msg(arg);
        auto msg = fmt::format("positional argument \"{}\": {}", option, conflict);
        throw Conflict(msg);
    }
    if (argc < nvals) {
        throw FewValues(this->name);
    }
    for (int i = 1; i < nvals; ++i) {
        if (*argv[i] == '-') {
            throw FewValues(this->name);
        }
    }
    int usable_argc = 1;
    if (nvals <= 0) {
        while (usable_argc < argc && *argv[usable_argc] != '-') {
            ++usable_argc;
        }
    }
    auto count = static_cast<size_t>(nvals > 0 ? nvals : (nvals == OPTIONAL ? 1 : usable_argc));
    if (store_value) {
        for (size_t i = 0; i < count; ++i) {
            if (values->size() <= i) {
                values->push_back(std::unique_ptr<libdnf::Option>((*init_value).clone()));
            }
            (*values)[i]->set(libdnf::Option::Priority::COMMANDLINE, argv[i]);
        }
    }
    ++parse_count;
    if (parse_hook) {
        parse_hook(this, usable_argc, argv);
    }
    return static_cast<int>(count);
}

int ArgumentParser::NamedArg::parse_long(const char * option, int argc, const char * const argv[]) {
    if (const auto * arg = get_conflict_argument()) {
        auto conflict = get_conflict_arg_msg(arg);
        auto msg = fmt::format("argument \"--{}\": {}", option, conflict);
        throw Conflict(msg);
    }
    const char * arg_value;
    int consumed_args;
    const auto * assign_ptr = strchr(option, '=');
    if (has_value) {
        if (assign_ptr) {
            arg_value = assign_ptr + 1;
            consumed_args = 1;
        } else {
            if (argc < 2) {
                throw MissingValue(std::string("--") + option);
            }
            arg_value = argv[1];
            consumed_args = 2;
        }
    } else {
        if (assign_ptr) {
            throw UnexpectedValue(std::string("--") + option);
        }
        arg_value = const_val.c_str();
        consumed_args = 1;
    }
    if (store_value && value) {
        value->set(libdnf::Option::Priority::COMMANDLINE, arg_value);
    }
    ++parse_count;
    if (parse_hook) {
        parse_hook(this, option, arg_value);
    }
    return consumed_args;
}


int ArgumentParser::NamedArg::parse_short(const char * option, int argc, const char * const argv[]) {
    if (const auto * arg = get_conflict_argument()) {
        auto conflict = get_conflict_arg_msg(arg);
        auto msg = fmt::format("argument \"-{}\": {:.1}", option, conflict);
        throw Conflict(msg);
    }
    const char * arg_value;
    int consumed_args;
    if (has_value) {
        if (option[1] != '\0') {
            arg_value = option + 1;
            consumed_args = 1;
        } else {
            if (argc < 2) {
                throw MissingValue(std::string("-") + *option);
            }
            arg_value = argv[1];
            consumed_args = 2;
        }
    } else {
        arg_value = const_val.c_str();
        consumed_args =
            option[1] == '\0' ? 1 : 0;  // consume only if we are the last option in grop, example of 3 options: -cvf
    }
    if (store_value && value) {
        value->set(libdnf::Option::Priority::COMMANDLINE, arg_value);
    }
    ++parse_count;
    if (parse_hook) {
        parse_hook(this, option, arg_value);
    }
    return consumed_args;
}

ArgumentParser::Command & ArgumentParser::Command::get_command(const std::string & name) const {
    for (auto * item : cmds) {
        if (item->get_name() == name) {
            return *item;
        }
    }
    throw CommandNotFound(name);
}

ArgumentParser::NamedArg & ArgumentParser::Command::get_named_arg(const std::string & name) const {
    for (auto * item : named_args) {
        if (item->get_name() == name) {
            return *item;
        }
    }
    throw NamedArgNotFound(name);
}

ArgumentParser::PositionalArg & ArgumentParser::Command::get_positional_arg(const std::string & name) const {
    for (auto * item : pos_args) {
        if (item->get_name() == name) {
            return *item;
        }
    }
    throw PositionalArgNotFound(name);
}

void ArgumentParser::Command::parse(const char * option, int argc, const char * const argv[]) {
    size_t used_positional_arguments = 0;
    int short_option_idx = 0;
    for (int i = 1; i < argc;) {
        bool used = false;
        const auto * tmp = argv[i];
        if (*tmp == '-') {
            bool long_option = *++tmp == '-';
            if (long_option) {
                ++tmp;
            }
            const auto * assign_ptr = strchr(tmp, '=');
            for (auto * opt : named_args) {
                if (long_option) {
                    if (!opt->get_long_name().empty() &&
                        (assign_ptr ? std::string(tmp).compare(
                                          0, static_cast<size_t>(assign_ptr - tmp), opt->get_long_name()) == 0
                                    : opt->get_long_name() == tmp)) {
                        i += opt->parse_long(tmp, argc - i, &argv[i]);
                        used = true;
                        break;
                    }
                } else {
                    if (opt->get_short_name() != '\0' && opt->get_short_name() == tmp[short_option_idx]) {
                        auto used_args = opt->parse_short(tmp + short_option_idx, argc - i, &argv[i]);
                        if (used_args > 0) {
                            i += used_args;
                            short_option_idx = 0;
                        } else {
                            ++short_option_idx;
                        }
                        used = true;
                        break;
                    }
                }
            }
        }
        if (!used) {
            for (auto & cmd : cmds) {
                if (cmd->name == argv[i]) {
                    if (const auto * arg = get_conflict_argument()) {
                        auto conflict = get_conflict_arg_msg(arg);
                        auto msg = fmt::format("command \"{}\": {}", option, conflict);
                        throw Conflict(msg);
                    }
                    cmd->parse(argv[i], argc - i, &argv[i]);
                    i = argc;
                    used = true;
                    break;
                }
            }
        }
        if (!used && *argv[i] != '-' && used_positional_arguments < pos_args.size()) {
            i += pos_args[used_positional_arguments]->parse(argv[i], argc - i, &argv[i]);
            ++used_positional_arguments;
            used = true;
        }
        if (!used) {
            throw UnknownArgument(argv[i]);
        }
    }
    ++parse_count;

    // Test that all required positional arguments are present.
    for (const auto * pos_arg : pos_args) {
        const auto nvals = pos_arg->get_nvals();
        if (pos_arg->get_parse_count() == 0 && nvals != PositionalArg::UNLIMITED && nvals != PositionalArg::OPTIONAL) {
            throw MissingPositionalArgument(pos_arg->get_name());
        }
    }

    if (parse_hook) {
        parse_hook(this, option, argc, argv);
    }
}


void ArgumentParser::Command::help() const noexcept {
    bool print = false;
    std::cout.flags(std::ios::left);

    if (!description.empty()) {
        std::cout << description << '\n';
        print = true;
    }

    if (!commands_help_header.empty()) {
        auto * table = libdnf::cli::output::create_help_table(commands_help_header);
        auto * out = libdnf::cli::output::get_stream(table);
        if (print) {
            fputs("\n", out);
        }
        fputs((commands_help_header + '\n').c_str(), out);
        for (const auto * arg : cmds) {
            libdnf::cli::output::add_line_into_help_table(table, arg->get_name(), arg->get_short_description());
        }
        libdnf::cli::output::print_and_unref_help_table(table);
        print = true;
    }

    if (!named_args_help_header.empty()) {
        auto * table = libdnf::cli::output::create_help_table(named_args_help_header);
        auto * out = libdnf::cli::output::get_stream(table);
        if (print) {
            fputs("\n", out);
        }
        fputs((named_args_help_header + '\n').c_str(), out);
        for (const auto * arg : named_args) {
            std::string arg_names;
            if (arg->get_short_name() != '\0') {
                arg_names = std::string("-") + arg->get_short_name();
                if (arg->get_has_value()) {
                    arg_names += arg->arg_value_help.empty() ? " VALUE" : ' ' + arg->arg_value_help;
                }
                if (!arg->get_long_name().empty()) {
                    arg_names += ", ";
                }
            }
            if (!arg->get_long_name().empty()) {
                arg_names += "--" + arg->get_long_name();
                if (arg->get_has_value()) {
                    arg_names += arg->arg_value_help.empty() ? "=VALUE" : '=' + arg->arg_value_help;
                }
            }
            libdnf::cli::output::add_line_into_help_table(table, "  " + arg_names, arg->get_short_description());
        }
        libdnf::cli::output::print_and_unref_help_table(table);
        print = true;
    }

    if (!positional_args_help_header.empty()) {
        auto * table = libdnf::cli::output::create_help_table(named_args_help_header);
        auto * out = scols_table_get_stream(table);
        if (print) {
            fputs("\n", out);
        }
        fputs((positional_args_help_header + '\n').c_str(), out);
        for (const auto * arg : pos_args) {
            libdnf::cli::output::add_line_into_help_table(table, "  " + arg->get_name(), arg->get_short_description());
        }
        libdnf::cli::output::print_and_unref_help_table(table);
    }
}


void ArgumentParser::parse(int argc, const char * const argv[]) {
    if (!root_command) {
        throw LogicError("root command is not set");
    }
    root_command->parse(argv[0], argc, argv);
}

void ArgumentParser::reset_parse_count() {
    for (auto & i : pos_args) {
        i->reset_parse_count();
    }
    for (auto & i : named_args) {
        i->reset_parse_count();
    }
    for (auto & i : cmds) {
        i->reset_parse_count();
    }
}

}  // namespace libdnf::cli
