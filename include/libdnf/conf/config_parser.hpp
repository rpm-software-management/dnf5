/*
Copyright (C) 2018-2020 Red Hat, Inc.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef LIBDNF_CONF_CONFIG_PARSER_HPP
#define LIBDNF_CONF_CONFIG_PARSER_HPP

#include "libdnf/utils/exception.hpp"
#include "libdnf/utils/preserve_order_map.hpp"

#include <istream>
#include <map>
#include <memory>
#include <ostream>
#include <stdexcept>
#include <string>
#include <utility>

namespace libdnf {

/**
* @class ConfigParser
* 
* @brief Class for parsing dnf/yum .ini configuration files.
*
* ConfigParser is used for parsing files. The class adds support for substitutions.
* User can get both substituded and original parsed values.
* The parsed items are stored into the PreserveOrderMap.
* ConfigParser preserve order of items. Comments and empty lines are kept.
*/
struct ConfigParser {
public:
    using Container = PreserveOrderMap<std::string, PreserveOrderMap<std::string, std::string>>;

    class Exception : public RuntimeError {
    public:
        using RuntimeError::RuntimeError;
        const char * get_domain_name() const noexcept override { return "libdnf::ConfigParser"; }
        const char * get_name() const noexcept override { return "Exception"; }
        const char * get_description() const noexcept override { return "ConfigParser exception"; }
    };
    class SectionNotFound : public Exception {
    public:
        using Exception::Exception;
        const char * get_name() const noexcept override { return "MissingSection"; }
        const char * get_description() const noexcept override { return "Section not found"; }
    };
    class OptionNotFound : public Exception {
    public:
        using Exception::Exception;
        const char * get_name() const noexcept override { return "OptionNotFound"; }
        const char * get_description() const noexcept override { return "Option not found"; }
    };

    /**
    * @brief Substitute values in text according to the substitutions map
    *
    * @param text The text for substitution
    * @param substitutions Substitution map
    */
    static void substitute(std::string & text, const std::map<std::string, std::string> & substitutions);
    void set_substitutions(const std::map<std::string, std::string> & substitutions);
    void set_substitutions(std::map<std::string, std::string> && substitutions);
    const std::map<std::string, std::string> & get_substitutions() const;
    /**
    * @brief Reads/parse one INI file
    *
    * Can be called repeately for reading/merge more INI files.
    *
    * @param file_path Name (with path) of file to read
    */
    void read(const std::string & file_path);
    /**
    * @brief Reads/parse from istream
    *
    * Can be called repeately for reading/merge more istreams.
    *
    * @param inputStream Stream to read
    */
    void read(std::unique_ptr<std::istream> && input_stream);
    /**
    * @brief Writes all data (all sections) to INI file
    *
    * @param file_path Name (with path) of file to write
    * @param append If true, existent file will be appended, otherwise overwritten
    */
    void write(const std::string & file_path, bool append) const;
    /**
    * @brief Writes one section data to INI file
    *
    * @param file_path Name (with path) of file to write
    * @param append If true, existent file will be appended, otherwise overwritten
    * @param section Section to write
    */
    void write(const std::string & file_path, bool append, const std::string & section) const;
    /**
    * @brief Writes one section data to stream
    *
    * @param output_stream Stream to write
    * @param section Section to write
    */
    void write(std::ostream & output_stream, const std::string & section) const;
    /**
    * @brief Writes all data (all sections) to stream
    *
    * @param output_stream Stream to write
    */
    void write(std::ostream & output_stream) const;
    bool add_section(const std::string & section, const std::string & raw_line);
    bool add_section(const std::string & section);
    bool add_section(std::string && section, std::string && raw_line);
    bool add_section(std::string && section);
    bool has_section(const std::string & section) const noexcept;
    bool has_option(const std::string & section, const std::string & key) const noexcept;
    void set_value(
        const std::string & section, const std::string & key, const std::string & value, const std::string & raw_item);
    void set_value(const std::string & section, const std::string & key, const std::string & value);
    void set_value(const std::string & section, std::string && key, std::string && value, std::string && raw_item);
    void set_value(const std::string & section, std::string && key, std::string && value);
    bool remove_section(const std::string & section);
    bool remove_option(const std::string & section, const std::string & key);
    void add_comment_line(const std::string & section, const std::string & comment);
    void add_comment_line(const std::string & section, std::string && comment);
    const std::string & get_value(const std::string & section, const std::string & key) const;
    std::string get_substituted_value(const std::string & section, const std::string & key) const;
    const std::string & get_header() const noexcept;
    std::string & get_header() noexcept;
    const Container & get_data() const noexcept;
    Container & get_data() noexcept;

private:
    std::map<std::string, std::string> substitutions;
    Container data;
    int item_number{0};
    std::string header;
    std::map<std::string, std::string> raw_items;
};

inline void ConfigParser::set_substitutions(const std::map<std::string, std::string> & substitutions) {
    this->substitutions = substitutions;
}

inline void ConfigParser::set_substitutions(std::map<std::string, std::string> && substitutions) {
    this->substitutions = std::move(substitutions);
}

inline const std::map<std::string, std::string> & ConfigParser::get_substitutions() const {
    return substitutions;
}

inline bool ConfigParser::add_section(const std::string & section, const std::string & raw_line) {
    if (data.find(section) != data.end()) {
        return false;
    }
    if (!raw_line.empty()) {
        raw_items[section] = raw_line;
    }
    data[section] = {};
    return true;
}

inline bool ConfigParser::add_section(const std::string & section) {
    return add_section(section, "");
}

inline bool ConfigParser::add_section(std::string && section, std::string && raw_line) {
    if (data.find(section) != data.end()) {
        return false;
    }
    if (!raw_line.empty()) {
        raw_items[section] = std::move(raw_line);
    }
    data[std::move(section)] = {};
    return true;
}

inline bool ConfigParser::add_section(std::string && section) {
    return add_section(std::move(section), "");
}

inline bool ConfigParser::has_section(const std::string & section) const noexcept {
    return data.find(section) != data.end();
}

inline bool ConfigParser::has_option(const std::string & section, const std::string & key) const noexcept {
    auto section_iter = data.find(section);
    return section_iter != data.end() && section_iter->second.find(key) != section_iter->second.end();
}

inline void ConfigParser::set_value(
    const std::string & section, const std::string & key, const std::string & value, const std::string & raw_item) {
    auto section_iter = data.find(section);
    if (section_iter == data.end()) {
        throw SectionNotFound(section);
    }
    if (raw_items.empty()) {
        raw_items.erase(section + ']' + key);
    } else {
        raw_items[section + ']' + key] = raw_item;
    }
    section_iter->second[key] = value;
}

inline void ConfigParser::set_value(
    const std::string & section, std::string && key, std::string && value, std::string && raw_item) {
    auto section_iter = data.find(section);
    if (section_iter == data.end()) {
        throw SectionNotFound(section);
    }
    if (raw_items.empty()) {
        raw_items.erase(section + ']' + key);
    } else {
        raw_items[section + ']' + key] = std::move(raw_item);
    }
    section_iter->second[std::move(key)] = std::move(value);
}

inline bool ConfigParser::remove_section(const std::string & section) {
    auto removed = data.erase(section) > 0;
    if (removed) {
        raw_items.erase(section);
    }
    return removed;
}

inline bool ConfigParser::remove_option(const std::string & section, const std::string & key) {
    auto section_iter = data.find(section);
    if (section_iter == data.end()) {
        return false;
    }
    auto removed = section_iter->second.erase(key) > 0;
    if (removed) {
        raw_items.erase(section + ']' + key);
    }
    return removed;
}

inline void ConfigParser::add_comment_line(const std::string & section, const std::string & comment) {
    auto section_iter = data.find(section);
    if (section_iter == data.end()) {
        throw SectionNotFound(section);
    }
    section_iter->second["#" + std::to_string(++item_number)] = comment;
}

inline void ConfigParser::add_comment_line(const std::string & section, std::string && comment) {
    auto section_iter = data.find(section);
    if (section_iter == data.end()) {
        throw SectionNotFound(section);
    }
    section_iter->second["#" + std::to_string(++item_number)] = std::move(comment);
}

inline const std::string & ConfigParser::get_header() const noexcept {
    return header;
}

inline std::string & ConfigParser::get_header() noexcept {
    return header;
}

inline const ConfigParser::Container & ConfigParser::get_data() const noexcept {
    return data;
}

inline ConfigParser::Container & ConfigParser::get_data() noexcept {
    return data;
}

}  // namespace libdnf

#endif
