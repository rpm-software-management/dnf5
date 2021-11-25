/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "test_iniparser.hpp"

#include "utils/iniparser.hpp"

#include <memory>
#include <sstream>

CPPUNIT_TEST_SUITE_REGISTRATION(IniparserTest);


void IniparserTest::setUp() {}


void IniparserTest::tearDown() {}


using ItemType = libdnf::IniParser::ItemType;

namespace {

struct Item {
    ItemType type;
    const char * section;
    const char * key;
    const char * value;
    const char * raw;
};

}  // namespace

void IniparserTest::test_iniparser() {

    // Source data
    const std::string ini_file_content =
R"**([section1]
key1 = value1
key2 =value2

key3= value3
# Comment1
key4=value4
;Comment2
key5    = value5
key6 = two line
 value1

key7 = two line
 value2
key8 = value8

[section2]  # Test section2

key1 = value1
)**";

    // Expected results from parser
    const Item expected_items[] = {
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
        {ItemType::EMPTY_LINE, "section2", "", "", "" "\n"},
        {ItemType::KEY_VAL, "section2", "key1", "value1", "key1 = value1\n"},
        {ItemType::END_OF_INPUT, "section2", "", "", ""}
    };

    // Parse input
    libdnf::IniParser parser(std::make_unique<std::istringstream>(ini_file_content));
    for (std::size_t idx = 0; idx < sizeof(expected_items)/sizeof(expected_items[0]); ++idx) {
        auto readedType = parser.next();
        CPPUNIT_ASSERT_EQUAL(readedType, expected_items[idx].type);
        CPPUNIT_ASSERT_EQUAL(parser.get_section(), std::string(expected_items[idx].section));
        if (readedType == ItemType::KEY_VAL) {
            CPPUNIT_ASSERT_EQUAL(parser.get_key(), std::string(expected_items[idx].key));
            CPPUNIT_ASSERT_EQUAL(parser.get_value(), std::string(expected_items[idx].value));
        }
        CPPUNIT_ASSERT_EQUAL(parser.get_raw_item(), std::string(expected_items[idx].raw));
    }
}

void IniparserTest::test_iniparser2() {

    // Source data
    const std::string ini_file_content =
R"**(
# Test comment1
# Test comment2
[section1]
key1 = value1
key2 = two line  
    value2
key3 = multi line
    with
    
    value3
key4 = value4
[section2]; Test section2
  
key1 = value1)**";

    // Expected results from parser
    const Item expected_items[] = {
        {ItemType::EMPTY_LINE, "", "", "", "\n"},
        {ItemType::COMMENT_LINE, "", "", "", "# Test comment1\n"},
        {ItemType::COMMENT_LINE, "", "", "", "# Test comment2\n"},
        {ItemType::SECTION, "section1", "", "", "[section1]\n"},
        {ItemType::KEY_VAL, "section1", "key1", "value1", "key1 = value1\n"},
        {ItemType::KEY_VAL, "section1", "key2", "two line\nvalue2", "key2 = two line  \n    value2\n"},
        {ItemType::KEY_VAL, "section1", "key3", "multi line\nwith\n\nvalue3", "key3 = multi line\n    with\n    \n    value3\n"},
        {ItemType::KEY_VAL, "section1", "key4", "value4", "key4 = value4\n"},
        {ItemType::SECTION, "section2", "", "", "[section2]; Test section2\n"},
        {ItemType::EMPTY_LINE, "section2", "", "", "  \n"},
        {ItemType::KEY_VAL, "section2", "key1", "value1", "key1 = value1"},
        {ItemType::END_OF_INPUT, "section2", "", "", ""}
    };

    // Parse input
    libdnf::IniParser parser(std::make_unique<std::istringstream>(ini_file_content));
    for (std::size_t idx = 0; idx < sizeof(expected_items)/sizeof(expected_items[0]); ++idx) {
        auto readedType = parser.next();
        CPPUNIT_ASSERT_EQUAL(readedType, expected_items[idx].type);
        CPPUNIT_ASSERT_EQUAL(parser.get_section(), std::string(expected_items[idx].section));
        if (readedType == ItemType::KEY_VAL) {
            CPPUNIT_ASSERT_EQUAL(parser.get_key(), std::string(expected_items[idx].key));
            CPPUNIT_ASSERT_EQUAL(parser.get_value(), std::string(expected_items[idx].value));
        }
        CPPUNIT_ASSERT_EQUAL(parser.get_raw_item(), std::string(expected_items[idx].raw));
    }
}
