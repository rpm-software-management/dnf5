/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "cmdline_aliases.hpp"

#include "utils/string.hpp"

#include <libdnf5/common/preserve_order_map.hpp>
#include <libdnf5/utils/bgettext/bgettext-mark-domain.h>
#include <libdnf5/utils/format_locale.hpp>
#include <toml.hpp>

#include <iostream>
#include <optional>
#include <vector>

namespace fs = std::filesystem;

namespace dnf5 {

namespace {

constexpr const char * CONF_FILE_VERSION = "1.0";

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


template <typename... Args>
void print_and_log_error(libdnf5::Logger & logger, BgettextMessage fmt_string, Args... args) {
    logger.error(b_gettextmsg_get_id(fmt_string), args...);
    std::cerr << libdnf5::utils::format(true, fmt_string, 1, args...) << std::endl;
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
                logger.warning(
                    "Unknown attribute \"{}\" of attached named argument for alias \"{}\" "
                    "in file \"{}\" on line {}: {}",
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

void load_aliases_from_toml_file(Context & context, const fs::path & config_file_path) {
    auto & arg_parser = context.get_argument_parser();
    auto logger = context.get_base().get_logger();

    try {
#ifdef TOML11_COMPAT
        const auto arg_parser_elements =
            toml::parse<::toml::discard_comments, libdnf5::PreserveOrderMap, std::vector>(config_file_path);
#else
        const toml::ordered_value arg_parser_elements = toml::parse<toml::ordered_type_config>(config_file_path);
#endif  // #ifdef TOML11_COMPAT

        try {
            const auto version = toml::find<std::string>(arg_parser_elements, "version");
            if (version != CONF_FILE_VERSION) {
                print_and_log_error(
                    *logger,
                    M_("Unsupported version \"{}\" in file \"{}\", \"{}\" expected"),
                    version,
                    config_file_path.native(),
                    CONF_FILE_VERSION);
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
                logger->warning(
                    "Unknown key \"{}\" in file \"{}\" on line {}: {}",
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
                                header = value.as_string();
                            } else {
                                const auto location = value.location();
                                logger->warning(
                                    "Unknown attribute \"{}\" of group \"{}\" in file \"{}\" on line {}: {}",
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
                                logger->warning(
                                    "Unknown attribute \"{}\" of named argument \"{}\" in file \"{}\" on line {}: {}",
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
                                description = value.as_string();
                            } else if (key == "has_value") {
                                has_value = value.as_boolean();
                            } else if (key == "value_help") {
                                value_help = value.as_string();
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
                                logger->warning(
                                    "Unknown attribute \"{}\" of named argument \"{}\" in file \"{}\" on line {}: {}",
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
                        ArgParser::Command * attached_command{nullptr};
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
                                description = value.as_string();
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
                                logger->warning(
                                    "Unknown attribute \"{}\" of command \"{}\" in file \"{}\" on line {}: {}",
                                    key,
                                    element_id_path,
                                    config_file_path.native(),
                                    location_first_line_num(location),
                                    location_lines(location));
                            }
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
            M_("Syntax error in file \"{}\" on line {}"),
            config_file_path.native(),
            location_first_line_num(loc));
        std::cerr << msg << std::endl;
#else
        for (const auto & err : e.errors()) {
            std::cerr << err;
        }
#endif  // #ifdef TOML11_COMPAT
    }
}

}  // namespace

void load_cmdline_aliases(Context & context, const std::filesystem::path & config_dir_path) {
    auto logger = context.get_base().get_logger();

    std::vector<fs::path> config_paths;
    std::error_code ec;  // Do not report errors if config_dir_path refers to a non-existing file or not a directory
    for (const auto & p : std::filesystem::directory_iterator(config_dir_path, ec)) {
        if ((p.is_regular_file() || p.is_symlink()) && p.path().extension() == ".conf") {
            config_paths.emplace_back(p.path());
        }
    }
    std::sort(config_paths.begin(), config_paths.end());

    std::string failed_filenames;
    for (const auto & path : config_paths) {
        load_aliases_from_toml_file(context, path);
    }
}

}  // namespace dnf5
