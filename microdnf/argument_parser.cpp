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

#include "argument_parser.hpp"

#include <fmt/format.h>

#include <cstring>
#include <iomanip>
#include <iostream>

#include "libdnf-cli/output/argument_parser.hpp"

namespace microdnf {

ArgumentParser::Argument * ArgumentParser::Argument::get_conflict_argument() const {
    if (conflict_args) {
        for (auto arg : *conflict_args) {
            if (arg != this && arg->get_parse_count() > 0) {
                return arg;
            }
        }
    }
    return nullptr;
}

std::string ArgumentParser::Argument::get_conflict_arg_msg(Argument * conflict_arg) {
    std::string msg;
    if (auto named_arg = dynamic_cast<NamedArg *>(conflict_arg)) {
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
    } else if (dynamic_cast<Command *>(conflict_arg)) {
        msg = fmt::format("not allowed with command {}", conflict_arg->name);
    } else {
        msg = fmt::format("not allowed with positional argument {}", conflict_arg->name);
    }
    return msg;
}

ArgumentParser::PositionalArg::PositionalArg(
    const std::string & name, std::vector<std::unique_ptr<libdnf::Option>> * values)
    : Argument(name)
    , nargs(static_cast<int>(values->size()))
    , init_value(nullptr)
    , values(values) {
    if (!values || values->empty()) {
        throw std::runtime_error("PositionalArg: Error: values constructor parameter can't be nullptr or empty vector");
    }
}

ArgumentParser::PositionalArg::PositionalArg(
    const std::string & name,
    int nargs,
    libdnf::Option * init_value,
    std::vector<std::unique_ptr<libdnf::Option>> * values)
    : Argument(name)
    , nargs(nargs)
    , init_value(init_value)
    , values(values) {
    if (!values) {
        throw std::runtime_error("PositionalArg: Error: values constructor parameter can't be nullptr");
    }
}

int ArgumentParser::PositionalArg::parse(const char * option, int argc, const char * const argv[]) {
    if (auto arg = get_conflict_argument()) {
        auto conflict = get_conflict_arg_msg(arg);
        auto msg = fmt::format("positional argument \"{}\": {}", option, conflict);
        throw std::runtime_error(msg);
    }
    if (argc < nargs) {
        throw std::runtime_error("Not enough parameters");
    }
    size_t count = nargs > 0 ? nargs : (nargs == OPTIONAL ? 1 : argc);
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
        parse_hook(this, argc, argv);
    }
    return static_cast<int>(count);
}

int ArgumentParser::NamedArg::parse_long(const char * option, int argc, const char * const argv[]) {
    if (auto arg = get_conflict_argument()) {
        auto conflict = get_conflict_arg_msg(arg);
        auto msg = fmt::format("argument \"--{}\": {}", option, conflict);
        throw std::runtime_error(msg);
    }
    const char * arg_value;
    int consumed_args;
    auto assign_ptr = strchr(option, '=');
    if (has_arg) {
        if (assign_ptr) {
            arg_value = assign_ptr + 1;
            consumed_args = 1;
        } else {
            if (argc < 2) {
                throw std::runtime_error("Not enough arguments");
            }
            arg_value = argv[1];
            consumed_args = 2;
        }
    } else {
        if (assign_ptr) {
            throw std::runtime_error(fmt::format("NamedArg: Error: Unexpected argument \"{}\"", assign_ptr + 1));
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
    if (auto arg = get_conflict_argument()) {
        auto conflict = get_conflict_arg_msg(arg);
        auto msg = fmt::format("argument \"-{}\": {:.1}", option, conflict);
        throw std::runtime_error(msg);
    }
    const char * arg_value;
    int consumed_args;
    if (has_arg) {
        if (option[1] != '\0') {
            arg_value = option + 1;
            consumed_args = 1;
        } else {
            if (argc < 2) {
                throw std::runtime_error("Not enough arguments");
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
    for (auto item : cmds) {
        if (item->get_name() == name) {
            return *item;
        }
    }
    throw std::runtime_error("Commnand not found");
}

ArgumentParser::NamedArg & ArgumentParser::Command::get_named_arg(const std::string & name) const {
    for (auto item : named_args) {
        if (item->get_name() == name) {
            return *item;
        }
    }
    throw std::runtime_error("Named argument not found");
}

ArgumentParser::PositionalArg & ArgumentParser::Command::get_positional_arg(const std::string & name) const {
    for (auto item : pos_args) {
        if (item->get_name() == name) {
            return *item;
        }
    }
    throw std::runtime_error("Positional argument not found");
}

void ArgumentParser::Command::parse(const char * option, int argc, const char * const argv[]) {
    size_t used_values = 0;
    int short_option_idx = 0;
    for (int i = 1; i < argc;) {
        bool used = false;
        auto tmp = argv[i];
        if (*tmp == '-') {
            bool long_option = *++tmp == '-';
            if (long_option) {
                ++tmp;
            }
            auto assign_ptr = strchr(tmp, '=');
            for (auto opt : named_args) {
                if (long_option) {
                    if (!opt->get_long_name().empty() &&
                        (assign_ptr ? std::string(tmp).compare(0, assign_ptr - tmp, opt->get_long_name()) == 0
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
                    if (auto arg = get_conflict_argument()) {
                        auto conflict = get_conflict_arg_msg(arg);
                        auto msg = fmt::format("command \"{}\": {}", option, conflict);
                        throw std::runtime_error(msg);
                    }
                    cmd->parse(argv[i], argc - i, &argv[i]);
                    i = argc;
                    used = true;
                    break;
                }
            }
        }
        if (!used && used_values < pos_args.size()) {
            i += pos_args[used_values]->parse(argv[i], argc - i, &argv[i]);
            ++used_values;
        }
        if (!used) {
            ++i;
        }
    }
    ++parse_count;
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
        auto table = libdnf::cli::output::create_help_table(commands_help_header);
        auto out = libdnf::cli::output::get_stream(table);
        if (print) {
            fputs("\n", out);
        }
        fputs((commands_help_header + '\n').c_str(), out);
        for (auto arg : cmds) {
            libdnf::cli::output::add_line_into_help_table(table, arg->get_name(), arg->get_short_description());
        }
        libdnf::cli::output::print_and_unref_help_table(table);
        print = true;
    }

    if (!named_args_help_header.empty()) {
        auto table = libdnf::cli::output::create_help_table(named_args_help_header);
        auto out = libdnf::cli::output::get_stream(table);
        if (print) {
            fputs("\n", out);
        }
        fputs((named_args_help_header + '\n').c_str(), out);
        for (auto arg : named_args) {
            std::string arg_names;
            if (arg->get_short_name() != '\0') {
                arg_names = std::string("-") + arg->get_short_name();
                if (arg->get_has_arg()) {
                    arg_names += arg->arg_value_help.empty() ? " VALUE" : ' ' + arg->arg_value_help;
                }
                if (!arg->get_long_name().empty()) {
                    arg_names += ", ";
                }
            }
            if (!arg->get_long_name().empty()) {
                arg_names += "--" + arg->get_long_name();
                if (arg->get_has_arg()) {
                    arg_names += arg->arg_value_help.empty() ? "=VALUE" : '=' + arg->arg_value_help;
                }
            }
            libdnf::cli::output::add_line_into_help_table(table, "  " + arg_names, arg->get_short_description());
        }
        libdnf::cli::output::print_and_unref_help_table(table);
        print = true;
    }

    if (!positional_args_help_header.empty()) {
        auto table = libdnf::cli::output::create_help_table(named_args_help_header);
        auto out = scols_table_get_stream(table);
        if (print) {
            fputs("\n", out);
        }
        fputs((positional_args_help_header + '\n').c_str(), out);
        for (auto arg : pos_args) {
            libdnf::cli::output::add_line_into_help_table(table, "  " + arg->get_name(), arg->get_short_description());
        }
        libdnf::cli::output::print_and_unref_help_table(table);
    }
}


void ArgumentParser::parse(int argc, const char * const argv[]) {
    if (!root_command) {
        throw std::runtime_error("ArgumentParser: Error: root command is not set");
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


}  // namespace microdnf
