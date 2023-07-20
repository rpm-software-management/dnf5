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

#include "libdnf5/conf/config_parser.hpp"

#include "utils/fs/file.hpp"
#include "utils/iniparser.hpp"
#include "utils/on_scope_exit.hpp"

#include "libdnf5/utils/bgettext/bgettext-mark-domain.h"

#include <glob.h>

#include <algorithm>

namespace {

constexpr std::string_view INCLUDE_KEY = "#!include_config ";
constexpr auto INCLUDE_KEY_LEN = INCLUDE_KEY.length();

}  // namespace

namespace libdnf5 {

static void read(
    ConfigParser & cfg_parser,
    const std::string & file_path,
    bool included,
    std::filesystem::path & include_root_path) try {
    IniParser parser(file_path);
    IniParser::ItemType readed_type;
    while ((readed_type = parser.next()) != IniParser::ItemType::END_OF_INPUT) {
        auto section = parser.get_section();
        if (readed_type == IniParser::ItemType::SECTION) {
            cfg_parser.add_section(std::move(section), std::move(parser.get_raw_item()));
        } else if (readed_type == IniParser::ItemType::KEY_VAL) {
            cfg_parser.set_value(
                section, std::move(parser.get_key()), std::move(parser.get_value()), std::move(parser.get_raw_item()));
        } else if (readed_type == IniParser::ItemType::COMMENT_LINE || readed_type == IniParser::ItemType::EMPTY_LINE) {
            if (section.empty()) {
                cfg_parser.get_header() += parser.get_raw_item();
            } else {
                cfg_parser.add_comment_line(section, parser.get_raw_item());
            }

            // Support for loading included configuration files.
            if (!included && readed_type == IniParser::ItemType::COMMENT_LINE &&
                parser.get_raw_item().starts_with(INCLUDE_KEY)) {
                auto end_pos = parser.get_raw_item().find_first_of("\r\n");
                std::filesystem::path glob_path =
                    parser.get_raw_item().substr(INCLUDE_KEY_LEN, end_pos - INCLUDE_KEY_LEN);

                if (glob_path.is_relative()) {
                    throw Error(M_("\"{}{}\": Only absolute paths allowed."), INCLUDE_KEY, glob_path.native());
                }

                if (!include_root_path.empty() && include_root_path != "/") {
                    glob_path = include_root_path / glob_path.relative_path();
                }

                glob_t glob_buf;
                glob(glob_path.c_str(), GLOB_MARK, nullptr, &glob_buf);
                utils::OnScopeExit free_glob_buf([&glob_buf]() noexcept { ::globfree(&glob_buf); });
                for (size_t i = 0; i < glob_buf.gl_pathc; ++i) {
                    auto path = glob_buf.gl_pathv[i];
                    if (path[strlen(path) - 1] != '/') {  // directories are skipped
                        ::libdnf5::read(cfg_parser, path, true, include_root_path);
                    }
                }
            }
        }
    }
} catch (const ConfigParserError &) {
    throw;
} catch (const std::filesystem::filesystem_error & e) {
    if (e.code().value() == ENOENT) {
        std::throw_with_nested(MissingConfigError(M_("Configuration file \"{}\" not found"), file_path));
    } else {
        std::throw_with_nested(InaccessibleConfigError(M_("Unable to access configuration file \"{}\""), file_path));
    }
} catch (const Error & e) {
    std::throw_with_nested(InvalidConfigError(M_("Error in configuration file \"{}\""), file_path));
}

ConfigParserSectionNotFoundError::ConfigParserSectionNotFoundError(const std::string & section)
    : ConfigParserError(M_("Section \"{}\" not found"), section) {}

ConfigParserOptionNotFoundError::ConfigParserOptionNotFoundError(
    const std::string & section, const std::string & option)
    : ConfigParserError(M_("Section \"{}\" does not contain option \"{}\""), section, option) {}

void ConfigParser::read(const std::string & file_path) {
    ::libdnf5::read(*this, file_path, false, include_root_path);
}

static std::string create_raw_item(const std::string & value, const std::string & old_raw_item) {
    auto eql_pos = old_raw_item.find('=');
    if (eql_pos == std::string::npos) {
        return "";
    }
    auto value_pos = old_raw_item.find_first_not_of(" \t", eql_pos + 1);
    auto key_and_delim_length = value_pos != std::string::npos ? value_pos : old_raw_item.length();
    return old_raw_item.substr(0, key_and_delim_length) + value + '\n';
}

void ConfigParser::set_value(const std::string & section, const std::string & key, const std::string & value) {
    auto raw_iter = raw_items.find(section + ']' + key);
    auto raw = create_raw_item(value, raw_iter != raw_items.end() ? raw_iter->second : "");
    set_value(section, key, value, raw);
}

void ConfigParser::set_value(const std::string & section, std::string && key, std::string && value) {
    auto raw_iter = raw_items.find(section + ']' + key);
    auto raw = create_raw_item(value, raw_iter != raw_items.end() ? raw_iter->second : "");
    set_value(section, std::move(key), std::move(value), std::move(raw));
}

const std::string & ConfigParser::get_value(const std::string & section, const std::string & key) const {
    auto sect = data.find(section);
    if (sect == data.end()) {
        throw ConfigParserSectionNotFoundError(section);
    }
    auto key_val = sect->second.find(key);
    if (key_val == sect->second.end()) {
        throw ConfigParserOptionNotFoundError(section, key);
    }
    return key_val->second;
}

static void write_key_vals(
    utils::fs::File & file,
    const std::string & section,
    const ConfigParser::Container::mapped_type & key_val_map,
    const std::map<std::string, std::string> & raw_items) {
    for (const auto & key_val : key_val_map) {
        auto first = key_val.first[0];
        if (first == '#' || first == ';') {
            file.write(key_val.second);
        } else {
            auto raw_item = raw_items.find(section + ']' + key_val.first);
            if (raw_item != raw_items.end()) {
                file.write(raw_item->second);
            } else {
                file.write(key_val.first);
                file.putc('=');
                for (const auto chr : key_val.second) {
                    file.putc(chr);
                    if (chr == '\n') {
                        file.putc(' ');
                    }
                }
                file.putc('\n');
            }
        }
    }
}

static void write_section(
    utils::fs::File & file,
    const std::string & section,
    const ConfigParser::Container::mapped_type & key_val_map,
    const std::map<std::string, std::string> & raw_items) {
    auto raw_item = raw_items.find(section);
    if (raw_item != raw_items.end()) {
        file.write(raw_item->second);
    } else {
        file.write(fmt::format("[{}]\n", raw_item->second));
    }
    write_key_vals(file, section, key_val_map, raw_items);
}

void ConfigParser::write(const std::string & file_path, bool append) const {
    utils::fs::File file(file_path, append ? "a" : "w");

    file.write(header);
    for (const auto & section : data) {
        write_section(file, section.first, section.second, raw_items);
    }
}

void ConfigParser::write(const std::string & file_path, bool append, const std::string & section) const {
    auto sit = data.find(section);
    if (sit == data.end()) {
        throw ConfigParserSectionNotFoundError(section);
    }

    utils::fs::File file(file_path, append ? "a" : "w");
    write_section(file, sit->first, sit->second, raw_items);
}

}  // namespace libdnf5
