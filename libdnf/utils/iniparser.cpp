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

#include "utils/iniparser.hpp"

namespace libdnf {

constexpr char DELIMITER = '\n';

IniParser::IniParser(const std::string & file_path) : is(new std::ifstream(file_path)) {
    if (!(*is)) {
        throw CantOpenFile(file_path);
    }
    is->exceptions(std::ifstream::badbit);
    line_number = 0;
    line_ready = false;
}

IniParser::IniParser(std::unique_ptr<std::istream> && input_stream) : is(std::move(input_stream)) {
    if (!(*is)) {
        throw CantOpenFile("bad input stream");
    }
    is->exceptions(std::ifstream::badbit);
    line_number = 0;
    line_ready = false;
}

void IniParser::trim_value() noexcept {
    auto end = value.find_last_not_of(DELIMITER);
    if (end != std::string::npos) {
        value.resize(end + 1);
    }
    if (value.length() > 1 && value.front() == value.back() && (value.front() == '\"' || value.front() == '\'')) {
        value.erase(--value.end());
        value.erase(value.begin());
    }
}

IniParser::ItemType IniParser::next() {
    bool previous_line_with_key_val = false;
    raw_item.clear();
    while (line_ready || !is->eof()) {
        if (!line_ready) {
            std::getline(*is, line, DELIMITER);
            ++line_number;
            line_ready = true;
        }

        // remove UTF-8 BOM (Byte order mark)
        constexpr const char * utf8_bom = "\xEF\xBB\xBF";
        if (line_number == 1 && line.compare(0, 3, utf8_bom) == 0) {
            line.erase(0, 3);
        }

        if (line.empty() || line[0] == '#' || line[0] == ';') {  // do not support [rR][eE][mM] comment
            if (previous_line_with_key_val) {
                trim_value();
                return ItemType::KEY_VAL;
            }
            if (line.empty()) {
                if (is->eof()) {
                    return ItemType::END_OF_INPUT;
                }
                line_ready = false;
                raw_item = DELIMITER;
                return ItemType::EMPTY_LINE;
            }
            raw_item = line;
            if (!is->eof()) {
                raw_item += DELIMITER;
            }
            line_ready = false;
            return ItemType::COMMENT_LINE;
        }
        auto start = line.find_first_not_of(" \t\r");
        if (start == std::string::npos) {
            if (previous_line_with_key_val) {
                value += DELIMITER;
                raw_item += line;
                if (!is->eof()) {
                    raw_item += DELIMITER;
                }
                line_ready = false;
                continue;
            }
            raw_item = line;
            if (!is->eof()) {
                raw_item += DELIMITER;
            }
            line_ready = false;
            return ItemType::EMPTY_LINE;
        }
        auto end = line.find_last_not_of(" \t\r");

        if (previous_line_with_key_val && (start == 0 || line[start] == '[')) {
            trim_value();
            return ItemType::KEY_VAL;
        }

        if (line[start] == '[') {
            auto end_sect_pos = line.find(']', ++start);
            if (end_sect_pos == std::string::npos) {
                throw MissingBracket(std::to_string(line_number));
            }
            if (end_sect_pos == start) {
                throw EmptySectionName(std::to_string(line_number));
            }
            for (auto idx = end_sect_pos + 1; idx < end; ++idx) {
                auto ch = line[idx];
                if (ch == '#' || ch == ';') {
                    break;
                }
                if (ch != ' ' && ch != '\t' && ch != '\r') {
                    throw TextAfterSection(std::to_string(line_number));
                }
            }
            this->section = line.substr(start, end_sect_pos - start);
            raw_item = line;
            if (!is->eof()) {
                raw_item += DELIMITER;
            }
            line_ready = false;
            return ItemType::SECTION;
        }

        if (section.empty()) {
            throw MissingSectionHeader(std::to_string(line_number));
        }

        if (start > 0) {
            if (!previous_line_with_key_val) {
                throw IllegalContinuationLine(std::to_string(line_number));
            }
            value += DELIMITER + line.substr(start, end - start + 1);
            raw_item += line;
            if (!is->eof()) {
                raw_item += DELIMITER;
            }
            line_ready = false;
        } else {
            if (line[start] == '=') {
                throw MissingKey(std::to_string(line_number));
            }
            auto eql_pos = line.find_first_of('=');
            if (eql_pos == std::string::npos) {
                throw MissingEqual(std::to_string(line_number));
            }
            auto endkeypos = line.find_last_not_of(" \t", eql_pos - 1);
            auto valuepos = line.find_first_not_of(" \t", eql_pos + 1);
            key = line.substr(start, endkeypos - start + 1);
            if (valuepos != std::string::npos) {
                value = line.substr(valuepos, end - valuepos + 1);
            } else {
                value.clear();
            }
            previous_line_with_key_val = true;
            raw_item = line;
            if (!is->eof()) {
                raw_item += DELIMITER;
            }
            line_ready = false;
        }
    }
    trim_value();
    return previous_line_with_key_val ? ItemType::KEY_VAL : ItemType::END_OF_INPUT;
}

}  // namespace libdnf
