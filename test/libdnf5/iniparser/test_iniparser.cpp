// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.

#include "test_iniparser.hpp"

#include "utils/iniparser.hpp"

#include "libdnf5/utils/fs/file.hpp"
#include "libdnf5/utils/fs/temp.hpp"

#include <string_view>
#include <vector>

CPPUNIT_TEST_SUITE_REGISTRATION(IniparserTest);


void IniparserTest::setUp() {}


void IniparserTest::tearDown() {}


using ItemType = libdnf5::IniParser::ItemType;

namespace {

struct Item {
    ItemType type;
    const char * section;
    const char * key;
    const char * value;
    const char * raw;
};

void parse_and_check_results(std::string_view ini_file_content, const std::vector<Item> & expected_items) {
    // write the content to a temporary ini file
    libdnf5::utils::fs::TempDir temp_dir("libdnf_test_iniparser");
    std::filesystem::path ini_path = temp_dir.get_path() / "test.ini";
    libdnf5::utils::fs::File(ini_path, "w").write(ini_file_content);

    // parse the ini file and check the results
    libdnf5::IniParser parser(ini_path);
    for (std::size_t idx = 0; idx < expected_items.size(); ++idx) {
        auto readedType = parser.next();
        CPPUNIT_ASSERT_EQUAL(expected_items[idx].type, readedType);
        CPPUNIT_ASSERT_EQUAL(std::string(expected_items[idx].section), parser.get_section());
        if (readedType == ItemType::KEY_VAL) {
            CPPUNIT_ASSERT_EQUAL(std::string(expected_items[idx].key), parser.get_key());
            CPPUNIT_ASSERT_EQUAL(std::string(expected_items[idx].value), parser.get_value());
        }
        CPPUNIT_ASSERT_EQUAL(std::string(expected_items[idx].raw), parser.get_raw_item());
    }
}

}  // namespace


void IniparserTest::test_iniparser() {
    // Source data
    constexpr std::string_view ini_file_content =
        "[section1]\n"
        "key1 = value1\n"
        "key2 =value2\n"
        "\n"
        "key3= value3\n"
        "# Comment1\n"
        "key4=value4\n"
        ";Comment2\n"
        "key5    = value5\n"
        "key6 = two line\n"
        " value1\n"
        "\n"
        "key7 = two line\n"
        " value2\n"
        "key8 = value8\n"
        "\n"
        "[section2]  # Test section2\n"
        "\n"
        "key1 = value1\n";

    // Expected results from parser
    const std::vector<Item> expected_items = {
        {ItemType::SECTION, "section1", "", "", "[section1]\n"},
        {ItemType::KEY_VAL, "section1", "key1", "value1", "key1 = value1\n"},
        {ItemType::KEY_VAL, "section1", "key2", "value2", "key2 =value2\n"},
        {ItemType::EMPTY_LINE, "section1", "", "", "\n"},
        {ItemType::KEY_VAL, "section1", "key3", "value3", "key3= value3\n"},
        {ItemType::COMMENT_LINE, "section1", "", "", "# Comment1\n"},
        {ItemType::KEY_VAL, "section1", "key4", "value4", "key4=value4\n"},
        {ItemType::COMMENT_LINE, "section1", "", "", ";Comment2\n"},
        {ItemType::KEY_VAL, "section1", "key5", "value5", "key5    = value5\n"},
        {ItemType::KEY_VAL, "section1", "key6", "two line\nvalue1", "key6 = two line\n value1\n"},
        {ItemType::EMPTY_LINE, "section1", "", "", "\n"},
        {ItemType::KEY_VAL, "section1", "key7", "two line\nvalue2", "key7 = two line\n value2\n"},
        {ItemType::KEY_VAL, "section1", "key8", "value8", "key8 = value8\n"},
        {ItemType::EMPTY_LINE, "section1", "", "", "\n"},
        {ItemType::SECTION, "section2", "", "", "[section2]  # Test section2\n"},
        {ItemType::EMPTY_LINE,
         "section2",
         "",
         "",
         ""
         "\n"},
        {ItemType::KEY_VAL, "section2", "key1", "value1", "key1 = value1\n"},
        {ItemType::END_OF_INPUT, "section2", "", "", ""}};

    parse_and_check_results(ini_file_content, expected_items);
}


void IniparserTest::test_iniparser2() {
    // Source data
    constexpr std::string_view ini_file_content =
        "\n"
        "# Test comment1\n"
        "# Test comment2\n"
        "[section1]\n"
        "key1 = value1\n"
        "key2 = two line  \n"
        "    value2\n"
        "key3 = multi line\n"
        "    with\n"
        "    \n"
        "    value3\n"
        "key4 = value4\n"
        "[sect[i]on2]; Test section2\n"
        "  \n"
        "key1 = value1";

    // Expected results from parser
    const std::vector<Item> expected_items = {
        {ItemType::EMPTY_LINE, "", "", "", "\n"},
        {ItemType::COMMENT_LINE, "", "", "", "# Test comment1\n"},
        {ItemType::COMMENT_LINE, "", "", "", "# Test comment2\n"},
        {ItemType::SECTION, "section1", "", "", "[section1]\n"},
        {ItemType::KEY_VAL, "section1", "key1", "value1", "key1 = value1\n"},
        {ItemType::KEY_VAL, "section1", "key2", "two line\nvalue2", "key2 = two line  \n    value2\n"},
        {ItemType::KEY_VAL,
         "section1",
         "key3",
         "multi line\nwith\n\nvalue3",
         "key3 = multi line\n    with\n    \n    value3\n"},
        {ItemType::KEY_VAL, "section1", "key4", "value4", "key4 = value4\n"},
        {ItemType::SECTION, "sect[i]on2", "", "", "[sect[i]on2]; Test section2\n"},
        {ItemType::EMPTY_LINE, "sect[i]on2", "", "", "  \n"},
        {ItemType::KEY_VAL, "sect[i]on2", "key1", "value1", "key1 = value1\n"},
        {ItemType::END_OF_INPUT, "sect[i]on2", "", "", ""}};

    parse_and_check_results(ini_file_content, expected_items);
}


void IniparserTest::test_iniparser_missing_section_header() {
    constexpr std::string_view ini_file_content = "# Test comment1\nkey1 = value1";

    const std::vector<Item> expected_items = {
        {ItemType::COMMENT_LINE, "", "", "", "# Test comment1\n"},
        {ItemType::KEY_VAL, "", "key1", "value1", "key1 = value1\n"}};

    CPPUNIT_ASSERT_THROW(
        parse_and_check_results(ini_file_content, expected_items), libdnf5::IniParserMissingSectionHeaderError);
}


void IniparserTest::test_iniparser_missing_bracket() {
    constexpr std::string_view ini_file_content = "# Test comment1\n[section1\nkey1 = value1\n";

    const std::vector<Item> expected_items = {
        {ItemType::COMMENT_LINE, "", "", "", "# Test comment1\n"},
        {ItemType::SECTION, "section1", "", "", "[section1\n"}};

    CPPUNIT_ASSERT_THROW(
        parse_and_check_results(ini_file_content, expected_items), libdnf5::IniParserMissingBracketError);
}


void IniparserTest::test_iniparser_missing_bracket2() {
    constexpr std::string_view ini_file_content = "# Test comment1\n[sect[ion1]\nkey1 = value1\n";

    const std::vector<Item> expected_items = {
        {ItemType::COMMENT_LINE, "", "", "", "# Test comment1\n"},
        {ItemType::SECTION, "section1", "", "", "[sect[ion1]\n"}};

    CPPUNIT_ASSERT_THROW(
        parse_and_check_results(ini_file_content, expected_items), libdnf5::IniParserMissingBracketError);
}


void IniparserTest::test_iniparser_empty_section_name() {
    constexpr std::string_view ini_file_content = "# Test comment1\n[] # Test\nkey1 = value1";

    const std::vector<Item> expected_items = {
        {ItemType::COMMENT_LINE, "", "", "", "# Test comment1\n"}, {ItemType::SECTION, "", "", "", "[] # Test\n"}};

    CPPUNIT_ASSERT_THROW(
        parse_and_check_results(ini_file_content, expected_items), libdnf5::IniParserEmptySectionNameError);
}


void IniparserTest::test_iniparser_text_after_section() {
    constexpr std::string_view ini_file_content = "# Test comment1\n[section1] Test\nkey1 = value1";

    const std::vector<Item> expected_items = {
        {ItemType::COMMENT_LINE, "", "", "", "# Test comment1\n"},
        {ItemType::SECTION, "section1", "", "", "[section1] Test\n"}};

    CPPUNIT_ASSERT_THROW(
        parse_and_check_results(ini_file_content, expected_items), libdnf5::IniParserTextAfterSectionError);
}


void IniparserTest::test_iniparser_illegal_continuation_line() {
    constexpr std::string_view ini_file_content =
        "# Test comment1\n[section1] # Test\n    key1 = value1\nkey2 = value2";

    const std::vector<Item> expected_items = {
        {ItemType::COMMENT_LINE, "", "", "", "# Test comment1\n"},
        {ItemType::SECTION, "section1", "", "", "[section1] # Test\n"},
        {ItemType::KEY_VAL, "section1", "key1", "value1", "    key1 = value1\n"}};

    CPPUNIT_ASSERT_THROW(
        parse_and_check_results(ini_file_content, expected_items), libdnf5::IniParserIllegalContinuationLineError);
}


void IniparserTest::test_iniparser_missing_key() {
    constexpr std::string_view ini_file_content = "# Test comment1\n[section1] # Test\n= value1\nkey2 = value2";

    const std::vector<Item> expected_items = {
        {ItemType::COMMENT_LINE, "", "", "", "# Test comment1\n"},
        {ItemType::SECTION, "section1", "", "", "[section1] # Test\n"},
        {ItemType::KEY_VAL, "section1", "", "value1", "= value1\n"}};

    CPPUNIT_ASSERT_THROW(parse_and_check_results(ini_file_content, expected_items), libdnf5::IniParserMissingKeyError);
}


void IniparserTest::test_iniparser_missing_equal() {
    constexpr std::string_view ini_file_content = "# Test comment1\n[section1] # Test\nkey1 \nkey2 = value2";

    const std::vector<Item> expected_items = {
        {ItemType::COMMENT_LINE, "", "", "", "# Test comment1\n"},
        {ItemType::SECTION, "section1", "", "", "[section1] # Test\n"},
        {ItemType::KEY_VAL, "section1", "key1", "", "key1 \n"}};

    CPPUNIT_ASSERT_THROW(
        parse_and_check_results(ini_file_content, expected_items), libdnf5::IniParserMissingEqualError);
}
