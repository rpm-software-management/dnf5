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

#include "utils/iniparser.hpp"

#include "libdnf5/utils/bgettext/bgettext-mark-domain.h"
#include "libdnf5/utils/fs/file.hpp"

#include <map>
#include <utility>


namespace libdnf5 {

static void read(ConfigParser & cfg_parser, IniParser & parser) {
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
                cfg_parser.add_comment_line(section, std::move(parser.get_raw_item()));
            }
        }
    }
}


ConfigParserSectionNotFoundError::ConfigParserSectionNotFoundError(const std::string & section)
    : ConfigParserError(M_("Section \"{}\" not found"), section) {}


ConfigParserOptionNotFoundError::ConfigParserOptionNotFoundError(
    const std::string & section, const std::string & option)
    : ConfigParserError(M_("Section \"{}\" does not contain option \"{}\""), section, option) {}


class ConfigParser::Impl {
private:
    friend ConfigParser;
    Container data;
    int item_number{0};
    std::string header;
    std::map<std::string, std::string> raw_items;
};

ConfigParser::ConfigParser() : p_impl(std::make_unique<Impl>()) {}

ConfigParser::~ConfigParser() = default;

ConfigParser::ConfigParser(const ConfigParser & src) : p_impl(new Impl(*src.p_impl)) {}
ConfigParser::ConfigParser(ConfigParser && src) noexcept = default;

ConfigParser & ConfigParser::operator=(const ConfigParser & src) {
    if (this != &src) {
        if (p_impl) {
            *p_impl = *src.p_impl;
        } else {
            p_impl = std::make_unique<Impl>(*src.p_impl);
        }
    }

    return *this;
}
ConfigParser & ConfigParser::operator=(ConfigParser && src) noexcept = default;

void ConfigParser::read(const std::string & file_path) try {
    IniParser parser(file_path);
    ::libdnf5::read(*this, parser);
} catch (const FileSystemError & e) {
    if (e.get_error_code() == ENOENT) {
        libdnf5::throw_with_nested(MissingConfigError(M_("Configuration file \"{}\" not found"), file_path));
    } else {
        libdnf5::throw_with_nested(
            InaccessibleConfigError(M_("Unable to access configuration file \"{}\""), file_path));
    }
} catch (const Error & e) {
    libdnf5::throw_with_nested(InvalidConfigError(M_("Error in configuration file \"{}\""), file_path));
}


bool ConfigParser::add_section(const std::string & section, const std::string & raw_line) {
    if (p_impl->data.find(section) != p_impl->data.end()) {
        return false;
    }
    if (!raw_line.empty()) {
        p_impl->raw_items[section] = raw_line;
    }
    p_impl->data[section] = {};
    return true;
}


bool ConfigParser::add_section(const std::string & section) {
    return add_section(section, "");
}


bool ConfigParser::add_section(std::string && section, std::string && raw_line) {
    if (p_impl->data.find(section) != p_impl->data.end()) {
        return false;
    }
    if (!raw_line.empty()) {
        p_impl->raw_items[section] = std::move(raw_line);
    }
    p_impl->data[std::move(section)] = {};
    return true;
}


bool ConfigParser::add_section(std::string && section) {
    return add_section(std::move(section), "");
}


bool ConfigParser::has_section(const std::string & section) const noexcept {
    return p_impl->data.find(section) != p_impl->data.end();
}


bool ConfigParser::has_option(const std::string & section, const std::string & key) const noexcept {
    auto section_iter = p_impl->data.find(section);
    return section_iter != p_impl->data.end() && section_iter->second.find(key) != section_iter->second.end();
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


void ConfigParser::set_value(
    const std::string & section, const std::string & key, const std::string & value, const std::string & raw_item) {
    auto section_iter = p_impl->data.find(section);
    if (section_iter == p_impl->data.end()) {
        throw ConfigParserSectionNotFoundError(section);
    }
    if (raw_item.empty()) {
        p_impl->raw_items.erase(section + ']' + key);
    } else {
        p_impl->raw_items[section + ']' + key] = raw_item;
    }
    section_iter->second[key] = value;
}


void ConfigParser::set_value(const std::string & section, const std::string & key, const std::string & value) {
    auto raw_iter = p_impl->raw_items.find(section + ']' + key);
    auto raw = create_raw_item(value, raw_iter != p_impl->raw_items.end() ? raw_iter->second : "");
    set_value(section, key, value, raw);
}


void ConfigParser::set_value(
    const std::string & section, std::string && key, std::string && value, std::string && raw_item) {
    auto section_iter = p_impl->data.find(section);
    if (section_iter == p_impl->data.end()) {
        throw ConfigParserSectionNotFoundError(section);
    }
    if (raw_item.empty()) {
        p_impl->raw_items.erase(section + ']' + key);
    } else {
        p_impl->raw_items[section + ']' + key] = std::move(raw_item);
    }
    section_iter->second[std::move(key)] = std::move(value);
}


void ConfigParser::set_value(const std::string & section, std::string && key, std::string && value) {
    auto raw_iter = p_impl->raw_items.find(section + ']' + key);
    auto raw = create_raw_item(value, raw_iter != p_impl->raw_items.end() ? raw_iter->second : "");
    set_value(section, std::move(key), std::move(value), std::move(raw));
}


bool ConfigParser::remove_section(const std::string & section) {
    auto removed = p_impl->data.erase(section) > 0;
    if (removed) {
        p_impl->raw_items.erase(section);
    }
    return removed;
}


bool ConfigParser::remove_option(const std::string & section, const std::string & key) {
    auto section_iter = p_impl->data.find(section);
    if (section_iter == p_impl->data.end()) {
        return false;
    }
    auto removed = section_iter->second.erase(key) > 0;
    if (removed) {
        p_impl->raw_items.erase(section + ']' + key);
    }
    return removed;
}


void ConfigParser::add_comment_line(const std::string & section, const std::string & comment) {
    auto section_iter = p_impl->data.find(section);
    if (section_iter == p_impl->data.end()) {
        throw ConfigParserSectionNotFoundError(section);
    }
    section_iter->second["#" + std::to_string(++p_impl->item_number)] = comment;
}


void ConfigParser::add_comment_line(const std::string & section, std::string && comment) {
    auto section_iter = p_impl->data.find(section);
    if (section_iter == p_impl->data.end()) {
        throw ConfigParserSectionNotFoundError(section);
    }
    section_iter->second["#" + std::to_string(++p_impl->item_number)] = std::move(comment);
}


const std::string & ConfigParser::get_value(const std::string & section, const std::string & key) const {
    auto sect = p_impl->data.find(section);
    if (sect == p_impl->data.end()) {
        throw ConfigParserSectionNotFoundError(section);
    }
    auto key_val = sect->second.find(key);
    if (key_val == sect->second.end()) {
        throw ConfigParserOptionNotFoundError(section, key);
    }
    return key_val->second;
}


const std::string & ConfigParser::get_header() const noexcept {
    return p_impl->header;
}


std::string & ConfigParser::get_header() noexcept {
    return p_impl->header;
}


const ConfigParser::Container & ConfigParser::get_data() const noexcept {
    return p_impl->data;
}


ConfigParser::Container & ConfigParser::get_data() noexcept {
    return p_impl->data;
}


static void write_key_vals(
    utils::fs::File & file,
    const std::string & section,
    const ConfigParser::Container::mapped_type & key_val_map,
    const std::map<std::string, std::string> & raw_items,
    bool & prepend_new_line) {
    for (const auto & key_val : key_val_map) {
        if (prepend_new_line) {
            file.putc('\n');
        }
        auto first = key_val.first[0];
        if (first == '#' || first == ';') {
            file.write(key_val.second);
            prepend_new_line = key_val.second.empty() || key_val.second.back() != '\n';
        } else {
            auto raw_item = raw_items.find(section + ']' + key_val.first);
            if (raw_item != raw_items.end()) {
                file.write(raw_item->second);
                prepend_new_line = raw_item->second.empty() || raw_item->second.back() != '\n';
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
                prepend_new_line = false;
            }
        }
    }
}


static void write_section(
    utils::fs::File & file,
    const std::string & section,
    const ConfigParser::Container::mapped_type & key_val_map,
    const std::map<std::string, std::string> & raw_items,
    bool & prepend_new_line) {
    if (prepend_new_line) {
        file.putc('\n');
    }

    auto raw_item = raw_items.find(section);
    if (raw_item != raw_items.end()) {
        file.write(raw_item->second);
        prepend_new_line = raw_item->second.empty() || raw_item->second.back() != '\n';
    } else {
        file.write(fmt::format("[{}]\n", section));
        prepend_new_line = false;
    }
    write_key_vals(file, section, key_val_map, raw_items, prepend_new_line);
}


void ConfigParser::write(const std::string & file_path, bool append) const {
    utils::fs::File file(file_path, append ? "a" : "w");

    bool prepend_new_line = append;
    if (!p_impl->header.empty()) {
        if (prepend_new_line) {
            file.putc('\n');
        }
        file.write(p_impl->header);
        prepend_new_line = p_impl->header.back() != '\n';
    }
    for (const auto & section : p_impl->data) {
        write_section(file, section.first, section.second, p_impl->raw_items, prepend_new_line);
    }

    // Make sure file ends with newline character '\n'.
    if (prepend_new_line) {
        file.putc('\n');
    }
}


void ConfigParser::write(const std::string & file_path, bool append, const std::string & section) const {
    auto sit = p_impl->data.find(section);
    if (sit == p_impl->data.end()) {
        throw ConfigParserSectionNotFoundError(section);
    }

    utils::fs::File file(file_path, append ? "a" : "w");

    bool prepend_new_line = append;
    write_section(file, sit->first, sit->second, p_impl->raw_items, prepend_new_line);

    // Make sure file ends with newline character '\n'.
    if (prepend_new_line) {
        file.putc('\n');
    }
}

}  // namespace libdnf5
