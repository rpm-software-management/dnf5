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

#ifndef LIBDNF_UTILS_INIPARSER_HPP
#define LIBDNF_UTILS_INIPARSER_HPP

#include "libdnf/common/exception.hpp"

#include <fstream>
#include <memory>
#include <string>

namespace libdnf {

/// @class IniParser
///
/// @brief Simple .INI file parser
///
/// IniParser is lowlevel one pass parser of .ini files designed primary for DNF .ini configuration files.
/// It parses input text to tokens - SECTION, KEY_VAL, COMMENT_LINE, EMPTY_LINE, and END_OF_INPUT.
class IniParser {
public:
    class Exception : public RuntimeError {
    public:
        using RuntimeError::RuntimeError;
        const char * get_domain_name() const noexcept override { return "libdnf::IniParser"; }
        const char * get_name() const noexcept override { return "Exception"; }
        const char * get_description() const noexcept override { return "IniParser exception"; }
    };

    class CantOpenFile : public Exception {
    public:
        using Exception::Exception;
        const char * get_name() const noexcept override { return "CantOpenFile"; }
        const char * get_description() const noexcept override { return "Can't open file"; }
    };

    class MissingSectionHeader : public Exception {
    public:
        using Exception::Exception;
        const char * get_name() const noexcept override { return "MissingSectionHeader"; }
        const char * get_description() const noexcept override { return "Missing section header"; }
    };

    class MissingBracket : public Exception {
    public:
        using Exception::Exception;
        const char * get_name() const noexcept override { return "MissingBracket"; }
        const char * get_description() const noexcept override { return "Missing ']'"; }
    };

    class EmptySectionName : public Exception {
    public:
        using Exception::Exception;
        const char * get_name() const noexcept override { return "EmptySectionName"; }
        const char * get_description() const noexcept override { return "Empty section name"; }
    };

    class TextAfterSection : public Exception {
    public:
        using Exception::Exception;
        const char * get_name() const noexcept override { return "TextAfterSection"; }
        const char * get_description() const noexcept override { return "Text after section"; }
    };

    class IllegalContinuationLine : public Exception {
    public:
        using Exception::Exception;
        const char * get_name() const noexcept override { return "IllegalContinuationLine"; }
        const char * get_description() const noexcept override { return "Illegal continuation line"; }
    };

    class MissingKey : public Exception {
    public:
        using Exception::Exception;
        const char * get_name() const noexcept override { return "MissingKey"; }
        const char * get_description() const noexcept override { return "Missing key"; }
    };

    class MissingEqual : public Exception {
    public:
        using Exception::Exception;
        const char * get_name() const noexcept override { return "MissingEqual"; }
        const char * get_description() const noexcept override { return "Missing '='"; }
    };

    enum class ItemType {
        SECTION,       // [section_name]
        KEY_VAL,       // key = value, (multiline value supported)
        COMMENT_LINE,  // line starting with '#' or ';' character
        EMPTY_LINE,    // zero length or only contains whitespace characters
        END_OF_INPUT
    };

    explicit IniParser(const std::string & file_path);
    explicit IniParser(std::unique_ptr<std::istream> && input_stream);
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
    std::unique_ptr<std::istream> is;
    int line_number;
    std::string section;
    std::string key;
    std::string value;
    std::string raw_item;
    std::string line;
    bool line_ready;
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

}  // namespace libdnf

#endif
