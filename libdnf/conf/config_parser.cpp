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

#include "ConfigParser.hpp"
#include "../utils/iniparser/iniparser.hpp"

#include <algorithm>
#include <fstream>

namespace libdnf {

void ConfigParser::substitute(std::string & text,
    const std::map<std::string, std::string> & substitutions)
{
    auto start = text.find_first_of("$");
    while (start != text.npos)
    {
        auto variable = start + 1;
        if (variable >= text.length())
            break;
        bool bracket;
        if (text[variable] == '{') {
            bracket = true;
            if (++variable >= text.length())
                break;
        } else
            bracket = false;
        auto it = std::find_if_not(text.begin()+variable, text.end(),
            [](char c){return std::isalnum(c) || c=='_';});
        if (bracket && it == text.end())
            break;
        auto pastVariable = std::distance(text.begin(), it);
        if (bracket && *it != '}') {
            start = text.find_first_of("$", pastVariable);
            continue;
        }
        auto subst = substitutions.find(text.substr(variable, pastVariable - variable));
        if (subst != substitutions.end()) {
            if (bracket)
                ++pastVariable;
            text.replace(start, pastVariable - start, subst->second);
            start = text.find_first_of("$", start + subst->second.length());
        } else {
            start = text.find_first_of("$", pastVariable);
        }
    }
}

static void read(ConfigParser & cfgParser, IniParser & parser)
{
    IniParser::ItemType readedType;
    while ((readedType = parser.next()) != IniParser::ItemType::END_OF_INPUT) {
        auto section = parser.getSection();
        if (readedType == IniParser::ItemType::SECTION) {
            cfgParser.addSection(std::move(section), std::move(parser.getRawItem()));
        }
        else if (readedType == IniParser::ItemType::KEY_VAL) {
            cfgParser.setValue(section, std::move(parser.getKey()), std::move(parser.getValue()), std::move(parser.getRawItem()));
        }
        else if (readedType == IniParser::ItemType::COMMENT_LINE || readedType == IniParser::ItemType::EMPTY_LINE) {
            if (section.empty())
                cfgParser.getHeader() += parser.getRawItem();
            else
                cfgParser.addCommentLine(section, std::move(parser.getRawItem()));
        }
    }
}

void ConfigParser::read(const std::string & filePath)
{
    try {
        IniParser parser(filePath);
        ::libdnf::read(*this, parser);
    } catch (const IniParser::CantOpenFile & e) {
        throw CantOpenFile(e.what());
    } catch (const IniParser::Exception & e) {
        throw ParsingError(e.what() + std::string(" at line ") + std::to_string(e.getLineNumber()));
    }
}

void ConfigParser::read(std::unique_ptr<std::istream> && inputStream)
{
    try {
        IniParser parser(std::move(inputStream));
        ::libdnf::read(*this, parser);
    } catch (const IniParser::CantOpenFile & e) {
        throw CantOpenFile(e.what());
    } catch (const IniParser::Exception & e) {
        throw ParsingError(e.what() + std::string(" at line ") + std::to_string(e.getLineNumber()));
    }
}

static std::string createRawItem(const std::string & value, const std::string & oldRawItem)
{
    auto eqlPos = oldRawItem.find('=');
    if (eqlPos == oldRawItem.npos)
        return "";
    auto valuepos = oldRawItem.find_first_not_of(" \t", eqlPos + 1);
    auto keyAndDelimLength = valuepos != oldRawItem.npos ? valuepos : oldRawItem.length();
    return oldRawItem.substr(0, keyAndDelimLength) + value + '\n';
}

void ConfigParser::setValue(const std::string & section, const std::string & key, const std::string & value)
{
    auto rawIter = rawItems.find(section + ']' + key);
    auto raw = createRawItem(value, rawIter != rawItems.end() ? rawIter->second : "");
    setValue(section, key, value, raw);
}

void ConfigParser::setValue(const std::string & section, std::string && key, std::string && value)
{
    auto rawIter = rawItems.find(section + ']' + key);
    auto raw = createRawItem(value, rawIter != rawItems.end() ? rawIter->second : "");
    setValue(section, std::move(key), std::move(value), std::move(raw));
}

const std::string &
ConfigParser::getValue(const std::string & section, const std::string & key) const
{
    auto sect = data.find(section);
    if (sect == data.end())
        throw MissingSection("OptionReader::getValue(): Missing section " + section);
    auto keyVal = sect->second.find(key);
    if (keyVal == sect->second.end())
        throw MissingOption("OptionReader::getValue(): Missing option " + key +
            " in section " + section);
    return keyVal->second;
}

std::string
ConfigParser::getSubstitutedValue(const std::string & section, const std::string & key) const
{
    auto ret = getValue(section, key);
    substitute(ret, substitutions);
    return ret;
}

static void writeKeyVals(std::ostream & out, const std::string & section, const ConfigParser::Container::mapped_type & keyValMap, const std::map<std::string, std::string> & rawItems)
{
    for (const auto & keyVal : keyValMap) {
        auto first = keyVal.first[0];
        if (first == '#' || first == ';')
            out << keyVal.second;
        else {
            auto rawItem = rawItems.find(section + ']' + keyVal.first);
            if (rawItem != rawItems.end())
                out << rawItem->second;
            else {
                out << keyVal.first << "=";
                for (const auto chr : keyVal.second) {
                    out << chr;
                    if (chr == '\n')
                        out << " ";
                }
                out << "\n";
            }
        }
    }
}

static void writeSection(std::ostream & out, const std::string & section, const ConfigParser::Container::mapped_type & keyValMap, const std::map<std::string, std::string> & rawItems)
{
    auto rawItem = rawItems.find(section);
    if (rawItem != rawItems.end())
        out << rawItem->second;
    else
        out << "[" << section << "]" << "\n";
    writeKeyVals(out, section, keyValMap, rawItems);
}

void ConfigParser::write(const std::string & filePath, bool append) const
{
    std::ofstream ofs;
    ofs.exceptions(std::ofstream::failbit | std::ofstream::badbit);
    ofs.open(filePath, append ? std::ofstream::app : std::ofstream::trunc);
    write(ofs);
}

void ConfigParser::write(const std::string & filePath, bool append, const std::string & section) const
{
    auto sit = data.find(section);
    if (sit == data.end())
        throw MissingSection("ConfigParser::write(): Missing section " + section);
    std::ofstream ofs;
    ofs.exceptions(std::ofstream::failbit | std::ofstream::badbit);
    ofs.open(filePath, append ? std::ofstream::app : std::ofstream::trunc);
    writeSection(ofs, sit->first, sit->second, rawItems);
}

void ConfigParser::write(std::ostream & outputStream) const
{
    outputStream << header;
    for (const auto & section : data) {
        writeSection(outputStream, section.first, section.second, rawItems);
    }
}

void ConfigParser::write(std::ostream & outputStream, const std::string & section) const
{
    auto sit = data.find(section);
    if (sit == data.end())
        throw MissingSection("ConfigParser::write(): Missing section " + section);
    writeSection(outputStream, sit->first, sit->second, rawItems);
}

}
