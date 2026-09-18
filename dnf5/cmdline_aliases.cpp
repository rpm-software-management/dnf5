// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.

#include "cmdline_aliases.hpp"

#include "utils/string.hpp"

#include <libdnf5/common/preserve_order_map.hpp>
#include <libdnf5/utils/bgettext/bgettext-lib.h>
#include <libdnf5/utils/bgettext/bgettext-mark-domain.h>
#include <libdnf5/utils/format.hpp>
#include <libdnf5/utils/format_locale.hpp>
#include <toml.hpp>

#include <algorithm>
#include <array>
#include <iostream>
#include <limits>
#include <optional>
#include <set>
#include <type_traits>
#include <vector>

namespace fs = std::filesystem;

namespace dnf5 {

namespace {

constexpr std::array<const char *, 3> CONF_FILE_SUPPORTED_VERSIONS = {"1.0", "1.1", "1.2"};

using ArgParser = libdnf5::cli::ArgumentParser;


#ifdef TOML11_COMPAT

using BasicValue = toml::basic_value<toml::discard_comments, libdnf5::PreserveOrderMap, std::vector>;

inline auto location_first_line_num(const toml::source_location & location) {
    return location.line();
}

inline std::string location_lines(const toml::source_location & location) {
    return location.line_str();
}

#else  // #ifdef TOML11_COMPAT

using BasicValue = toml::ordered_value;

inline auto location_first_line_num(const toml::source_location & location) {
    return location.first_line_number();
}

inline std::string location_lines(const toml::source_location & location) {
    return libdnf5::utils::string::join(location.lines(), "\n");
}

#endif  // #ifdef TOML11_COMPAT


// If the `arg` is of type `std::array<const char *, N>`, it joins the elements with a `delimiter` string;
// otherwise, it returns the original value.
template <typename T>
struct is_const_char_ptr_array : std::false_type {};
template <std::size_t N>
struct is_const_char_ptr_array<std::array<const char *, N>> : std::true_type {};

template <typename T>
auto join_if_array(const T & arg, const char * delimiter) {
    if constexpr (is_const_char_ptr_array<std::decay_t<T>>::value) {
        return libdnf5::utils::string::join(arg, delimiter);
    } else {
        return arg;
    }
}

constexpr BgettextMessage ARRAY_ITEMS_DELIMITER = M_(", ");

template <typename... Args>
void print_and_log(
    libdnf5::Logger & logger, libdnf5::Logger::Level level, BgettextMessage fmt_string, const Args &... args) {
    static const auto array_delim = b_gettextmsg_get_id(ARRAY_ITEMS_DELIMITER);
    static const auto array_delim_localized = b_dmgettext(NULL, ARRAY_ITEMS_DELIMITER, 1);

    logger.log(level, b_gettextmsg_get_id(fmt_string), join_if_array(args, array_delim)...);
    std::cerr << libdnf5::utils::format(true, fmt_string, 1, join_if_array(args, array_delim_localized)...)
              << std::endl;
}

template <typename... Args>
void print_and_log_warning(libdnf5::Logger & logger, BgettextMessage fmt_string, const Args &... args) {
    print_and_log(logger, libdnf5::Logger::Level::WARNING, fmt_string, args...);
}

template <typename... Args>
void print_and_log_error(libdnf5::Logger & logger, BgettextMessage fmt_string, const Args &... args) {
    print_and_log(logger, libdnf5::Logger::Level::ERROR, fmt_string, args...);
}


std::optional<std::string> get_string_locale(const BasicValue & value, const std::string & locale_name) {
    if (value.is_string()) {
        return value.as_string();
    }

    const auto & table = value.as_table();
    if (auto it = table.find(locale_name); it != table.end()) {
        return it->second.as_string();
    }
    if (auto it = table.find("C"); it != table.end()) {
        return it->second.as_string();
    }

    return {};
}


// Attach additional named arguments to the alias
template <typename ArgT>
bool attach_named_args(
    libdnf5::Logger & logger,
    const fs::path & path,
    ArgT & alias_arg,
    const BasicValue::array_type & attached_named_args,
    const std::string & alias_id_path) {
    for (auto & attached_arg : attached_named_args) {
        std::optional<std::string> attached_arg_id_path;
        std::optional<std::string> arg_value;
        for (auto & [key, value] : attached_arg.as_table()) {
            if (key == "id_path") {
                attached_arg_id_path = value.as_string();
                try {
                    alias_arg.get_argument_parser().get_named_arg(*attached_arg_id_path, false);
                } catch (const libdnf5::cli::ArgumentParserNotFoundError & e) {
                    const auto location = value.location();
                    print_and_log_error(
                        logger,
                        M_("Attached named argument \"{}\" not found: {}: Requested in file \"{}\" on line {}: {}"),
                        *attached_arg_id_path,
                        e.what(),
                        path.native(),
                        location_first_line_num(location),
                        location_lines(location));
                    return false;
                }
            } else if (key == "value") {
                arg_value = value.as_string();
            } else {
                const auto location = value.location();
                print_and_log_warning(
                    logger,
                    M_("Unknown attribute \"{}\" of attached named argument for alias \"{}\" "
                       "in file \"{}\" on line {}: {}"),
                    key,
                    alias_id_path,
                    path.native(),
                    location_first_line_num(location),
                    location_lines(location));
            }
        }
        if (!attached_arg_id_path) {
            print_and_log_error(
                logger,
                M_("Missing attribute \"id_path\" for alias \"{}\" in file \"{}\""),
                alias_id_path,
                path.native());
            return false;
        }

        alias_arg.attach_named_arg(*attached_arg_id_path, arg_value ? *arg_value : "");
    }
    return true;
}

void load_aliases_from_toml_file(
    Context & context,
    const fs::path & config_file_path,
    const std::string & locale_name,
    CommandFlagAliasTable & command_flag_aliases) {
    auto & arg_parser = context.get_argument_parser();
    auto logger = context.get_base().get_logger();

    try {
#ifdef TOML11_COMPAT
        const auto arg_parser_elements =
            toml::parse<::toml::discard_comments, libdnf5::PreserveOrderMap, std::vector>(config_file_path);
#else
        const toml::ordered_value arg_parser_elements = toml::parse<toml::ordered_type_config>(config_file_path);
#endif  // #ifdef TOML11_COMPAT

        std::string version;
        try {
            version = toml::find<std::string>(arg_parser_elements, "version");
            bool supported{false};
            for (auto * supported_version : CONF_FILE_SUPPORTED_VERSIONS) {
                if (version == supported_version) {
                    supported = true;
                    break;
                }
            }
            if (!supported) {
                print_and_log_error(
                    *logger,
                    M_("Unsupported version \"{}\" in file \"{}\". Supported versions: {}"),
                    version,
                    config_file_path.native(),
                    CONF_FILE_SUPPORTED_VERSIONS);
                return;
            }
        } catch (const toml::type_error & e) {
            auto loc = e.location();
            print_and_log_error(
                *logger,
                M_("Bad value type of attribute \"version\" in file \"{}\" on line {}: {}"),
                config_file_path.native(),
                location_first_line_num(loc),
                location_lines(loc));
            return;
        } catch (const std::out_of_range & e) {
            print_and_log_error(*logger, M_("Missing attribute \"version\" in file \"{}\""), config_file_path.native());
            return;
        }

        for (const auto & [element_id_path, element_options] : arg_parser_elements.as_table()) {
            if (!element_options.is_table()) {
                if (element_id_path == "version") {
                    continue;
                }
                const auto location = element_options.location();
                print_and_log_warning(
                    *logger,
                    M_("Unknown key \"{}\" in file \"{}\" on line {}: {}"),
                    element_id_path,
                    config_file_path.native(),
                    location_first_line_num(location),
                    location_lines(location));
                continue;
            }
            auto element_id_pos = element_id_path.rfind('.');
            if (element_id_pos != std::string::npos) {
                ++element_id_pos;
            } else {
                element_id_pos = 0;
            }
            if (element_id_pos == element_id_path.size()) {
                const auto location = element_options.location();
                print_and_log_error(
                    *logger,
                    M_("Empty or bad element id path in file \"{}\" on line {}: {}"),
                    config_file_path.native(),
                    location_first_line_num(location),
                    location_lines(location));
                continue;
            }

            // Split element_id_path into parent id_path and element id
            const std::string element_id = element_id_path.substr(element_id_pos);
            const std::string element_parent_id_path =
                element_id_pos == 0 ? "" : element_id_path.substr(0, element_id_pos - 1);

            // dnf4 compatibility: 'command_flag' entries are handled before
            // parent-command resolution - the parent may be a command alias
            // (like updateinfo), and the table only needs the names.
            {
                bool is_command_flag = false;
                std::optional<toml::source_location> type_location;
                try {
#ifdef TOML11_COMPAT
                    const auto el_type = toml::find(element_options, "type");
#else
                    const auto el_type = toml::find<toml::ordered_value>(element_options, "type");
#endif  // #ifdef TOML11_COMPAT
                    is_command_flag = (std::string(el_type.as_string()) == "command_flag");
                    type_location = el_type.location();
                } catch (const std::out_of_range &) {
                } catch (const toml::type_error &) {
                }
                if (is_command_flag) {
                    if (version == "1.0" || version == "1.1") {
                        print_and_log_error(
                            *logger,
                            M_("Used config file version \"{}\" for alias type \"command_flag\" of element \"{}\" "
                               "in file \"{}\" on line {}: {}"),
                            version,
                            element_id_path,
                            config_file_path.native(),
                            location_first_line_num(*type_location),
                            location_lines(*type_location));
                        continue;
                    }
                    if (element_parent_id_path.empty() || element_parent_id_path.find('.') != std::string::npos) {
                        print_and_log_warning(
                            *logger,
                            M_("Command flag alias \"{}\" must be named \"<command>.<flag>\" in file \"{}\" "
                               "on line {}: {}"),
                            element_id_path,
                            config_file_path.native(),
                            location_first_line_num(*type_location),
                            location_lines(*type_location));
                        continue;
                    }
                    const auto alias_key =
                        std::pair<std::string, std::string>{element_parent_id_path, "--" + element_id};
                    if (command_flag_aliases.count(alias_key) > 0) {
                        print_and_log_error(
                            *logger,
                            M_("Command flag alias \"{}\" already registered. Requested in file \"{}\" on line {}: {}"),
                            element_id_path,
                            config_file_path.native(),
                            location_first_line_num(*type_location),
                            location_lines(*type_location));
                        continue;
                    }
                    CommandFlagAlias alias;
                    bool defined = false;
                    bool broken = false;
                    try {
                        for (auto & [key, value] : element_options.as_table()) {
                            if (key == "type") {
                                continue;
                            } else if (key == "reject_message") {
                                alias.reject_message = value.as_string();
                                if (alias.reject_message.empty()) {
                                    const auto location = value.location();
                                    print_and_log_error(
                                        *logger,
                                        M_("Bad value \"\" of attribute \"reject_message\" for element \"{}\" "
                                           "in file \"{}\" on line {}: {}"),
                                        element_id_path,
                                        config_file_path.native(),
                                        location_first_line_num(location),
                                        location_lines(location));
                                    broken = true;
                                } else {
                                    defined = true;
                                }
                            } else if (key == "attached_command") {
                                const std::string attached{value.as_string()};
                                alias.words = libdnf5::utils::string::split(attached, ".");
                                bool bad_path = alias.words.empty();
                                for (const auto & word : alias.words) {
                                    if (word.empty()) {
                                        bad_path = true;
                                    }
                                }
                                if (bad_path) {
                                    const auto location = value.location();
                                    print_and_log_error(
                                        *logger,
                                        M_("Bad value \"{}\" of attribute \"attached_command\" for element \"{}\" "
                                           "in file \"{}\" on line {}: {}"),
                                        attached,
                                        element_id_path,
                                        config_file_path.native(),
                                        location_first_line_num(location),
                                        location_lines(location));
                                    broken = true;
                                } else {
                                    defined = true;
                                }
                            } else if (key == "attached_flags") {
                                for (const auto & flag : value.as_array()) {
                                    alias.attached_flags.emplace_back(flag.as_string());
                                    if (alias.attached_flags.back().empty()) {
                                        const auto location = value.location();
                                        print_and_log_error(
                                            *logger,
                                            M_("Bad value \"\" of attribute \"attached_flags\" for element \"{}\" "
                                               "in file \"{}\" on line {}: {}"),
                                            element_id_path,
                                            config_file_path.native(),
                                            location_first_line_num(location),
                                            location_lines(location));
                                        broken = true;
                                        break;
                                    }
                                }
                            } else if (key == "positional_template") {
                                alias.positional_template = value.as_string();
                            } else if (key == "token_prefix_maps") {
                                for (const auto & map_entry : value.as_array()) {
                                    std::optional<std::string> prefix;
                                    std::optional<std::string> map_template;
                                    for (auto & [map_key, map_value] : map_entry.as_table()) {
                                        if (map_key == "prefix") {
                                            prefix = std::string(map_value.as_string());
                                        } else if (map_key == "template") {
                                            map_template = std::string(map_value.as_string());
                                        } else {
                                            const auto location = map_value.location();
                                            print_and_log_warning(
                                                *logger,
                                                M_("Unknown attribute \"{}\" of \"token_prefix_maps\" for element "
                                                   "\"{}\" in file \"{}\" on line {}: {}"),
                                                map_key,
                                                element_id_path,
                                                config_file_path.native(),
                                                location_first_line_num(location),
                                                location_lines(location));
                                        }
                                    }
                                    if (prefix && prefix->empty()) {
                                        const auto location = value.location();
                                        print_and_log_error(
                                            *logger,
                                            M_("Bad value \"\" of attribute \"prefix\" in \"token_prefix_maps\" "
                                               "for element \"{}\" in file \"{}\" on line {}: {}"),
                                            element_id_path,
                                            config_file_path.native(),
                                            location_first_line_num(location),
                                            location_lines(location));
                                        broken = true;
                                        break;
                                    }
                                    if (!prefix || !map_template) {
                                        const auto location = value.location();
                                        print_and_log_error(
                                            *logger,
                                            M_("Missing attribute \"prefix\" or \"template\" in \"token_prefix_maps\" "
                                               "for element \"{}\" in file \"{}\" on line {}: {}"),
                                            element_id_path,
                                            config_file_path.native(),
                                            location_first_line_num(location),
                                            location_lines(location));
                                        broken = true;
                                        break;
                                    }
                                    alias.token_prefix_maps.emplace_back(std::move(*prefix), std::move(*map_template));
                                }
                            } else if (key == "companion_flag") {
                                alias.companion_flag = value.as_string();
                            } else if (key == "precedence") {
                                const auto precedence = value.as_integer();
                                if (precedence < std::numeric_limits<int>::min() ||
                                    precedence > std::numeric_limits<int>::max()) {
                                    const auto location = value.location();
                                    print_and_log_error(
                                        *logger,
                                        M_("Bad value \"{}\" of attribute \"precedence\" for element \"{}\" "
                                           "in file \"{}\" on line {}: {}"),
                                        std::to_string(precedence),
                                        element_id_path,
                                        config_file_path.native(),
                                        location_first_line_num(location),
                                        location_lines(location));
                                    broken = true;
                                } else {
                                    alias.precedence = static_cast<int>(precedence);
                                }
                            } else if (key == "drop_bare_positionals") {
                                alias.drop_bare_positionals = value.as_boolean();
                            } else if (key == "dropped_note") {
                                alias.dropped_note = value.as_string();
                            } else if (key == "leading_positionals") {
                                alias.leading_positionals = value.as_boolean();
                            } else if (key == "positional_suffix_gate") {
                                alias.positional_suffix_gate = value.as_string();
                            } else if (key == "gate_reject_message") {
                                alias.gate_reject_message = value.as_string();
                            } else {
                                const auto location = value.location();
                                print_and_log_warning(
                                    *logger,
                                    M_("Unknown attribute \"{}\" of command flag alias \"{}\" in file \"{}\" "
                                       "on line {}: {}"),
                                    key,
                                    element_id_path,
                                    config_file_path.native(),
                                    location_first_line_num(location),
                                    location_lines(location));
                            }
                        }
                    } catch (const toml::type_error & e) {
                        auto location = e.location();
                        print_and_log_error(
                            *logger,
                            M_("Bad value type in file \"{}\" on line {}: {}"),
                            config_file_path.native(),
                            location_first_line_num(location),
                            location_lines(location));
                        continue;
                    }
                    if (broken) {
                        continue;
                    }
                    if (!defined) {
                        print_and_log_error(
                            *logger,
                            M_("Missing attribute \"attached_command\" or \"reject_message\" for element \"{}\" in "
                               "file \"{}\""),
                            element_id_path,
                            config_file_path.native());
                        continue;
                    }
                    command_flag_aliases.emplace(alias_key, std::move(alias));
                    continue;
                }
            }
            ArgParser::Command * element_parent_cmd;
            try {
                element_parent_cmd = &arg_parser.get_command(element_parent_id_path);
            } catch (const libdnf5::cli::ArgumentParserNotFoundError & e) {
                const auto location = element_options.location();
                print_and_log_error(
                    *logger,
                    M_("Parent command \"{}\" not found: {}: Requested in file \"{}\" on line {}: {}"),
                    element_parent_id_path,
                    e.what(),
                    config_file_path.native(),
                    location_first_line_num(location),
                    location_lines(location));
                continue;
            }

            enum class ElementType { GROUP, CLONED_NAMED_ARG, NAMED_ARG, COMMAND } element_type;
            try {
#ifdef TOML11_COMPAT
                const auto el_type = toml::find(element_options, "type");
#else
                const auto el_type = toml::find<toml::ordered_value>(element_options, "type");
#endif  // #ifdef TOML11_COMPAT
                const std::string type = el_type.as_string();
                if (type == "group") {
                    element_type = ElementType::GROUP;
                } else if (type == "cloned_named_arg") {
                    element_type = ElementType::CLONED_NAMED_ARG;
                } else if (type == "named_arg") {
                    element_type = ElementType::NAMED_ARG;
                } else if (type == "command") {
                    element_type = ElementType::COMMAND;
                } else {
                    const auto location = el_type.location();
                    print_and_log_error(
                        *logger,
                        M_("Unknown type \"{}\" of element \"{}\" in file \"{}\" on line {}: {}"),
                        type,
                        element_id_path,
                        config_file_path.native(),
                        location_first_line_num(location),
                        location_lines(location));
                    continue;
                }
            } catch (const std::out_of_range & e) {
                print_and_log_error(
                    *logger,
                    M_("Missing attribute \"type\" for element \"{}\" in file \"{}\""),
                    element_id_path,
                    config_file_path.native());
                continue;
            }

            // Check if an element of the given type with the given id path already exists
            bool found{false};
            switch (element_type) {
                case ElementType::GROUP:
                    break;
                case ElementType::CLONED_NAMED_ARG:
                case ElementType::NAMED_ARG:
                    for (auto * tmp : element_parent_cmd->get_named_args()) {
                        if (tmp->get_id() == element_id) {
                            found = true;
                            const auto location = element_options.location();
                            print_and_log_error(
                                *logger,
                                M_("Named argument \"{}\" already registered. Requested in file \"{}\" on line {}: {}"),
                                element_id_path,
                                config_file_path.native(),
                                location_first_line_num(location),
                                location_lines(location));
                            break;
                        }
                    }
                    break;
                case ElementType::COMMAND:
                    for (auto * tmp : element_parent_cmd->get_commands()) {
                        if (tmp->get_id() == element_id) {
                            found = true;
                            const auto location = element_options.location();
                            print_and_log_error(
                                *logger,
                                M_("Command \"{}\" already registered. Requested in file \"{}\" on line {}: {}"),
                                element_id_path,
                                config_file_path.native(),
                                location_first_line_num(location),
                                location_lines(location));
                            break;
                        }
                    }
            }
            if (found) {
                // An element with given id path already exists. Skip creation of new one.
                continue;
            }

            try {
                switch (element_type) {
                    // Creates a new group if a group with the given path id does not exist
                    case ElementType::GROUP: {
                        std::optional<std::string> header;
                        for (auto & [key, value] : element_options.as_table()) {
                            if (key == "type") {
                                continue;
                            } else if (key == "header") {
                                header = get_string_locale(value, locale_name);
                            } else {
                                const auto location = value.location();
                                print_and_log_warning(
                                    *logger,
                                    M_("Unknown attribute \"{}\" of group \"{}\" in file \"{}\" on line {}: {}"),
                                    key,
                                    element_id_path,
                                    config_file_path.native(),
                                    location_first_line_num(location),
                                    location_lines(location));
                            }
                        }

                        if (!header) {
                            print_and_log_error(
                                *logger,
                                M_("Missing attribute \"header\" for element \"{}\" in file \"{}\""),
                                element_id_path,
                                config_file_path.native());
                            continue;
                        }

                        // Check if the group already exists.
                        ArgParser::Group * group{nullptr};
                        for (auto * tmp_group : element_parent_cmd->get_groups()) {
                            if (tmp_group->get_id() == element_id) {
                                group = tmp_group;
                                break;
                            }
                        }
                        // If the group is not found, create it.
                        if (!group) {
                            group = arg_parser.add_new_group(element_id);
                            group->set_header(*header);
                            element_parent_cmd->register_group(group);
                        }
                        break;
                    }

                    // Creates a new named argument as a clone of the existing one
                    case ElementType::CLONED_NAMED_ARG: {
                        std::optional<std::string> long_name;
                        std::optional<char> short_name;
                        ArgParser::NamedArg * source{nullptr};
                        ArgParser::Group * group{nullptr};
                        bool complete{false};
                        for (auto & [key, value] : element_options.as_table()) {
                            if (key == "type") {
                                continue;
                            } else if (key == "long_name") {
                                long_name = value.as_string();
                            } else if (key == "short_name") {
                                const std::string tmp = value.as_string();
                                if (tmp.length() != 1) {
                                    const auto location = value.location();
                                    print_and_log_error(
                                        *logger,
                                        M_("The \"short_name\" attribute of named argument \"{}\" "
                                           "must be one character long "
                                           "in file \"{}\" on line {}: {}"),
                                        element_id_path,
                                        config_file_path.native(),
                                        location_first_line_num(location),
                                        location_lines(location));
                                    continue;
                                }
                                short_name = tmp[0];
                            } else if (key == "source") {
                                const std::string source_id_path = value.as_string();
                                try {
                                    source = &arg_parser.get_named_arg(source_id_path, false);
                                } catch (const libdnf5::cli::ArgumentParserNotFoundError & e) {
                                    const auto location = value.location();
                                    print_and_log_error(
                                        *logger,
                                        M_("Source \"{}\" not found: {}: Requested in file \"{}\" on line {}: {}"),
                                        source_id_path,
                                        e.what(),
                                        config_file_path.native(),
                                        location_first_line_num(location),
                                        location_lines(location));
                                    continue;
                                }
                            } else if (key == "group_id") {
                                const std::string group_id = value.as_string();
                                try {
                                    group = &element_parent_cmd->get_group(group_id);
                                } catch (const libdnf5::cli::ArgumentParserNotFoundError & e) {
                                    const auto location = value.location();
                                    print_and_log_error(
                                        *logger,
                                        M_("Group \"{}\" not found: {}: Requested in file \"{}\" on line {}: {}"),
                                        group_id,
                                        e.what(),
                                        config_file_path.native(),
                                        location_first_line_num(location),
                                        location_lines(location));
                                    continue;
                                }
                            } else if (key == "complete") {
                                complete = value.as_boolean();
                            } else {
                                const auto location = value.location();
                                print_and_log_warning(
                                    *logger,
                                    M_("Unknown attribute \"{}\" of named argument \"{}\" "
                                       "in file \"{}\" on line {}: {}"),
                                    key,
                                    element_id_path,
                                    config_file_path.native(),
                                    location_first_line_num(location),
                                    location_lines(location));
                            }
                        }

                        if (!source) {
                            print_and_log_error(
                                *logger,
                                M_("Missing attribute \"source\" for named argument \"{}\" in file \"{}\""),
                                element_id_path,
                                config_file_path.native());
                            continue;
                        }

                        if ((!long_name || long_name->empty()) && (!short_name || short_name == '\0')) {
                            print_and_log_error(
                                *logger,
                                M_("At least one of the attributes \"long_name\" and \"short_name\" must be set "
                                   "for named argument \"{}\" in file \"{}\""),
                                element_id_path,
                                config_file_path.native());
                            continue;
                        }

                        auto alias_arg = source->add_alias(
                            element_id, long_name ? *long_name : "", short_name ? *short_name : '\0', nullptr);
                        alias_arg->set_complete(complete);

                        if (group) {
                            group->register_argument(alias_arg);
                        }
                        element_parent_cmd->register_named_arg(alias_arg);
                        break;
                    }

                    // Creates a new named argument
                    case ElementType::NAMED_ARG: {
                        std::optional<std::string> long_name;
                        std::optional<char> short_name;
                        std::optional<std::string> description;
                        bool has_value{false};
                        std::optional<std::string> value_help;
                        std::optional<std::string> const_value;
                        ArgParser::Group * group{nullptr};
                        bool complete{false};
                        const decltype(arg_parser_elements)::array_type * attached_named_args = nullptr;
                        for (auto & [key, value] : element_options.as_table()) {
                            if (key == "type") {
                                continue;
                            } else if (key == "long_name") {
                                long_name = value.as_string();
                            } else if (key == "short_name") {
                                const std::string tmp = value.as_string();
                                if (tmp.length() != 1) {
                                    const auto location = value.location();
                                    print_and_log_error(
                                        *logger,
                                        M_("The \"short_name\" attribute of named argument \"{}\" must be one "
                                           "character long in file \"{}\" on line {}: {}"),
                                        element_id_path,
                                        config_file_path.native(),
                                        location_first_line_num(location),
                                        location_lines(location));
                                    continue;
                                }
                                short_name = tmp[0];
                            } else if (key == "descr") {
                                description = get_string_locale(value, locale_name);
                            } else if (key == "has_value") {
                                has_value = value.as_boolean();
                            } else if (key == "value_help") {
                                value_help = get_string_locale(value, locale_name);
                            } else if (key == "const_value") {
                                const_value = value.as_string();
                            } else if (key == "group_id") {
                                const std::string group_id = value.as_string();
                                try {
                                    group = &element_parent_cmd->get_group(group_id);
                                } catch (const libdnf5::cli::ArgumentParserNotFoundError & e) {
                                    const auto location = value.location();
                                    print_and_log_error(
                                        *logger,
                                        M_("Group \"{}\" not found: {}: Requested in file \"{}\" on line {}: {}"),
                                        group_id,
                                        e.what(),
                                        config_file_path.native(),
                                        location_first_line_num(location),
                                        location_lines(location));
                                    continue;
                                }
                            } else if (key == "complete") {
                                complete = value.as_boolean();
                            } else if (key == "attached_named_args") {
                                attached_named_args = &value.as_array();
                            } else {
                                const auto location = value.location();
                                print_and_log_warning(
                                    *logger,
                                    M_("Unknown attribute \"{}\" of named argument \"{}\" "
                                       "in file \"{}\" on line {}: {}"),
                                    key,
                                    element_id_path,
                                    config_file_path.native(),
                                    location_first_line_num(location),
                                    location_lines(location));
                            }
                        }

                        if ((!long_name || long_name->empty()) && (!short_name || short_name == '\0')) {
                            print_and_log_error(
                                *logger,
                                M_("At least one of the attributes \"long_name\" and \"short_name\" must be set "
                                   "for named argument \"{}\" in file \"{}\""),
                                element_id_path,
                                config_file_path.native());
                            continue;
                        }

                        auto * alias_arg = arg_parser.add_new_named_arg(element_id);
                        if (long_name) {
                            alias_arg->set_long_name(*long_name);
                        }
                        if (short_name) {
                            alias_arg->set_short_name(*short_name);
                        }
                        if (description) {
                            alias_arg->set_description(*description);
                        }
                        alias_arg->set_has_value(has_value);
                        if (value_help) {
                            alias_arg->set_arg_value_help(*value_help);
                        }
                        if (const_value) {
                            alias_arg->set_const_value(*const_value);
                        }
                        alias_arg->set_complete(complete);

                        // Attach named arguments
                        if (attached_named_args) {
                            if (!attach_named_args(
                                    *logger, config_file_path, *alias_arg, *attached_named_args, element_id_path)) {
                                continue;
                            }
                        }

                        if (group) {
                            group->register_argument(alias_arg);
                        }
                        element_parent_cmd->register_named_arg(alias_arg);
                        break;
                    }

                    // Creates a new command
                    case ElementType::COMMAND: {
                        bool error{false};
                        ArgParser::Command * attached_command{nullptr};
                        struct RequiredValue {
                            std::string value_help;
                            std::string descr;
                        };
                        std::vector<RequiredValue> required_values;
                        std::optional<std::string> description;
                        ArgParser::Group * group{nullptr};
                        bool complete{false};
                        const decltype(arg_parser_elements)::array_type * attached_named_args = nullptr;
                        for (auto & [key, value] : element_options.as_table()) {
                            if (key == "type") {
                                continue;
                            } else if (key == "attached_command") {
                                const std::string attached_command_id_path = value.as_string();
                                try {
                                    attached_command = &arg_parser.get_command(attached_command_id_path);
                                } catch (const libdnf5::cli::ArgumentParserNotFoundError & e) {
                                    const auto location = value.location();
                                    print_and_log_error(
                                        *logger,
                                        M_("Attached command \"{}\" not found: {}: "
                                           "Requested in file \"{}\" on line {}: {}"),
                                        attached_command_id_path,
                                        e.what(),
                                        config_file_path.native(),
                                        location_first_line_num(location),
                                        location_lines(location));
                                    continue;
                                }
                            } else if (key == "descr") {
                                description = get_string_locale(value, locale_name);
                            } else if (key == "group_id") {
                                const std::string group_id = value.as_string();
                                try {
                                    group = &element_parent_cmd->get_group(group_id);
                                } catch (const libdnf5::cli::ArgumentParserNotFoundError & e) {
                                    const auto location = value.location();
                                    print_and_log_error(
                                        *logger,
                                        M_("Group \"{}\" not found: {}: Requested in file \"{}\" on line {}: {}"),
                                        group_id,
                                        e.what(),
                                        config_file_path.native(),
                                        location_first_line_num(location),
                                        location_lines(location));
                                    continue;
                                }
                            } else if (key == "complete") {
                                complete = value.as_boolean();
                            } else if (key == "required_values") {
                                if (version == "1.0") {
                                    auto location = value.location();
                                    print_and_log_error(
                                        *logger,
                                        M_("Used config file version \"1.0\" for attribute \"{}\" of command \"{}\" "
                                           "in file \"{}\" on line {}: {}"),
                                        key,
                                        element_id_path,
                                        config_file_path.native(),
                                        location_first_line_num(location),
                                        location_lines(location));
                                    break;
                                }
                                const auto & req_values = value.as_array();
                                for (const auto & required_value : req_values) {
                                    std::optional<std::string> value_help;
                                    std::string descr;
                                    for (auto & [key, value] : required_value.as_table()) {
                                        if (key == "value_help") {
                                            value_help = get_string_locale(value, locale_name);
                                        } else if (key == "descr") {
                                            descr = get_string_locale(value, locale_name).value_or("");
                                        } else {
                                            const auto & location = value.location();
                                            print_and_log_warning(
                                                *logger,
                                                M_("Unknown attribute \"{}\" of required value for alias \"{}\" "
                                                   "in file \"{}\" on line {}: {}"),
                                                key,
                                                element_id_path,
                                                config_file_path.native(),
                                                location_first_line_num(location),
                                                location_lines(location));
                                        }
                                    }
                                    if (!value_help) {
                                        error = true;
                                        print_and_log_error(
                                            *logger,
                                            M_("Missing attribute \"value_help\" of required value for alias \"{}\" in "
                                               "file \"{}\""),
                                            element_id_path,
                                            config_file_path.native());
                                        break;
                                    }
                                    required_values.emplace_back(*value_help, descr);
                                }
                            } else if (key == "attached_named_args") {
                                attached_named_args = &value.as_array();
                            } else {
                                const auto location = value.location();
                                print_and_log_warning(
                                    *logger,
                                    M_("Unknown attribute \"{}\" of command \"{}\" in file \"{}\" on line {}: {}"),
                                    key,
                                    element_id_path,
                                    config_file_path.native(),
                                    location_first_line_num(location),
                                    location_lines(location));
                            }
                        }

                        if (error) {
                            continue;
                        }

                        if (!attached_command) {
                            print_and_log_error(
                                *logger,
                                M_("Missing attribute \"attached_command\" for command \"{}\" in file \"{}\""),
                                element_id_path,
                                config_file_path.native());
                            continue;
                        }
                        auto * alias_cmd = arg_parser.add_new_command_alias(element_id, *attached_command);

                        if (description) {
                            alias_cmd->set_description(*description);
                        }
                        alias_cmd->set_complete(complete);

                        // Add required values to the command alias
                        for (const auto & [value_help, descr] : required_values) {
                            alias_cmd->add_required_value(value_help, descr);
                        }

                        // Attach additional named arguments to the command alias
                        if (attached_named_args) {
                            if (!attach_named_args(
                                    *logger, config_file_path, *alias_cmd, *attached_named_args, element_id_path)) {
                                continue;
                            }
                        }

                        if (group) {
                            group->register_argument(alias_cmd);
                        }
                        element_parent_cmd->register_command(alias_cmd);
                    }
                }
            } catch (const toml::type_error & e) {
                auto loc = e.location();
                print_and_log_error(
                    *logger,
                    M_("Bad value type in file \"{}\" on line {}: {}"),
                    config_file_path.native(),
                    location_first_line_num(loc),
                    location_lines(loc));
            }
        }
    } catch (const toml::syntax_error & e) {
        logger->error("{}", e.what());

#ifdef TOML11_COMPAT
        auto loc = e.location();
        auto msg = libdnf5::utils::format(
            true,
            M_("Syntax error in file \"{}\" on line {}:"),
            1,
            config_file_path.native(),
            location_first_line_num(loc));
        std::cerr << msg << std::endl;
        std::cerr << e.what() << std::endl;
#else
        for (const auto & err : e.errors()) {
            std::cerr << err;
        }
#endif  // #ifdef TOML11_COMPAT
    }
}

}  // namespace

void load_cmdline_aliases(
    Context & context,
    const std::filesystem::path & config_dir_path,
    const std::string & locale,
    CommandFlagAliasTable & command_flag_aliases) {
    auto logger = context.get_base().get_logger();

    std::vector<fs::path> config_paths;
    std::error_code ec;  // Do not report errors if config_dir_path refers to a non-existing file or not a directory
    for (const auto & p : std::filesystem::directory_iterator(config_dir_path, ec)) {
        if ((p.is_regular_file() || p.is_symlink()) && p.path().extension() == ".conf") {
            config_paths.emplace_back(p.path());
        }
    }
    std::sort(config_paths.begin(), config_paths.end());

    const std::string locale_name = locale.substr(0, locale.find('.'));  // Strip encoding (e.g. ".UTF-8") from locale
    for (const auto & path : config_paths) {
        load_aliases_from_toml_file(context, path, locale_name, command_flag_aliases);
    }
}


namespace {

/// Collects the value-taking named arguments visible to the rewrite: global
/// options on the root command, options of the given command word, and
/// options of every resolvable prefix of the attached command path.
std::vector<libdnf5::cli::ArgumentParser::NamedArg *> collect_value_args(
    libdnf5::cli::ArgumentParser & arg_parser, const std::string & cmd_word, const std::vector<std::string> & words) {
    std::vector<libdnf5::cli::ArgumentParser::NamedArg *> value_args;
    auto add_from = [&value_args, &arg_parser](const std::string & path) {
        try {
            for (auto * named_arg : arg_parser.get_command(path).get_named_args()) {
                if (named_arg->get_has_value()) {
                    value_args.push_back(named_arg);
                }
            }
        } catch (const libdnf5::cli::ArgumentParserNotFoundError &) {
        }
    };
    add_from("");
    if (!cmd_word.empty()) {
        add_from(cmd_word);
    }
    std::string path;
    for (const auto & word : words) {
        path = path.empty() ? word : path + "." + word;
        add_from(path);
    }
    return value_args;
}

/// Whether the token is an option that takes its value as the next argument.
bool takes_separate_value(
    const std::vector<libdnf5::cli::ArgumentParser::NamedArg *> & value_args, const std::string & token) {
    if (token.rfind("-", 0) != 0 || token.find('=') != std::string::npos) {
        return false;
    }
    for (auto * named_arg : value_args) {
        if (token.rfind("--", 0) == 0) {
            if (!named_arg->get_long_name().empty() && token.substr(2) == named_arg->get_long_name()) {
                return true;
            }
        } else if (token.size() == 2 && named_arg->get_short_name() == token[1]) {
            return true;
        }
    }
    return false;
}

}  // namespace

std::optional<RewrittenArgv> apply_command_flag_aliases(
    Context & context, const CommandFlagAliasTable & command_flag_aliases, int argc, const char * const * argv) {
    if (command_flag_aliases.empty()) {
        return std::nullopt;
    }
    auto & arg_parser = context.get_argument_parser();
    auto & logger = *context.get_base().get_logger();

    // The command word is the first argument that is not an option and not
    // the value of a preceding value-taking global option.
    const auto root_value_args = collect_value_args(arg_parser, "", {});
    int cmd_i = -1;
    for (int i = 1; i < argc; ++i) {
        if (argv[i][0] != '-') {
            cmd_i = i;
            break;
        }
        if (takes_separate_value(root_value_args, argv[i])) {
            ++i;  // the next token is the option's value, not the command
        }
    }
    if (cmd_i <= 0) {
        return std::nullopt;
    }
    const std::string cmd_word = argv[cmd_i];

    // Select the entry to execute before executing anything: a dnf4 line can
    // carry several matching flags in any order ('updateinfo --all --list').
    // The highest-precedence match executes; ties go to the leftmost token.
    // A trigger that names an existing option of the command would change
    // the meaning of a working command line; such entries are ignored.
    auto is_existing_option = [&arg_parser, &cmd_word](const std::string & flag_name) {
        for (const auto & path : {std::string(), cmd_word}) {
            try {
                for (auto * named_arg : arg_parser.get_command(path).get_named_args()) {
                    if (named_arg->get_long_name() == flag_name) {
                        return true;
                    }
                }
            } catch (const libdnf5::cli::ArgumentParserNotFoundError &) {
            }
        }
        return false;
    };

    auto best_it = command_flag_aliases.end();
    int best_i = -1;
    std::set<std::string> shadow_warned;
    // The scan must know the value-taking options of every candidate
    // attached command too: their values must not be read as triggers.
    auto scan_value_args = collect_value_args(arg_parser, cmd_word, {});
    for (const auto & [alias_key, alias_entry] : command_flag_aliases) {
        if (alias_key.first != cmd_word || alias_entry.words.empty()) {
            continue;
        }
        for (auto * named_arg : collect_value_args(arg_parser, "", alias_entry.words)) {
            if (std::find(scan_value_args.begin(), scan_value_args.end(), named_arg) == scan_value_args.end()) {
                scan_value_args.push_back(named_arg);
            }
        }
    }
    for (int i = cmd_i + 1; i < argc; ++i) {
        std::string token = argv[i];
        if (takes_separate_value(scan_value_args, token)) {
            ++i;  // the next token is the option's value, not a trigger
            continue;
        }
        const auto eq = token.find('=');
        if (token.rfind("--", 0) == 0 && eq != std::string::npos) {
            token = token.substr(0, eq);
        }
        const auto entry = command_flag_aliases.find({cmd_word, token});
        if (entry == command_flag_aliases.end()) {
            continue;
        }
        if (is_existing_option(entry->first.second.substr(2))) {
            if (shadow_warned.insert(entry->first.second).second) {
                print_and_log_warning(
                    logger,
                    M_("Command flag alias \"{}.{}\" matches an existing option of the command, ignored"),
                    cmd_word,
                    entry->first.second.substr(2));
            }
            continue;
        }
        if (best_it == command_flag_aliases.end() || entry->second.precedence > best_it->second.precedence) {
            best_it = entry;
            best_i = i;
        }
    }
    if (best_it == command_flag_aliases.end()) {
        return std::nullopt;
    }
    // Everything after a bare "--" is an operand by parser contract; the
    // rewrite cannot preserve that boundary, so leave such lines untouched.
    for (int i = 1; i < argc; ++i) {
        if (std::string_view{argv[i]} == "--") {
            return std::nullopt;
        }
    }
    const auto & alias = best_it->second;

    if (!alias.reject_message.empty()) {
        // Print the guidance and leave the arguments untouched so the parser
        // produces its normal error.
        std::cerr << alias.reject_message << std::endl;
        logger.warning("Command flag alias for \"{} {}\": {}", cmd_word, argv[best_i], alias.reject_message);
        return std::nullopt;
    }

    // Validate the attached command before rewriting so a broken alias entry
    // produces a clear error instead of a confusing parse error.
    std::string attached_path;
    for (const auto & word : alias.words) {
        attached_path = attached_path.empty() ? word : attached_path + "." + word;
    }
    try {
        arg_parser.get_command(attached_path);
    } catch (const libdnf5::cli::ArgumentParserNotFoundError &) {
        print_and_log_error(
            logger,
            M_("Attached command \"{}\" of command flag alias \"{}{}\" not found"),
            attached_path,
            cmd_word + ".",
            best_it->first.second.substr(2));
        return std::nullopt;
    }

    std::string eq_value;
    bool has_eq = false;
    {
        const std::string token = argv[best_i];
        const auto eq = token.find('=');
        if (token.rfind("--", 0) == 0 && eq != std::string::npos) {
            has_eq = true;
            eq_value = token.substr(eq + 1);
        }
    }
    // A value given in the --flag=VALUE form is only meaningful with a
    // template, and an empty value is only meaningful to reject. Otherwise
    // print a note and leave the arguments untouched so the parser error
    // stands instead of the value being silently discarded.
    if (has_eq && (alias.positional_template.empty() || eq_value.empty())) {
        const auto note =
            alias.positional_template.empty()
                ? libdnf5::utils::sformat(
                      _("Command flag alias \"{}.{}\" does not accept a value"),
                      cmd_word,
                      best_it->first.second.substr(2))
                : libdnf5::utils::sformat(
                      _("Command flag alias \"{}.{}\" requires a value"), cmd_word, best_it->first.second.substr(2));
        std::cerr << note << std::endl;
        logger.warning("{}", note);
        return std::nullopt;
    }

    const auto value_args = collect_value_args(arg_parser, cmd_word, alias.words);

    if (!alias.positional_suffix_gate.empty()) {
        bool gate_ok = true;
        const auto & suffix = alias.positional_suffix_gate;
        auto ends_with = [&suffix](const std::string & value) {
            return value.size() >= suffix.size() &&
                   value.compare(value.size() - suffix.size(), suffix.size(), suffix) == 0;
        };
        if (!eq_value.empty() && !ends_with(eq_value)) {
            gate_ok = false;
        }
        bool skip_value = false;
        const int gate_from = alias.leading_positionals ? cmd_i + 1 : best_i + 1;
        for (int j = gate_from; gate_ok && j < argc; ++j) {
            if (j == best_i) {
                continue;
            }
            const std::string value = argv[j];
            if (skip_value) {
                skip_value = false;
                continue;
            }
            if (takes_separate_value(value_args, value)) {
                skip_value = true;
                continue;
            }
            if (value.rfind("-", 0) != 0 && !ends_with(value)) {
                gate_ok = false;
            }
        }
        if (!gate_ok) {
            // Value shape does not match; print the guidance and leave the
            // arguments untouched (the normal parser error stands).
            if (!alias.gate_reject_message.empty()) {
                std::cerr << alias.gate_reject_message << std::endl;
                logger.warning(
                    "Command flag alias for \"{} {}\": {}", cmd_word, argv[best_i], alias.gate_reject_message);
            }
            return std::nullopt;
        }
    }

    auto apply_template = [](const std::string & template_string, const std::string & value) {
        std::string out = template_string;
        const auto placeholder = out.find("${}");
        if (placeholder != std::string::npos) {
            out.replace(placeholder, 3, value);
        }
        return out;
    };

    RewrittenArgv rewritten;
    bool copy_value_verbatim = false;
    for (int j = 0; j < argc; ++j) {
        if (j == cmd_i) {
            for (const auto & word : alias.words) {
                rewritten.storage.emplace_back(word);
            }
            for (const auto & flag : alias.attached_flags) {
                rewritten.storage.emplace_back(flag);
            }
            if (!eq_value.empty()) {
                rewritten.storage.emplace_back(apply_template(alias.positional_template, eq_value));
            }
            continue;
        }
        if (j == best_i) {
            continue;
        }
        std::string current = argv[j];
        if (copy_value_verbatim) {
            // The value of the preceding value-taking option, not an operand.
            copy_value_verbatim = false;
            rewritten.storage.emplace_back(std::move(current));
            continue;
        }
        if (j > cmd_i && !alias.companion_flag.empty() && current == alias.companion_flag) {
            continue;  // consumed together with the trigger flag
        }
        // dnf4 grammars marked leading_positionals accepted operands before
        // the flag too; tokens before the command word stay untouched.
        const bool operand_scope = j > best_i || (alias.leading_positionals && j > cmd_i && j < best_i);
        if (j > cmd_i && takes_separate_value(value_args, current)) {
            // The next token is this option's value wherever the option
            // stands, so the value slot is protected in the whole line.
            copy_value_verbatim = true;
            rewritten.storage.emplace_back(std::move(current));
            continue;
        }
        if (operand_scope && current.rfind("-", 0) != 0 && alias.drop_bare_positionals &&
            current.find('=') == std::string::npos) {
            // dnf4 allowed redundant bare arguments here; dnf5's target
            // grammar has no place for them. Drop, loudly.
            if (!alias.dropped_note.empty()) {
                const auto note =
                    libdnf5::utils::sformat(_("{}: ignoring argument \"{}\""), alias.dropped_note, current);
                std::cerr << note << std::endl;
                logger.warning("{}", note);
            }
            continue;
        }
        if (operand_scope && current.rfind("-", 0) != 0 && !alias.positional_template.empty()) {
            rewritten.storage.emplace_back(apply_template(alias.positional_template, current));
            continue;
        }
        if (operand_scope && current.rfind("-", 0) == 0) {
            bool mapped = false;
            for (const auto & [prefix, map_template] : alias.token_prefix_maps) {
                if (current.rfind(prefix, 0) == 0) {
                    rewritten.storage.emplace_back(apply_template(map_template, current.substr(prefix.size())));
                    mapped = true;
                    break;
                }
            }
            if (mapped) {
                continue;
            }
        }
        rewritten.storage.emplace_back(std::move(current));
    }
    rewritten.argv.reserve(rewritten.storage.size());
    for (const auto & piece : rewritten.storage) {
        rewritten.argv.push_back(piece.c_str());
    }
    logger.debug(
        "Command flag alias \"{}.{}\" rewrote the command line to: {}",
        cmd_word,
        best_it->first.second.substr(2),
        libdnf5::utils::string::join(rewritten.storage, " "));
    return rewritten;
}

}  // namespace dnf5
