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

#ifndef LIBDNF_CONFIG_PARSER_HPP
#define LIBDNF_CONFIG_PARSER_HPP

#ifdef LIBDNF_UNSTABLE_API

#include "../utils/PreserveOrderMap.hpp"

#include <istream>
#include <ostream>
#include <map>
#include <memory>
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
* IniParser preserve order of items. Comments and empty lines are kept.
*/
struct ConfigParser {
public:
    typedef PreserveOrderMap<std::string, PreserveOrderMap<std::string, std::string>> Container;

    struct Exception : public std::runtime_error {
        Exception(const std::string & what) : runtime_error(what) {}
    };
    struct CantOpenFile : public Exception {
        CantOpenFile(const std::string & what) : Exception(what) {}
    };
    struct ParsingError : public Exception {
        ParsingError(const std::string & what) : Exception(what) {}
    };
    struct MissingSection : public Exception {
        MissingSection(const std::string & what) : Exception(what) {}
    };
    struct MissingOption : public Exception {
        MissingOption(const std::string & what) : Exception(what) {}
    };

    /**
    * @brief Substitute values in text according to the substitutions map
    *
    * @param text The text for substitution
    * @param substitutions Substitution map
    */
    static void substitute(std::string & text,
        const std::map<std::string, std::string> & substitutions);
    void setSubstitutions(const std::map<std::string, std::string> & substitutions);
    void setSubstitutions(std::map<std::string, std::string> && substitutions);
    const std::map<std::string, std::string> & getSubstitutions() const;
    /**
    * @brief Reads/parse one INI file
    *
    * Can be called repeately for reading/merge more INI files.
    *
    * @param filePath Name (with path) of file to read
    */
    void read(const std::string & filePath);
    /**
    * @brief Reads/parse from istream
    *
    * Can be called repeately for reading/merge more istreams.
    *
    * @param inputStream Stream to read
    */
    void read(std::unique_ptr<std::istream> && inputStream);
    /**
    * @brief Writes all data (all sections) to INI file
    *
    * @param filePath Name (with path) of file to write
    * @param append If true, existent file will be appended, otherwise overwritten
    */
    void write(const std::string & filePath, bool append) const;
    /**
    * @brief Writes one section data to INI file
    *
    * @param filePath Name (with path) of file to write
    * @param append If true, existent file will be appended, otherwise overwritten
    * @param section Section to write
    */
    void write(const std::string & filePath, bool append, const std::string & section) const;
    /**
    * @brief Writes one section data to stream
    *
    * @param outputStream Stream to write
    * @param section Section to write
    */
    void write(std::ostream & outputStream, const std::string & section) const;
    /**
    * @brief Writes all data (all sections) to stream
    *
    * @param outputStream Stream to write
    */
    void write(std::ostream & outputStream) const;
    bool addSection(const std::string & section, const std::string & rawLine);
    bool addSection(const std::string & section);
    bool addSection(std::string && section, std::string && rawLine);
    bool addSection(std::string && section);
    bool hasSection(const std::string & section) const noexcept;
    bool hasOption(const std::string & section, const std::string & key) const noexcept;
    void setValue(const std::string & section, const std::string & key, const std::string & value, const std::string & rawItem);
    void setValue(const std::string & section, const std::string & key, const std::string & value);
    void setValue(const std::string & section, std::string && key, std::string && value, std::string && rawItem);
    void setValue(const std::string & section, std::string && key, std::string && value);
    bool removeSection(const std::string & section);
    bool removeOption(const std::string & section, const std::string & key);
    void addCommentLine(const std::string & section, const std::string & comment);
    void addCommentLine(const std::string & section, std::string && comment);
    const std::string & getValue(const std::string & section, const std::string & key) const;
    std::string getSubstitutedValue(const std::string & section, const std::string & key) const;
    const std::string & getHeader() const noexcept;
    std::string & getHeader() noexcept;
    const Container & getData() const noexcept;
    Container & getData() noexcept;

private:
    std::map<std::string, std::string> substitutions;
    Container data;
    int itemNumber{0};
    std::string header;
    std::map<std::string, std::string> rawItems;
};

inline void ConfigParser::setSubstitutions(const std::map<std::string, std::string> & substitutions)
{
    this->substitutions = substitutions;
}

inline void ConfigParser::setSubstitutions(std::map<std::string, std::string> && substitutions)
{
    this->substitutions = std::move(substitutions);
}

inline const std::map<std::string, std::string> & ConfigParser::getSubstitutions() const
{
    return substitutions;
}

inline bool ConfigParser::addSection(const std::string & section, const std::string & rawLine)
{
    if (data.find(section) != data.end())
        return false;
    if (!rawLine.empty())
        rawItems[section] = rawLine;
    data[section] = {};
    return true;
}

inline bool ConfigParser::addSection(const std::string & section)
{
    return addSection(section, "");
}

inline bool ConfigParser::addSection(std::string && section, std::string && rawLine)
{
    if (data.find(section) != data.end())
        return false;
    if (!rawLine.empty())
        rawItems[section] = std::move(rawLine);
    data[std::move(section)] = {};
    return true;
}

inline bool ConfigParser::addSection(std::string && section)
{
    return addSection(std::move(section), "");
}

inline bool ConfigParser::hasSection(const std::string & section) const noexcept
{
    return data.find(section) != data.end();
}

inline bool ConfigParser::hasOption(const std::string & section, const std::string & key) const noexcept
{
    auto sectionIter = data.find(section);
    return sectionIter != data.end() && sectionIter->second.find(key) != sectionIter->second.end();
}

inline void ConfigParser::setValue(const std::string & section, const std::string & key, const std::string & value, const std::string & rawItem)
{
    auto sectionIter = data.find(section);
    if (sectionIter == data.end())
        throw MissingSection(section);
    if (rawItem.empty())
        rawItems.erase(section + ']' + key);
    else
        rawItems[section + ']' + key] = rawItem;
    sectionIter->second[key] = value;
}

inline void ConfigParser::setValue(const std::string & section, std::string && key, std::string && value, std::string && rawItem)
{
    auto sectionIter = data.find(section);
    if (sectionIter == data.end())
        throw MissingSection(section);
    if (rawItem.empty())
        rawItems.erase(section + ']' + key);
    else
        rawItems[section + ']' + key] = std::move(rawItem);
    sectionIter->second[std::move(key)] = std::move(value);
}

inline bool ConfigParser::removeSection(const std::string & section)
{
    auto removed = data.erase(section) > 0;
    if (removed)
        rawItems.erase(section);
    return removed;
}

inline bool ConfigParser::removeOption(const std::string & section, const std::string & key)
{
    auto sectionIter = data.find(section);
    if (sectionIter == data.end())
        return false;
    auto removed = sectionIter->second.erase(key) > 0;
    if (removed)
        rawItems.erase(section + ']' + key);
    return removed;
}

inline void ConfigParser::addCommentLine(const std::string & section, const std::string & comment)
{
    auto sectionIter = data.find(section);
    if (sectionIter == data.end())
        throw MissingSection(section);
    sectionIter->second["#"+std::to_string(++itemNumber)] = comment;
}

inline void ConfigParser::addCommentLine(const std::string & section, std::string && comment)
{
    auto sectionIter = data.find(section);
    if (sectionIter == data.end())
        throw MissingSection(section);
    sectionIter->second["#"+std::to_string(++itemNumber)] = std::move(comment);
}

inline const std::string & ConfigParser::getHeader() const noexcept
{
    return header;
}

inline std::string & ConfigParser::getHeader() noexcept
{
    return header;
}

inline const ConfigParser::Container & ConfigParser::getData() const noexcept
{
    return data;
}

inline ConfigParser::Container & ConfigParser::getData() noexcept
{
    return data;
}

}

#endif

#endif
