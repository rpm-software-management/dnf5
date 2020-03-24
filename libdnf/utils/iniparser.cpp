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

#include "iniparser.hpp"

constexpr char DELIMITER = '\n';

const char * IniParser::CantOpenFile::what() const noexcept
{
    return "IniParser: Can't open file";
}

const char * IniParser::MissingSectionHeader::what() const noexcept
{
    return "IniParser: Missing section header";
}

const char * IniParser::MissingBracket::what() const noexcept
{
    return "IniParser: Missing ']'";
}

const char * IniParser::EmptySectionName::what() const noexcept
{
    return "IniParser: Empty section name";
}

const char * IniParser::TextAfterSection::what() const noexcept
{
    return "IniParser: Text after section";
}

const char * IniParser::IllegalContinuationLine::what() const noexcept
{
    return "IniParser: Illegal continuation line";
}

const char * IniParser::MissingKey::what() const noexcept
{
    return "IniParser: Missing key";
}

const char * IniParser::MissingEqual::what() const noexcept
{
    return "IniParser: Missing '='";
}

IniParser::IniParser(const std::string & filePath)
: is(new std::ifstream(filePath))
{
    if (!(*is))
        throw CantOpenFile();
    is->exceptions(std::ifstream::badbit);
    lineNumber = 0;
}

IniParser::IniParser(std::unique_ptr<std::istream> && inputStream)
: is(std::move(inputStream))
{
    if (!(*is))
        throw CantOpenFile();
    is->exceptions(std::ifstream::badbit);
    lineNumber = 0;
}

void IniParser::trimValue() noexcept {
    auto end = value.find_last_not_of(DELIMITER);
    if (end != value.npos)
        value.resize(end + 1);
    if (value.length() > 1 &&
        value.front() == value.back() &&
        (value.front() == '\"' || value.front() == '\'')) {
        value.erase(--value.end());
        value.erase(value.begin());
    }
}

IniParser::ItemType IniParser::next()
{
    bool previousLineWithKeyVal = false;
    rawItem.clear();
    while (!line.empty() || !is->eof()) {
        if (line.empty()) {
            std::getline(*is, line, DELIMITER);
            ++lineNumber;
        }

        // remove UTF-8 BOM
        if (lineNumber == 1 && line.length() >= 3 &&
            static_cast<unsigned char>(line[0]) == 0xEF &&
            static_cast<unsigned char>(line[1]) == 0xBB &&
            static_cast<unsigned char>(line[2]) == 0xBF)
            line.erase(0, 3);

        if (line.length() == 0 || line[0] == '#' || line[0] == ';') {// do not support [rR][eE][mM] comment
            if (previousLineWithKeyVal) {
                trimValue();
                return ItemType::KEY_VAL;
            }
            if (line.length() == 0)
                return ItemType::EMPTY_LINE;
            rawItem = line + DELIMITER;
            line.clear();
            return ItemType::COMMENT_LINE;
        }
        auto start = line.find_first_not_of(" \t\r");
        if (start == std::string::npos) {
            if (previousLineWithKeyVal) {
                value += DELIMITER;
                rawItem += line + DELIMITER;
                line.clear();
                continue;
            }
            rawItem = line + DELIMITER;
            line.clear();
            return ItemType::EMPTY_LINE;
        }
        auto end = line.find_last_not_of(" \t\r");

        if (previousLineWithKeyVal && (start == 0 || line[start] == '[')) {
            trimValue();
            return ItemType::KEY_VAL;
        }

        if (line[start] == '[') {
            auto endSectPos = line.find("]", ++start);
            if (endSectPos == line.npos)
                throw MissingBracket(lineNumber);
            else if (endSectPos == start)
                throw EmptySectionName(lineNumber);
            for (auto idx = endSectPos + 1; idx < end; ++idx)
            {
                auto ch = line[idx];
                if (ch == '#' || ch == ';')
                    break;
                if (ch != ' ' && ch != '\t' && ch != '\r')
                    throw TextAfterSection(lineNumber);
            }
            this->section = line.substr(start, endSectPos - start);
            rawItem = line + DELIMITER;
            line.clear();
            return ItemType::SECTION;
        }

        if (section.empty())
            throw MissingSectionHeader(lineNumber);

        if (start > 0) {
            if (!previousLineWithKeyVal)
                throw IllegalContinuationLine(lineNumber);
            value += DELIMITER + line.substr(start, end - start + 1);
            rawItem += line + DELIMITER;
            line.clear();
        } else {
            if (line[start] == '=')
                throw MissingKey(lineNumber);
            auto eqlpos = line.find_first_of("=");
            if (eqlpos == std::string::npos)
                throw MissingEqual(lineNumber);
            auto endkeypos = line.find_last_not_of(" \t", eqlpos - 1);
            auto valuepos = line.find_first_not_of(" \t", eqlpos + 1);
            key = line.substr(start, endkeypos - start + 1);
            if (valuepos != line.npos)
                value = line.substr(valuepos, end - valuepos + 1);
            else
                value.clear();
            previousLineWithKeyVal = true;
            rawItem = line + DELIMITER;
            line.clear();
        }
    }
    trimValue();
    return previousLineWithKeyVal ? ItemType::KEY_VAL : ItemType::END_OF_INPUT;
}
