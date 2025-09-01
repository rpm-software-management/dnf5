// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 2.1 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.

#ifndef LIBDNF5_UTILS_INIPARSER_HPP
#define LIBDNF5_UTILS_INIPARSER_HPP

#include "libdnf5/common/exception.hpp"
#include "libdnf5/utils/fs/file.hpp"

#include <memory>
#include <string>


namespace libdnf5 {

class IniParserError : public Error {
public:
    using Error::Error;
    const char * get_domain_name() const noexcept override { return "libdnf5"; }
    const char * get_name() const noexcept override { return "IniParserError"; }
};

class IniParserMissingSectionHeaderError : public IniParserError {
public:
    using IniParserError::IniParserError;
    const char * get_name() const noexcept override { return "IniParserMissingSectionHeaderError"; }
};

class IniParserMissingBracketError : public IniParserError {
public:
    using IniParserError::IniParserError;
    const char * get_name() const noexcept override { return "IniParserMissingBracketError"; }
};

class IniParserEmptySectionNameError : public IniParserError {
public:
    using IniParserError::IniParserError;
    const char * get_name() const noexcept override { return "IniParserEmptySectionNameError"; }
};

class IniParserTextAfterSectionError : public IniParserError {
public:
    using IniParserError::IniParserError;
    const char * get_name() const noexcept override { return "IniParserTextAfterSectionError"; }
};

class IniParserIllegalContinuationLineError : public IniParserError {
public:
    using IniParserError::IniParserError;
    const char * get_name() const noexcept override { return "IniParserIllegalContinuationLineError"; }
};

class IniParserMissingKeyError : public IniParserError {
public:
    using IniParserError::IniParserError;
    const char * get_name() const noexcept override { return "IniParserMissingKeyError"; }
};

class IniParserMissingEqualError : public IniParserError {
public:
    using IniParserError::IniParserError;
    const char * get_name() const noexcept override { return "IniParserMissingEqualError"; }
};

/// @class IniParser
///
/// @brief Simple .INI file parser
///
/// IniParser is lowlevel one pass parser of .ini files designed primary for DNF .ini configuration files.
/// It parses input text to tokens - SECTION, KEY_VAL, COMMENT_LINE, EMPTY_LINE, and END_OF_INPUT.
class IniParser {
public:
    enum class ItemType {
        SECTION,       // [section_name]
        KEY_VAL,       // key = value, (multiline value supported)
        COMMENT_LINE,  // line starting with '#' or ';' character
        EMPTY_LINE,    // zero length or only contains whitespace characters
        END_OF_INPUT
    };

    explicit IniParser(const std::string & file_path);
    /**
    * @brief Parse one item from input file
    *
    * Returns type of parsed item. Parsed values can be obtained by methods
    * getSection(), getKey(), getValue().
    *
    * @return IniParser::ItemType Type of parsed value
    */
    ItemType next();
    const std::string & get_section() const noexcept;
    const std::string & get_key() const noexcept;
    std::string & get_key() noexcept;
    const std::string & get_value() const noexcept;
    std::string & get_value() noexcept;
    const std::string & get_raw_item() const noexcept;
    std::string & get_raw_item() noexcept;
    const std::string & get_line() const noexcept;
    void clear_line() noexcept;
    void trim_value() noexcept;

private:
    utils::fs::File file;
    int line_number{0};
    std::string section;
    std::string key;
    std::string value;
    std::string raw_item;
    std::string line;
    bool line_ready{false};
};

inline const std::string & IniParser::get_section() const noexcept {
    return section;
}
inline const std::string & IniParser::get_key() const noexcept {
    return key;
}
inline std::string & IniParser::get_key() noexcept {
    return key;
}
inline const std::string & IniParser::get_value() const noexcept {
    return value;
}
inline std::string & IniParser::get_value() noexcept {
    return value;
}
inline const std::string & IniParser::get_raw_item() const noexcept {
    return raw_item;
}
inline std::string & IniParser::get_raw_item() noexcept {
    return raw_item;
}
inline const std::string & IniParser::get_line() const noexcept {
    return line;
}
inline void IniParser::clear_line() noexcept {
    line.clear();
}

}  // namespace libdnf5

#endif
