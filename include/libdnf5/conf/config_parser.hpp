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

#ifndef LIBDNF5_CONF_CONFIG_PARSER_HPP
#define LIBDNF5_CONF_CONFIG_PARSER_HPP

#include "libdnf5/common/exception.hpp"
#include "libdnf5/common/preserve_order_map.hpp"

#include <map>
#include <string>


namespace libdnf5 {

/// Error accessing config file other than ENOENT; e.g. we don't have read permission
class InaccessibleConfigError : public Error {
public:
    using Error::Error;
    const char * get_domain_name() const noexcept override { return "libdnf5"; }
    const char * get_name() const noexcept override { return "InaccessibleConfigError"; }
};

/// Configuration file is missing
class MissingConfigError : public Error {
public:
    using Error::Error;
    const char * get_domain_name() const noexcept override { return "libdnf5"; }
    const char * get_name() const noexcept override { return "MissingConfigError"; }
};

/// Configuration file is invalid
class InvalidConfigError : public Error {
public:
    using Error::Error;
    const char * get_domain_name() const noexcept override { return "libdnf5"; }
    const char * get_name() const noexcept override { return "InvalidConfigError"; }
};

class ConfigParserError : public Error {
public:
    using Error::Error;
    const char * get_domain_name() const noexcept override { return "libdnf5"; }
    const char * get_name() const noexcept override { return "ConfigParserError"; }
};

class ConfigParserSectionNotFoundError : public ConfigParserError {
public:
    explicit ConfigParserSectionNotFoundError(const std::string & section);
    const char * get_name() const noexcept override { return "ConfigParserSectionNotFoundError"; }
};

class ConfigParserOptionNotFoundError : public ConfigParserError {
public:
    explicit ConfigParserOptionNotFoundError(const std::string & section, const std::string & option);
    const char * get_name() const noexcept override { return "ConfigParserOptionNotFoundError"; }
};

/**
* @class ConfigParser
*
* @brief Class for parsing dnf/yum .ini configuration files.
*
* ConfigParser is used for parsing files.
* User can get both substituted and original parsed values.
* The parsed items are stored into the PreserveOrderMap.
* ConfigParser preserve order of items. Comments and empty lines are kept.
*/
struct ConfigParser {
public:
    using Container = PreserveOrderMap<std::string, PreserveOrderMap<std::string, std::string>>;

    /**
    * @brief Reads/parse one INI file
    *
    * Can be called repeately for reading/merge more INI files.
    *
    * @param file_path Name (with path) of file to read
    */
    void read(const std::string & file_path);
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
    const std::string & get_header() const noexcept;
    std::string & get_header() noexcept;
    const Container & get_data() const noexcept;
    Container & get_data() noexcept;

private:
    Container data;
    int item_number{0};
    std::string header;
    std::map<std::string, std::string> raw_items;
};

}  // namespace libdnf5

#endif
