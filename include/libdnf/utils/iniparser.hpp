/*
 * Copyright (C) 2018 Red Hat, Inc.
 *
 * Licensed under the GNU Lesser General Public License Version 2.1
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef _INIPARSER_HPP
#define _INIPARSER_HPP

#include <exception>
#include <fstream>
#include <memory>
#include <string>

/**
* @class IniParser
*
* @brief Simple .INI file parser
*
* The goal is to be compatible with dnf / yum .ini configuration files.
*/
class IniParser {
public:
    struct Exception : public std::exception {
        Exception(int lineNumber) : lineNumber(lineNumber) {}
        Exception() : lineNumber(0) {}
        int getLineNumber() const noexcept { return lineNumber; }
    protected:
        int lineNumber;
    };
    struct CantOpenFile : public Exception {
        CantOpenFile() {}
        const char * what() const noexcept override;
    };
    struct MissingSectionHeader : public Exception {
        MissingSectionHeader(int lineNumber) : Exception(lineNumber) {}
        const char * what() const noexcept override;
    };
    struct MissingBracket : public Exception {
        MissingBracket(int lineNumber) : Exception(lineNumber) {}
        const char * what() const noexcept override;
    };
    struct EmptySectionName : public Exception {
        EmptySectionName(int lineNumber) : Exception(lineNumber) {}
        const char * what() const noexcept override;
    };
    struct TextAfterSection : public Exception {
        TextAfterSection(int lineNumber) : Exception(lineNumber) {}
        const char * what() const noexcept override;
    };
    struct IllegalContinuationLine : public Exception {
        IllegalContinuationLine(int lineNumber) : Exception(lineNumber) {}
        const char * what() const noexcept override;
    };
    struct MissingKey : public Exception {
        MissingKey(int lineNumber) : Exception(lineNumber) {}
        const char * what() const noexcept override;
    };
    struct MissingEqual : public Exception {
        MissingEqual(int lineNumber) : Exception(lineNumber) {}
        const char * what() const noexcept override;
    };

    enum class ItemType {
        SECTION,        // [section_name]
        KEY_VAL,        // key = value, (multiline value supported)
        COMMENT_LINE,   // line starting with '#' or ';' character
        EMPTY_LINE,     // zero length or only contains whitespace characters
        END_OF_INPUT
    };

    IniParser(const std::string & filePath);
    IniParser(std::unique_ptr<std::istream> && inputStream);
    /**
    * @brief Parse one item from input file
    *
    * Returns type of parsed item. Parsed values can be obtained by methods
    * getSection(), getKey(), getValue().
    *
    * @return IniParser::ItemType Type of parsed value
    */
    ItemType next();
    const std::string & getSection() const noexcept;
    const std::string & getKey() const noexcept;
    std::string & getKey() noexcept;
    const std::string & getValue() const noexcept;
    std::string & getValue() noexcept;
    const std::string & getRawItem() const noexcept;
    std::string & getRawItem() noexcept;
    const std::string & getLine() const noexcept;
    void clearLine() noexcept;
    void trimValue() noexcept;

private:
    std::unique_ptr<std::istream> is;
    int lineNumber;
    std::string section;
    std::string key;
    std::string value;
    std::string rawItem;
    std::string line;
};

inline const std::string & IniParser::getSection() const noexcept { return section; }
inline const std::string & IniParser::getKey() const noexcept { return key; }
inline std::string & IniParser::getKey() noexcept { return key; }
inline const std::string & IniParser::getValue() const noexcept { return value; }
inline std::string & IniParser::getValue() noexcept { return value; }
inline const std::string & IniParser::getRawItem() const noexcept { return rawItem; }
inline std::string & IniParser::getRawItem() noexcept { return rawItem; }
inline const std::string & IniParser::getLine() const noexcept { return line; }
inline void IniParser::clearLine() noexcept { line.clear(); }

#endif
