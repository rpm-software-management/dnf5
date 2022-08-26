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


#include "libdnf-cli/argument_parser.hpp"

#include "output/argument_parser.hpp"
#include "utils/bgettext/bgettext-lib.h"
#include "utils/bgettext/bgettext-mark-domain.h"
#include "utils/string.hpp"

#include "libdnf/common/exception.hpp"

#include <fmt/format.h>

#include <cstring>
#include <iomanip>
#include <iostream>
#include <set>
#include <utility>

namespace libdnf::cli {

namespace {

std::pair<BgettextMessage, std::string> get_conflict_arg_msg(const ArgumentParser::Argument * conflict_arg) {
    if (const auto * named_arg = dynamic_cast<const ArgumentParser::NamedArg *>(conflict_arg)) {
        std::string conflict;
        if (!named_arg->get_long_name().empty()) {
            conflict = "--" + named_arg->get_long_name();
        }
        if (!named_arg->get_long_name().empty() && named_arg->get_short_name() != '\0') {
            conflict += "/";
        }
        if (named_arg->get_short_name() != '\0') {
            conflict = std::string("-") + named_arg->get_short_name();
        }
        return {M_("\"{}\" not allowed together with named argument \"{}\""), conflict};
    } else if (dynamic_cast<const ArgumentParser::Command *>(conflict_arg)) {
        return {M_("\"{}\" not allowed in command \"{}\""), conflict_arg->get_id()};
    } else {
        return {M_("\"{}\" not allowed together with positional argument \"{}\""), conflict_arg->get_id()};
    }
}

}  // namespace

void ArgumentParser::Group::register_argument(Argument * arg) {
    for (auto * item : arguments) {
        if (item->id == arg->id) {
            throw ArgumentParserGroupArgumentIdRegisteredError(
                M_("Argument id \"{}\" already registered in group \"{}\""), arg->id, id);
        }
    }
    arguments.push_back(arg);
}

ArgumentParser::Argument::Argument(ArgumentParser & owner, std::string id) : owner(owner) {
    if (id.find('.') != id.npos) {
        throw ArgumentParserArgumentInvalidIdError(M_("Invalid character '.' in argument id \"{}\""), id);
    }
    this->id = std::move(id);
}

void ArgumentParser::Argument::add_conflict_argument(ArgumentParser::Argument & conflict_arg) {
    auto add_conflict = [](Argument & arg, Argument & conflict_arg) {
        if (arg.conflict_args) {
            auto it = std::find(arg.conflict_args->begin(), arg.conflict_args->end(), &conflict_arg);
            if (it == arg.conflict_args->end()) {
                arg.conflict_args->push_back(&conflict_arg);
            }
        } else {
            // Creates a new group of conflict arguments if it does not exist
            arg.conflict_args = arg.owner.add_conflict_args_group(std::make_unique<std::vector<Argument *>>());
            arg.conflict_args->push_back(&conflict_arg);
        }
    };

    // Adds forward conflict
    add_conflict(*this, conflict_arg);

    // Adds reverse (back) conflict
    add_conflict(conflict_arg, *this);
}

void ArgumentParser::Argument::add_conflict_arguments_from_another(Argument & src_arg) {
    // Return immediately if there are no conflicting arguments in the source argument or it is a self-assignment.
    if (!src_arg.conflict_args || &src_arg == this) {
        return;
    }

    for (std::size_t idx = 0; idx < src_arg.conflict_args->size(); ++idx) {
        auto * conflict_arg = (*src_arg.conflict_args)[idx];

        // Don't add a conflict to the argument itself and to the source argument.
        if (conflict_arg == this || conflict_arg == &src_arg) {
            continue;
        }

        add_conflict_argument(*conflict_arg);
    }
}

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

ArgumentParser::PositionalArg::PositionalArg(
    ArgumentParser & owner, const std::string & id, std::vector<std::unique_ptr<libdnf::Option>> * values)
    : Argument(owner, id),
      init_value(nullptr),
      values(values) {
    libdnf_assert(values && !values->empty(), "\"values\" constructor parameter cannot be nullptr or an empty vector");

    nvals = static_cast<int>(values->size());
}

ArgumentParser::PositionalArg::PositionalArg(
    ArgumentParser & owner,
    const std::string & id,
    int nvals,
    libdnf::Option * init_value,
    std::vector<std::unique_ptr<libdnf::Option>> * values)
    : Argument(owner, id),
      nvals(nvals),
      init_value(init_value),
      values(values),
      store_value(values) {
    libdnf_assert(!values || init_value, "\"init_value\" constructor parameter cannot be nullptr if \"value\" is set");
}

void ArgumentParser::PositionalArg::set_store_value(bool enable) {
    libdnf_assert(!enable || values, "set_store_value(true) was called but storage array \"values\" is not set");

    store_value = enable;
}

int ArgumentParser::PositionalArg::parse(const char * option, int argc, const char * const argv[]) {
    if (const auto * arg = get_conflict_argument()) {
        auto [fmt_message, conflict_option] = get_conflict_arg_msg(arg);
        throw ArgumentParserConflictingArgumentsError(fmt_message, std::string(option), conflict_option);
    }
    if (owner.complete_arg_ptr) {
        int usable_argc = 1;
        while (usable_argc < argc && *argv[usable_argc] != '-') {
            ++usable_argc;
        }
        auto count = static_cast<size_t>(nvals > 0 ? nvals : (nvals == OPTIONAL ? 1 : usable_argc));
        for (size_t i = 0; i < count; ++i) {
            if (owner.complete_arg_ptr == argv + i) {
                if (get_complete() && complete_hook) {
                    auto result = complete_hook(argv[i]);
                    if (result.size() == 1) {
                        if (result[0] != option) {
                            std::cout << result[0] << std::endl;
                        }
                    } else {
                        for (const auto & line : result) {
                            std::cout << line << std::endl;
                        }
                    }
                }
                return static_cast<int>(count);
            }
        }
    }
    if (argc < nvals) {
        throw ArgumentParserPositionalArgFewValuesError(M_("Too few values for positional argument \"{}\""), this->id);
    }
    for (int i = 1; i < nvals; ++i) {
        if (*argv[i] == '-') {
            throw ArgumentParserPositionalArgFewValuesError(
                M_("Too few values for positional argument \"{}\""), this->id);
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
        parse_hook(this, static_cast<int>(count), argv);
    }
    return static_cast<int>(count);
}

int ArgumentParser::NamedArg::parse_long(const char * option, int argc, const char * const argv[]) {
    if (const auto * arg = get_conflict_argument()) {
        auto [fmt_message, conflict_option] = get_conflict_arg_msg(arg);
        throw ArgumentParserConflictingArgumentsError(fmt_message, "--" + std::string(option), conflict_option);
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
                throw ArgumentParserNamedArgMissingValueError(
                    M_("Missing value for named argument \"--{}\""), std::string(option));
            }
            arg_value = argv[1];
            consumed_args = 2;
        }
    } else {
        if (assign_ptr) {
            throw ArgumentParserNamedArgValueNotExpectedError(
                M_("Unexpected value for named argumend \"--{}\""), std::string(option));
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
        auto [fmt_message, conflict_option] = get_conflict_arg_msg(arg);
        throw ArgumentParserConflictingArgumentsError(fmt_message, '-' + std::string(option), conflict_option);
    }
    const char * arg_value;
    int consumed_args;
    if (has_value) {
        if (option[1] != '\0') {
            arg_value = option + 1;
            consumed_args = 1;
        } else {
            if (argc < 2) {
                throw ArgumentParserNamedArgMissingValueError(M_("Missing value for named argument \"-{}\""), *option);
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

void ArgumentParser::Command::register_command(Command * cmd) {
    for (auto * item : cmds) {
        if (item->id == cmd->id) {
            throw ArgumentParserIdAlreadyRegisteredError(
                M_("Command id \"{}\" already registered for command \"{}\""), cmd->id, id);
        }
    }
    cmd->parent = this;
    cmds.push_back(cmd);
}

void ArgumentParser::Command::register_named_arg(NamedArg * arg) {
    for (auto * item : named_args) {
        if (item->id == arg->id) {
            throw ArgumentParserIdAlreadyRegisteredError(
                M_("Named argument id \"{}\" already registered for command \"{}\""), arg->id, id);
        }
    }
    named_args.push_back(arg);
}

void ArgumentParser::Command::register_positional_arg(PositionalArg * arg) {
    for (auto * item : pos_args) {
        if (item->id == arg->id) {
            throw ArgumentParserIdAlreadyRegisteredError(
                M_("Positional argument id \"{}\" already registered for command \"{}\""), arg->id, id);
        }
    }
    pos_args.push_back(arg);
}

void ArgumentParser::Command::register_group(Group * grp) {
    for (auto * item : groups) {
        if (item->id == grp->id) {
            throw ArgumentParserIdAlreadyRegisteredError(
                M_("Group id \"{}\" already registered for command \"{}\""), grp->id, id);
        }
    }
    groups.push_back(grp);
}

template <class Arg>
static Arg * find_arg(const std::vector<Arg *> & args, const std::string & id) {
    for (auto * item : args) {
        if (item->get_id() == id) {
            return item;
        }
    }
    return nullptr;
}

ArgumentParser::Command & ArgumentParser::Command::get_command(const std::string & id) const {
    if (auto ret = find_arg(cmds, id)) {
        return *ret;
    }
    throw ArgumentParserNotFoundError(M_("Command id \"{}\" does not contain subcommand with id \"{}\""), this->id, id);
}

ArgumentParser::NamedArg & ArgumentParser::Command::get_named_arg(const std::string & id) const {
    if (auto ret = find_arg(named_args, id)) {
        return *ret;
    }
    throw ArgumentParserNotFoundError(
        M_("Command id \"{}\" does not contain named argument with id \"{}\""), this->id, id);
}

ArgumentParser::PositionalArg & ArgumentParser::Command::get_positional_arg(const std::string & id) const {
    if (auto ret = find_arg(pos_args, id)) {
        return *ret;
    }
    throw ArgumentParserNotFoundError(
        M_("Command id \"{}\" does not contain positional argument with id \"{}\""), this->id, id);
}

void ArgumentParser::Command::parse(const char * option, int argc, const char * const argv[]) {
    if (owner.inherit_named_args) {
        const std::vector<NamedArg *> additional_named_args;
        parse(option, argc, argv, &additional_named_args);
    } else {
        parse(option, argc, argv, nullptr);
    }
}

// Prints a completed argument `arg` or a table with suggestions and help to complete
// if there is more than one solution.
void ArgumentParser::Command::print_complete(
    const char * arg, std::vector<ArgumentParser::NamedArg *> named_args, size_t used_positional_arguments) {
    // Using the Help class to print the completion suggestions, as it prints a table of two columns
    // which is also what we need here.
    libdnf::cli::output::Help help;
    std::string last;

    // Search for matching commands.
    if (arg[0] == '\0' || arg[0] != '-') {
        for (const auto * opt : get_commands()) {
            if (!opt->get_complete()) {
                continue;
            }
            auto & name = opt->get_id();
            if (name.compare(0, strlen(arg), arg) == 0) {
                help.add_line(name, '(' + opt->get_short_description() + ')', nullptr);
                last = name + ' ';
            }
        }
        if (last.empty() && used_positional_arguments < get_positional_args().size()) {
            auto pos_arg = get_positional_args()[used_positional_arguments];
            if (pos_arg->get_complete() && pos_arg->complete_hook) {
                auto result = pos_arg->complete_hook(arg);
                if (result.size() == 1) {
                    if (result[0] == arg) {
                        return;
                    }
                    std::cout << result[0] << std::endl;
                    return;
                }
                for (const auto & line : result) {
                    std::cout << line << std::endl;
                }
            }
        }
    }

    // Search for matching named arguments.
    if (arg[0] == '-') {
        for (const auto * opt : named_args) {
            if (!opt->get_complete()) {
                continue;
            }
            if ((arg[1] == '\0' && opt->get_short_name() != '\0') ||
                (arg[1] == opt->get_short_name() && arg[2] == '\0')) {
                std::string name = std::string("-") + opt->get_short_name();
                std::string extended_name = name;
                if (opt->get_has_value()) {
                    extended_name += opt->get_arg_value_help().empty() ? "VALUE" : opt->get_arg_value_help();
                }
                help.add_line(extended_name, '(' + opt->get_short_description() + ')', nullptr);
                last = name;
                if (!opt->get_has_value()) {
                    last += ' ';
                }
            }
            if (!opt->get_long_name().empty()) {
                std::string name = "--" + opt->get_long_name();
                std::string extended_name = name;
                if (opt->get_has_value()) {
                    name += '=';
                    extended_name += '=' + (opt->get_arg_value_help().empty() ? "VALUE" : opt->get_arg_value_help());
                }
                if (name.compare(0, strlen(arg), arg) == 0) {
                    help.add_line(extended_name, '(' + opt->get_short_description() + ')', nullptr);
                    last = name;
                    if (!opt->get_has_value()) {
                        last += ' ';
                    }
                }
            }
        }
    }

    // Prints a completed argument or a table with suggestions and help to complete if there is more than one solution.
    if (scols_table_get_nlines(help.get_table()) > 1) {
        help.print();
    } else if (!last.empty() && last != arg) {
        std::cout << last << std::endl;
    }
}

void ArgumentParser::Command::parse(
    const char * option, int argc, const char * const argv[], const std::vector<NamedArg *> * additional_named_args) {
    std::vector<NamedArg *> extended_named_args;
    if (additional_named_args) {
        extended_named_args.reserve(named_args.size() + additional_named_args->size());
        extended_named_args.insert(extended_named_args.begin(), named_args.begin(), named_args.end());
        extended_named_args.insert(
            extended_named_args.end(), additional_named_args->begin(), additional_named_args->end());
    }
    size_t used_positional_arguments = 0;
    int short_option_idx = 0;
    for (int i = 1; i < argc;) {
        if (owner.complete_arg_ptr) {
            if (argv + i > owner.complete_arg_ptr) {
                return;
            } else if (argv + i == owner.complete_arg_ptr) {
                print_complete(
                    argv[i], additional_named_args ? extended_named_args : named_args, used_positional_arguments);
                return;
            }
        }
        bool used = false;
        const auto * tmp = argv[i];
        if (*tmp == '-') {
            bool long_option = *++tmp == '-';
            if (long_option) {
                ++tmp;
            }
            const auto * assign_ptr = strchr(tmp, '=');
            for (auto * opt : (additional_named_args ? extended_named_args : named_args)) {
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
                if (cmd->id == argv[i]) {
                    if (const auto * arg = get_conflict_argument()) {
                        auto [fmt_message, conflict_option] = get_conflict_arg_msg(arg);
                        throw ArgumentParserConflictingArgumentsError(
                            fmt_message, std::string(option), conflict_option);
                    }
                    // the last subcommand wins
                    owner.selected_command = cmd;
                    cmd->parse(argv[i], argc - i, &argv[i], additional_named_args ? &extended_named_args : nullptr);
                    i = argc;
                    used = true;
                    // The subcommand processed the completion of the argument. There is no need to continue parsing.
                    if (owner.complete_arg_ptr) {
                        return;
                    }
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
            throw ArgumentParserUnknownArgumentError(M_("Unknown argument \"{}\" for command \"{}\""), argv[i], id);
        }
    }
    ++parse_count;

    // Test that all required positional arguments are present.
    for (const auto * pos_arg : pos_args) {
        const auto nvals = pos_arg->get_nvals();
        if (pos_arg->get_parse_count() == 0 && nvals != PositionalArg::UNLIMITED && nvals != PositionalArg::OPTIONAL) {
            throw ArgumentParserMissingPositionalArgumentError(
                M_("Missing positional argument \"{}\" for command \"{}\""), pos_arg->get_id(), id);
        }
    }

    if (parse_hook) {
        parse_hook(this, option, argc, argv);
    }
}

static std::string get_named_arg_names(const ArgumentParser::NamedArg * arg) {
    std::string arg_names;
    if (arg->get_short_name() != '\0') {
        arg_names = std::string("-") + arg->get_short_name();
        if (arg->get_has_value()) {
            arg_names += arg->get_arg_value_help().empty() ? " VALUE" : ' ' + arg->get_arg_value_help();
        }
        if (!arg->get_long_name().empty()) {
            arg_names += ", ";
        }
    }
    if (!arg->get_long_name().empty()) {
        arg_names += "--" + arg->get_long_name();
        if (arg->get_has_value()) {
            arg_names += arg->get_arg_value_help().empty() ? "=VALUE" : '=' + arg->get_arg_value_help();
        }
    }
    return arg_names;
}

void ArgumentParser::Command::help() const noexcept {
    libdnf::cli::output::Usage usage_output;

    // generate usage
    // start with the current command name
    std::string usage = get_id();

    if (cmds.empty()) {
        // leaf command
        usage += " [GLOBAL OPTIONS] [OPTIONS] [ARGUMENTS]";
    } else {
        // command that has subcommands
        usage += " <COMMAND> [--help] ...";
    }

    // prepend parent commands to usage
    Command * cmd = parent;
    while (cmd) {
        usage = cmd->get_id() + " " + usage;
        cmd = cmd->parent;
    }

    // print usage
    auto * usage_header = usage_output.add_header(_("Usage:"));
    for (auto & line : libdnf::utils::string::split(usage, "\n")) {
        usage_output.add_line(line, usage_header);
    }

    // print description
    if (!description.empty()) {
        usage_output.add_newline();
        auto * desc_header = usage_output.add_header(_("Description:"));
        for (auto & line : libdnf::utils::string::split(description, "\n")) {
            usage_output.add_line(line, desc_header);
        }
    }

    libdnf::cli::output::Help help;

    // Arguments used in groups are not printed as ungrouped.
    std::set<Argument *> args_used_in_groups;

    if (!commands_help_header.empty() && !cmds.empty()) {
        const std::set<Argument *> cmds_set(cmds.begin(), cmds.end());

        // Processing commands in groups.
        for (auto * grp : groups) {
            // we'll initialize the header line later when the first line with an argument is added
            struct libscols_line * header{nullptr};
            for (auto * arg : grp->get_arguments()) {
                if (dynamic_cast<Command *>(arg) && cmds_set.count(arg) > 0) {
                    if (!header) {
                        help.add_newline();
                        header = help.add_header(grp->get_header());
                    }
                    help.add_line(arg->get_id(), arg->get_short_description(), header);
                    args_used_in_groups.insert(arg);
                }
            }
        }

        // gather commands that don't belong to any group
        std::vector<Command *> ungrouped_commands;
        for (auto * cmd : cmds) {
            if (args_used_in_groups.count(cmd) == 0) {
                ungrouped_commands.push_back(cmd);
            }
        }

        // print the commands that don't belong to any group
        // avoid printing `commands_help_header` if the list is empty
        if (!ungrouped_commands.empty()) {
            help.add_newline();
            struct libscols_line * header = help.add_header(commands_help_header);
            for (auto * arg : ungrouped_commands) {
                help.add_line(arg->get_id(), arg->get_short_description(), header);
            }
        }
    }

    if (!named_args_help_header.empty() && !named_args.empty()) {
        const std::set<Argument *> named_args_set(named_args.begin(), named_args.end());

        // Processing named arguments in groups.
        for (auto * grp : groups) {
            // we'll initialize the header line later when the first line with an argument is added
            struct libscols_line * header{nullptr};
            for (auto * arg : grp->get_arguments()) {
                auto * named_arg = dynamic_cast<NamedArg *>(arg);
                if (named_arg && named_args_set.count(arg) > 0) {
                    if (!header) {
                        help.add_newline();
                        header = help.add_header(grp->get_header());
                    }
                    help.add_line(get_named_arg_names(named_arg), arg->get_short_description(), header);
                    args_used_in_groups.insert(arg);
                }
            }
        }

        // gather named args that don't belong to any group
        std::vector<NamedArg *> ungrouped_named_args;
        for (auto * arg : named_args) {
            if (args_used_in_groups.count(static_cast<NamedArg *>(arg)) == 0) {
                ungrouped_named_args.push_back(arg);
            }
        }

        // print the named args that don't belong to any group
        // avoid printing `named_args_help_header` if the list is empty
        if (!ungrouped_named_args.empty()) {
            help.add_newline();
            struct libscols_line * header = help.add_header(named_args_help_header);
            for (const auto * arg : ungrouped_named_args) {
                help.add_line(get_named_arg_names(arg), arg->get_short_description(), header);
            }
        }
    }

    if (!positional_args_help_header.empty() && !pos_args.empty()) {
        const std::set<Argument *> pos_args_set(pos_args.begin(), pos_args.end());

        // Processing positional arguments in groups.
        for (auto * grp : groups) {
            // we'll initialize the header line later when the first line with an argument is added
            struct libscols_line * header{nullptr};
            for (auto * arg : grp->get_arguments()) {
                if (dynamic_cast<PositionalArg *>(arg) && pos_args_set.count(arg) > 0) {
                    if (!header) {
                        help.add_newline();
                        header = help.add_header(grp->get_header());
                    }
                    help.add_line(arg->get_id(), arg->get_short_description(), header);
                    args_used_in_groups.insert(arg);
                }
            }
        }

        // Processing ungrouped positional arguments.
        help.add_newline();
        struct libscols_line * header = help.add_header(positional_args_help_header);
        for (const auto * arg : pos_args) {
            help.add_line(arg->get_id(), arg->get_short_description(), header);
        }
    }

    usage_output.print();
    help.print();
}

ArgumentParser::Command * ArgumentParser::add_new_command(const std::string & id) {
    std::unique_ptr<Command> arg(new Command(*this, id));
    auto * ptr = arg.get();
    cmds.push_back(std::move(arg));
    return ptr;
}

ArgumentParser::NamedArg * ArgumentParser::add_new_named_arg(const std::string & id) {
    std::unique_ptr<NamedArg> arg(new NamedArg(*this, id));
    auto * ptr = arg.get();
    named_args.push_back(std::move(arg));
    return ptr;
}

ArgumentParser::PositionalArg * ArgumentParser::add_new_positional_arg(
    const std::string & id, std::vector<std::unique_ptr<libdnf::Option>> * values) {
    std::unique_ptr<PositionalArg> arg(new PositionalArg(*this, id, values));
    auto * ptr = arg.get();
    pos_args.push_back(std::move(arg));
    return ptr;
}

ArgumentParser::PositionalArg * ArgumentParser::add_new_positional_arg(
    const std::string & id,
    int nargs,
    libdnf::Option * init_value,
    std::vector<std::unique_ptr<libdnf::Option>> * values) {
    std::unique_ptr<PositionalArg> arg(new PositionalArg(*this, id, nargs, init_value, values));
    auto * ptr = arg.get();
    pos_args.push_back(std::move(arg));
    return ptr;
}

ArgumentParser::Group * ArgumentParser::add_new_group(const std::string & id) {
    std::unique_ptr<Group> group(new Group(id));
    auto * ptr = group.get();
    groups.push_back(std::move(group));
    return ptr;
}

std::vector<ArgumentParser::Argument *> * ArgumentParser::add_conflict_args_group(
    std::unique_ptr<std::vector<Argument *>> && conflict_args_group) {
    auto * ptr = conflict_args_group.get();
    conflict_args_groups.push_back(std::move(conflict_args_group));
    return ptr;
}

libdnf::Option * ArgumentParser::add_init_value(std::unique_ptr<libdnf::Option> && src) {
    auto * ptr = src.get();
    values_init.push_back(std::move(src));
    return ptr;
}

std::vector<std::unique_ptr<libdnf::Option>> * ArgumentParser::add_new_values() {
    std::unique_ptr<std::vector<std::unique_ptr<libdnf::Option>>> tmp(new std::vector<std::unique_ptr<libdnf::Option>>);
    auto * ptr = tmp.get();
    values.push_back(std::move(tmp));
    return ptr;
}

std::vector<std::unique_ptr<libdnf::Option>> * ArgumentParser::add_values(
    std::unique_ptr<std::vector<std::unique_ptr<libdnf::Option>>> && values) {
    auto * ptr = values.get();
    this->values.push_back(std::move(values));
    return ptr;
}

void ArgumentParser::parse(int argc, const char * const argv[]) {
    assert_root_command();

    // mark root command as selected; overwrite with a subcommand in Command::parse()
    selected_command = root_command;
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

ArgumentParser::Command & ArgumentParser::get_command(const std::string & id_path) {
    assert_root_command();

    auto * cmd = root_command;
    if (id_path.empty()) {
        return *cmd;
    }
    std::string::size_type start_pos = 0;
    auto dot_pos = id_path.find('.');
    while (dot_pos != id_path.npos) {
        cmd = &cmd->get_command(std::string(id_path, start_pos, dot_pos - start_pos));
        start_pos = dot_pos + 1;
        dot_pos = id_path.find('.', start_pos);
    }
    cmd = &cmd->get_command(std::string(id_path, start_pos));
    return *cmd;
}

template <class Arg>
static const std::vector<Arg *> & get_command_args(ArgumentParser::Command & command) {
    if constexpr (std::is_same<Arg, ArgumentParser::NamedArg>::value) {
        return command.get_named_args();
    }
    if constexpr (std::is_same<Arg, ArgumentParser::PositionalArg>::value) {
        return command.get_positional_args();
    }
}

template <class Arg>
static Arg * get_arg(ArgumentParser::Command * root_command, const std::string & id_path, bool search_in_parent) {
    auto * cmd = root_command;
    std::string::size_type start_pos = 0;
    auto dot_pos = id_path.find('.');
    auto arg_id_name = dot_pos == id_path.npos ? id_path : std::string(id_path, id_path.rfind('.') + 1);
    Arg * ret = nullptr;
    while (dot_pos != id_path.npos) {
        if (search_in_parent) {
            if (auto tmp = find_arg(get_command_args<Arg>(*cmd), arg_id_name)) {
                ret = tmp;
            }
        }
        cmd = &cmd->get_command(std::string(id_path, start_pos, dot_pos - start_pos));
        start_pos = dot_pos + 1;
        dot_pos = id_path.find('.', start_pos);
    }
    if (auto tmp = find_arg(get_command_args<Arg>(*cmd), arg_id_name)) {
        ret = tmp;
    }
    return ret;
}

ArgumentParser::NamedArg & ArgumentParser::get_named_arg(const std::string & id_path, bool search_in_parent) {
    assert_root_command();

    if (auto ret = get_arg<ArgumentParser::NamedArg>(root_command, id_path, search_in_parent)) {
        return *ret;
    }
    throw ArgumentParserNotFoundError(M_("Named argument with path id \"{}\" not found"), id_path);
}

libdnf::cli::ArgumentParser::NamedArg * ArgumentParser::NamedArg::add_alias(
    const std::string & id,
    const std::string & long_name,
    char short_name,
    libdnf::cli::ArgumentParser::Group * group) {
    auto * alias = get_argument_parser().add_new_named_arg(id);
    alias->set_long_name(long_name);
    alias->set_short_name(short_name);

    // Set description
    std::string descr;
    if (get_short_name() != '\0') {
        descr = std::string("'-") + get_short_name() + "'";
        if (!get_long_name().empty()) {
            descr += ", ";
        }
    }
    if (!get_long_name().empty()) {
        descr += "'--" + get_long_name() + "'";
    }
    alias->set_short_description(fmt::format("Alias for {}", descr));

    // Copy from source argument
    alias->set_has_value(get_has_value());
    alias->link_value(get_linked_value());
    alias->set_store_value(get_store_value());
    alias->set_const_value(get_const_value());
    alias->set_arg_value_help(get_arg_value_help());
    alias->set_parse_hook_func(libdnf::cli::ArgumentParser::NamedArg::ParseHookFunc(get_parse_hook_func()));

    // Do not offer aliases in completion
    alias->set_complete(false);

    if (group) {
        group->register_argument(alias);
    }

    if (conflict_args) {
        // Create a new conflict group for the alias
        auto tmp_conflict_args = owner.add_conflict_args_group(std::make_unique<std::vector<Argument *>>());
        alias->set_conflict_arguments(tmp_conflict_args);

        for (std::size_t idx = 0; idx < conflict_args->size(); ++idx) {
            auto * conflict_arg = (*conflict_args)[idx];

            // Do not add the alias as a conflict for the aliased argument
            if (conflict_arg == this) {
                continue;
            }

            tmp_conflict_args->push_back(conflict_arg);

            // Set reverse conflicts to alias
            auto * conflict_conflict_args = conflict_arg->get_conflict_arguments();
            if (conflict_conflict_args) {
                auto it = std::find(conflict_conflict_args->begin(), conflict_conflict_args->end(), alias);
                if (it == conflict_conflict_args->end()) {
                    conflict_conflict_args->push_back(alias);
                }
            } else {
                conflict_conflict_args = get_argument_parser().add_conflict_args_group(
                    std::unique_ptr<std::vector<ArgumentParser::Argument *>>(
                        new std::vector<ArgumentParser::Argument *>{alias}));
                conflict_arg->set_conflict_arguments(conflict_conflict_args);
            }
        }
    }

    return alias;
}

ArgumentParser::PositionalArg & ArgumentParser::get_positional_arg(const std::string & id_path, bool search_in_parent) {
    assert_root_command();

    if (auto ret = get_arg<ArgumentParser::PositionalArg>(root_command, id_path, search_in_parent)) {
        return *ret;
    }
    throw ArgumentParserNotFoundError(M_("Positional argument with path id \"{}\" not found"), id_path);
}

void ArgumentParser::complete(int argc, const char * const argv[], int complete_arg_idx) {
    if (complete_arg_idx < 1 || complete_arg_idx >= argc) {
        return;
    }
    complete_arg_ptr = argv + complete_arg_idx;
    try {
        parse(argc, argv);
    } catch (...) {
    }
}


void ArgumentParser::assert_root_command() {
    libdnf_assert(root_command != nullptr, "Root command is not set");
}

}  // namespace libdnf::cli
