// Copyright Contributors to the DNF5 project.
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

#include "test_config_parser.hpp"

#include "libdnf5/utils/fs/file.hpp"
#include "libdnf5/utils/fs/temp.hpp"

#include <libdnf5/conf/config_parser.hpp>

#include <string_view>
#include <vector>

CPPUNIT_TEST_SUITE_REGISTRATION(ConfigParserTest);


void ConfigParserTest::setUp() {}


void ConfigParserTest::tearDown() {}


namespace {

enum class ItemType {
    HEADER,        // header - comments and empty lines at the beginning of the file
    SECTION,       // [section_name]
    KEY_VAL,       // key = value, (multiline value supported)
    COMMENT_LINE,  // line starting with '#' or ';' character and empty line
};

// Describes one ConfigParser data item.
struct Item {
    ItemType type;
    const char * section;
    const char * key;
    const char * value;
    const char * raw;
};

// The contents of a simple ini file.
// Contains only sections and keys with values. No comments and empty lines. Default formatting.
constexpr std::string_view simple_ini_content =
    "[section1]\n"
    "key1=value1\n"
    "key2=value2\n"
    "[section3]\n"
    "key2=value2\n"
    "key1=value1\n"
    "[section2]\n"
    "key1=value1\n"
    "key2=value2\n";

// The contents of the ini file extended by comments and an empty line.
// Contains only default formatting.
constexpr std::string_view with_comments_header_content =
    "# Header\n"
    "; Header\n"
    "[section1]\n"
    "key1=value1\n"
    "# Comment 1\n"
    "key2=value2\n"
    "\n"
    "[section3]\n"
    "# Comment 3\n"
    "key2=value2\n"
    "key1=value1\n"
    "# Comment 2\n"
    "[section2]\n"
    "key1=value1\n"
    "key2=value2\n";

// The contents of the ini file containing everything supported including (crazy) custom formatting.
constexpr std::string_view crazy_ini_content =
    "# Header\n"
    "; Header\n"
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

// The contents of the ini file containing everything supported including custom formatting.
// Based on "crazy_ini_content". Intended for test modification of ini file content.
constexpr std::string_view modified_crazy_ini_content =
    "# Header\n"
    "; Header\n"
    "[section1]\n"
    "key1 = value1\n"
    "\n"
    "key3= value3\n"
    "# Comment1\n"
    "key4=value4\n"
    ";Comment2\n"
    "key5    = new value5\n"
    "key6 = two line\n"
    " value1\n"
    "\n"
    "key7 = two line\n"
    " value2\n"
    "key8 = value8\n"
    "\n"
    "nkey10=nvalue10\n"
    "[section3]\n"
    "#Added section\n"
    "nkey1=nvalue1\n"
    ";Another comment line\n"
    "nkey2 = nvalue2\n"
    "nkey3=nvalue3\n";

// Description of the contents of the parsed simple ini file.
const std::vector<Item> simple_items = {
    {ItemType::SECTION, "section1", "", "", "[section1]\n"},
    {ItemType::KEY_VAL, "section1", "key1", "value1", "key1=value1\n"},
    {ItemType::KEY_VAL, "section1", "key2", "value2", "key2=value2\n"},
    {ItemType::SECTION, "section3", "", "", "[section3]\n"},
    {ItemType::KEY_VAL, "section3", "key2", "value2", "key2=value2\n"},
    {ItemType::KEY_VAL, "section3", "key1", "value1", "key1=value1\n"},
    {ItemType::SECTION, "section2", "", "", "[section2]\n"},
    {ItemType::KEY_VAL, "section2", "key1", "value1", "key1=value1\n"},
    {ItemType::KEY_VAL, "section2", "key2", "value2", "key2=value2\n"}};

// Description of the contents of the parsed ini file.
const std::vector<Item> items_with_comments_header = {
    {ItemType::HEADER, "", "", "# Header\n; Header\n", ""},
    {ItemType::SECTION, "section1", "", "", "[section1]\n"},
    {ItemType::KEY_VAL, "section1", "key1", "value1", "key1=value1\n"},
    {ItemType::COMMENT_LINE, "section1", "", "# Comment 1\n", ""},
    {ItemType::KEY_VAL, "section1", "key2", "value2", "key2=value2\n"},
    {ItemType::COMMENT_LINE, "section1", "", "\n", ""},
    {ItemType::SECTION, "section3", "", "", "[section3]\n"},
    {ItemType::COMMENT_LINE, "section3", "", "# Comment 3\n", ""},
    {ItemType::KEY_VAL, "section3", "key2", "value2", "key2=value2\n"},
    {ItemType::KEY_VAL, "section3", "key1", "value1", "key1=value1\n"},
    {ItemType::COMMENT_LINE, "section3", "", "# Comment 2\n", ""},
    {ItemType::SECTION, "section2", "", "", "[section2]\n"},
    {ItemType::KEY_VAL, "section2", "key1", "value1", "key1=value1\n"},
    {ItemType::KEY_VAL, "section2", "key2", "value2", "key2=value2\n"}};

// Description of the contents of the parsed crazy ini file.
const std::vector<Item> crazy_items = {
    {ItemType::HEADER, "", "", "# Header\n; Header\n", ""},
    {ItemType::SECTION, "section1", "", "", "[section1]\n"},
    {ItemType::KEY_VAL, "section1", "key1", "value1", "key1 = value1\n"},
    {ItemType::KEY_VAL, "section1", "key2", "value2", "key2 =value2\n"},
    {ItemType::COMMENT_LINE, "section1", "", "\n", ""},
    {ItemType::KEY_VAL, "section1", "key3", "value3", "key3= value3\n"},
    {ItemType::COMMENT_LINE, "section1", "", "# Comment1\n", ""},
    {ItemType::KEY_VAL, "section1", "key4", "value4", "key4=value4\n"},
    {ItemType::COMMENT_LINE, "section1", "", ";Comment2\n", ""},
    {ItemType::KEY_VAL, "section1", "key5", "value5", "key5    = value5\n"},
    {ItemType::KEY_VAL, "section1", "key6", "two line\nvalue1", "key6 = two line\n value1\n"},
    {ItemType::COMMENT_LINE, "section1", "", "\n", ""},
    {ItemType::KEY_VAL, "section1", "key7", "two line\nvalue2", "key7 = two line\n value2\n"},
    {ItemType::KEY_VAL, "section1", "key8", "value8", "key8 = value8\n"},
    {ItemType::COMMENT_LINE, "section1", "", "\n", ""},
    {ItemType::SECTION, "section2", "", "", "[section2]  # Test section2\n"},
    {ItemType::COMMENT_LINE, "section2", "", "\n", ""},
    {ItemType::KEY_VAL, "section2", "key1", "value1", "key1 = value1\n"}};


// Test creating an ini file from the user description. If `use_raw_values` is `false`,
// the configuration parser ignores the `raw` text and generates its own (default formatting).
void create(const std::vector<Item> & items, std::string_view expected_ini_file_content, bool use_raw_values) {
    libdnf5::utils::fs::TempDir temp_dir("libdnf_test_config_parser");
    std::filesystem::path ini_path = temp_dir.get_path() / "output.ini";

    libdnf5::ConfigParser parser;
    for (auto & [type, section, key, value, raw] : items) {
        switch (type) {
            case ItemType::KEY_VAL:
                if (use_raw_values) {
                    parser.set_value(section, key, value, raw);
                } else {
                    parser.set_value(section, key, value);
                }
                break;
            case ItemType::SECTION:
                if (use_raw_values) {
                    parser.add_section(section, raw);
                } else {
                    parser.add_section(section);
                }
                break;
            case ItemType::COMMENT_LINE:
                parser.add_comment_line(section, value);
                break;
            case ItemType::HEADER:
                parser.get_header() = value;
                break;
        }
    }
    parser.write(ini_path, false);

    auto output_content = libdnf5::utils::fs::File(ini_path, "r").read(expected_ini_file_content.size() + 100);
    CPPUNIT_ASSERT_EQUAL(expected_ini_file_content.size(), output_content.size());
    CPPUNIT_ASSERT_EQUAL(expected_ini_file_content, std::string_view{output_content});
}


// Parse the ini file and check the parsed content.
void parse_and_check_results(std::string_view ini_file_content, const std::vector<Item> & expected_items) {
    // Write the content to be parsed into a temporary ini file.
    libdnf5::utils::fs::TempDir temp_dir("libdnf_test_config_parser");
    std::filesystem::path ini_path = temp_dir.get_path() / "input.ini";
    libdnf5::utils::fs::File(ini_path, "w").write(ini_file_content);

    // parse the ini file
    libdnf5::ConfigParser parser;
    parser.read(ini_path);

    // check the result
    std::size_t idx = 0;
    const auto & header = parser.get_header();
    if (!header.empty()) {
        CPPUNIT_ASSERT_EQUAL(expected_items[idx].type, ItemType::HEADER);
        CPPUNIT_ASSERT_EQUAL(std::string(expected_items[idx].value), header);
        ++idx;
    }
    for (const auto & [section, items] : parser.get_data()) {
        CPPUNIT_ASSERT_LESS(expected_items.size(), idx);
        CPPUNIT_ASSERT_EQUAL(expected_items[idx].type, ItemType::SECTION);
        CPPUNIT_ASSERT_EQUAL(std::string{expected_items[idx].section}, section);
        ++idx;
        for (const auto & [key, value] : items) {
            CPPUNIT_ASSERT_LESS(expected_items.size(), idx);
            ItemType type = key[0] == '#' ? ItemType::COMMENT_LINE : ItemType::KEY_VAL;
            CPPUNIT_ASSERT_EQUAL(expected_items[idx].type, type);
            CPPUNIT_ASSERT_EQUAL(std::string{expected_items[idx].section}, section);
            if (type == ItemType::KEY_VAL) {
                CPPUNIT_ASSERT_EQUAL(std::string{expected_items[idx].key}, key);
            }
            CPPUNIT_ASSERT_EQUAL(std::string{expected_items[idx].value}, value);
            ++idx;
        }
    }
    CPPUNIT_ASSERT_EQUAL(expected_items.size(), idx);

    // Reverse check.
    // Find the items from the description in ConfigParser.
    for (auto & [type, section, key, value, raw] : expected_items) {
        switch (type) {
            case ItemType::KEY_VAL:
                CPPUNIT_ASSERT(parser.has_option(section, key));
                CPPUNIT_ASSERT_EQUAL(std::string{value}, parser.get_value(section, key));
                break;
            case ItemType::SECTION:
                CPPUNIT_ASSERT(parser.has_section(section));
                break;
            case ItemType::COMMENT_LINE:
                break;
            case ItemType::HEADER:
                CPPUNIT_ASSERT_EQUAL(std::string{value}, parser.get_header());
                break;
        }
    }
}


// Parses an ini file and creates a new ini file from the parsed items.
// Checks that the newly created file matches the original.
void test_read_write(std::string_view in_file_content) {
    libdnf5::utils::fs::TempDir temp_dir("libdnf_test_config_parser");
    std::filesystem::path input_ini_path = temp_dir.get_path() / "input.ini";
    std::filesystem::path output_path = temp_dir.get_path() / "output.ini";

    libdnf5::utils::fs::File(input_ini_path, "w").write(in_file_content);

    libdnf5::ConfigParser parser;
    parser.read(input_ini_path);
    parser.write(output_path, false);

    auto output_content = libdnf5::utils::fs::File(output_path, "r").read(in_file_content.size() + 100);

    CPPUNIT_ASSERT_EQUAL(in_file_content.size(), output_content.size());
    CPPUNIT_ASSERT_EQUAL(in_file_content, std::string_view{output_content});
}

}  // namespace


// Creates simple ini file.
// Raw texts are not used, ini file uses default format.
void ConfigParserTest::test_create_simple() {
    create(simple_items, simple_ini_content, false);
}

// Creates ini file with comments and header.
// Raw texts are not used, ini file uses default format.
void ConfigParserTest::test_create_with_comments_header() {
    create(items_with_comments_header, with_comments_header_content, false);
}

// Creates crazy (crazy custom formatting) ini file.
// Uses raw texts, ini file uses custom formatting.
void ConfigParserTest::test_create_crazy() {
    create(crazy_items, crazy_ini_content, true);
}

void ConfigParserTest::test_parse_check_results_simple() {
    parse_and_check_results(simple_ini_content, simple_items);
}

void ConfigParserTest::test_parse_check_results_with_comments_header() {
    parse_and_check_results(with_comments_header_content, items_with_comments_header);
}

void ConfigParserTest::test_parse_check_results_crazy() {
    parse_and_check_results(crazy_ini_content, crazy_items);
}

void ConfigParserTest::test_read_write_simple() {
    test_read_write(simple_ini_content);
}

void ConfigParserTest::test_read_write_with_comments_header() {
    test_read_write(with_comments_header_content);
}

void ConfigParserTest::test_read_write_crazy() {
    test_read_write(crazy_ini_content);
}

// Parses the ini file ("crazy_ini_content"), makes the modifications and creates a new ini file.
// Checks that the newly created file matches the expected one ("modified_crazy_ini_content").
void ConfigParserTest::test_read_modify_write() {
    libdnf5::utils::fs::TempDir temp_dir("libdnf_test_config_parser");
    std::filesystem::path input_ini_path = temp_dir.get_path() / "input.ini";
    std::filesystem::path output_path = temp_dir.get_path() / "output.ini";

    libdnf5::utils::fs::File(input_ini_path, "w").write(crazy_ini_content);

    libdnf5::ConfigParser parser;
    parser.read(input_ini_path);

    parser.set_value("section1", "key5", "new value5");
    parser.add_section("section3");
    parser.add_comment_line("section3", "#Added section");
    parser.remove_option("section1", "key2");
    parser.set_value("section3", "nkey1", "nvalue1");
    parser.add_comment_line("section3", ";Another comment line\n");
    parser.set_value("section3", "nkey2", "nvalue2", "nkey2 = nvalue2\n");
    parser.set_value("section3", "nkey3", "nvalue3");
    parser.remove_section("section2");
    parser.set_value("section1", "nkey10", "nvalue10");

    parser.write(output_path, false);

    auto output_content = libdnf5::utils::fs::File(output_path, "r").read(modified_crazy_ini_content.size() + 100);

    CPPUNIT_ASSERT_EQUAL(modified_crazy_ini_content.size(), output_content.size());
    CPPUNIT_ASSERT_EQUAL(modified_crazy_ini_content, std::string_view{output_content});
}
