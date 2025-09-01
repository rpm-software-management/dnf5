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


#include "test_option.hpp"

#include "../shared/utils.hpp"

#include <libdnf5/conf/option_bool.hpp>
#include <libdnf5/conf/option_child.hpp>
#include <libdnf5/conf/option_enum.hpp>
#include <libdnf5/conf/option_number.hpp>
#include <libdnf5/conf/option_path.hpp>
#include <libdnf5/conf/option_seconds.hpp>
#include <libdnf5/conf/option_string.hpp>
#include <libdnf5/conf/option_string_list.hpp>


CPPUNIT_TEST_SUITE_REGISTRATION(OptionTest);

using namespace libdnf5;

static const std::vector<std::string> DEFAULT_TRUE_VALUES{"1", "yes", "true", "on"};
static const std::vector<std::string> DEFAULT_FALSE_VALUES{"0", "no", "false", "off"};

static void common_test_option_bool(
    OptionBool & option, const std::vector<std::string> & true_values, const std::vector<std::string> & false_values) {
    CPPUNIT_ASSERT_EQUAL(true_values, option.get_true_values());
    CPPUNIT_ASSERT_EQUAL(false_values, option.get_false_values());

    option.set(Option::Priority::COMMANDLINE, true);
    CPPUNIT_ASSERT_EQUAL(true, option.get_value());
    CPPUNIT_ASSERT_EQUAL(Option::Priority::COMMANDLINE, option.get_priority());

    option.set(Option::Priority::MAINCONFIG, false);
    CPPUNIT_ASSERT_EQUAL(true, option.get_value());
    CPPUNIT_ASSERT_EQUAL(Option::Priority::COMMANDLINE, option.get_priority());

    option.set(Option::Priority::COMMANDLINE, false);
    CPPUNIT_ASSERT_EQUAL(false, option.get_value());
    CPPUNIT_ASSERT_EQUAL(Option::Priority::COMMANDLINE, option.get_priority());

    for (auto & str_val : true_values) {
        option.set(Option::Priority::COMMANDLINE, false);
        option.set(Option::Priority::COMMANDLINE, str_val);
        CPPUNIT_ASSERT_EQUAL(true, option.get_value());
    }

    for (auto & str_val : false_values) {
        option.set(Option::Priority::COMMANDLINE, true);
        option.set(Option::Priority::COMMANDLINE, str_val);
        CPPUNIT_ASSERT_EQUAL(false, option.get_value());
    }
}

void OptionTest::test_options_bool() {
    CPPUNIT_ASSERT_EQUAL(DEFAULT_TRUE_VALUES, OptionBool::get_default_true_values());
    CPPUNIT_ASSERT_EQUAL(DEFAULT_FALSE_VALUES, OptionBool::get_default_false_values());

    OptionBool option(false);
    CPPUNIT_ASSERT_EQUAL(false, option.get_default_value());
    CPPUNIT_ASSERT_EQUAL(false, option.get_value());
    CPPUNIT_ASSERT_EQUAL(Option::Priority::DEFAULT, option.get_priority());

    common_test_option_bool(option, DEFAULT_TRUE_VALUES, DEFAULT_FALSE_VALUES);

    option.set(Option::Priority::COMMANDLINE, std::string("TrUe"));
    CPPUNIT_ASSERT_EQUAL(false, option.get_default_value());
    CPPUNIT_ASSERT_EQUAL(true, option.get_value());

    option.set(Option::Priority::RUNTIME, std::string("FalSe"));
    CPPUNIT_ASSERT_EQUAL(false, option.get_value());
    CPPUNIT_ASSERT_EQUAL(Option::Priority::RUNTIME, option.get_priority());

    CPPUNIT_ASSERT_THROW(option.set(Option::Priority::RUNTIME, std::string("invalid")), OptionInvalidValueError);


    const std::vector<std::string> MY_TRUE_VALUES{"1", "ano", "true", "zap"};
    const std::vector<std::string> MY_FALSE_VALUES{"0", "ne", "false", "vyp"};

    OptionBool option2(true, MY_TRUE_VALUES, MY_FALSE_VALUES);
    CPPUNIT_ASSERT_EQUAL(true, option2.get_default_value());
    CPPUNIT_ASSERT_EQUAL(true, option2.get_value());
    CPPUNIT_ASSERT_EQUAL(Option::Priority::DEFAULT, option2.get_priority());

    common_test_option_bool(option2, MY_TRUE_VALUES, MY_FALSE_VALUES);

    option2.set(Option::Priority::COMMANDLINE, std::string("AnO"));
    CPPUNIT_ASSERT_EQUAL(true, option2.get_value());

    option2.set(Option::Priority::RUNTIME, std::string("nE"));
    CPPUNIT_ASSERT_EQUAL(true, option2.get_default_value());
    CPPUNIT_ASSERT_EQUAL(false, option2.get_value());
    CPPUNIT_ASSERT_EQUAL(Option::Priority::RUNTIME, option2.get_priority());

    CPPUNIT_ASSERT_THROW(option2.set(Option::Priority::RUNTIME, std::string("invalid")), OptionInvalidValueError);

    option.lock("option locked by test_options_bool");
    CPPUNIT_ASSERT_THROW(option.set(Option::Priority::RUNTIME, true), UserAssertionError);
    CPPUNIT_ASSERT_THROW(option.set(Option::Priority::RUNTIME, false), UserAssertionError);
    CPPUNIT_ASSERT_THROW(option.set(Option::Priority::RUNTIME, std::string("true")), UserAssertionError);
    CPPUNIT_ASSERT_THROW(option.set(true), UserAssertionError);
}


void OptionTest::test_options_child() {
    OptionBool oparent(true);
    OptionChild<OptionBool> ochild(oparent);
    CPPUNIT_ASSERT_EQUAL(true, ochild.get_default_value());
    CPPUNIT_ASSERT_EQUAL(true, ochild.get_value());
    CPPUNIT_ASSERT_EQUAL(Option::Priority::DEFAULT, ochild.get_priority());

    oparent.set(Option::Priority::RUNTIME, false);
    CPPUNIT_ASSERT_EQUAL(true, ochild.get_default_value());
    CPPUNIT_ASSERT_EQUAL(false, ochild.get_value());
    CPPUNIT_ASSERT_EQUAL(Option::Priority::RUNTIME, ochild.get_priority());

    ochild.set(Option::Priority::COMMANDLINE, false);
    CPPUNIT_ASSERT_EQUAL(true, ochild.get_default_value());
    CPPUNIT_ASSERT_EQUAL(false, ochild.get_value());
    CPPUNIT_ASSERT_EQUAL(Option::Priority::COMMANDLINE, ochild.get_priority());

    ochild.set(Option::Priority::MAINCONFIG, true);
    CPPUNIT_ASSERT_EQUAL(false, ochild.get_value());
    CPPUNIT_ASSERT_EQUAL(Option::Priority::COMMANDLINE, ochild.get_priority());

    ochild.set(Option::Priority::COMMANDLINE, true);
    CPPUNIT_ASSERT_EQUAL(true, ochild.get_value());
    CPPUNIT_ASSERT_EQUAL(Option::Priority::COMMANDLINE, ochild.get_priority());

    CPPUNIT_ASSERT_THROW(ochild.set(Option::Priority::COMMANDLINE, std::string("invalid")), OptionInvalidValueError);

    ochild.set(Option::Priority::RUNTIME, true);
    ochild.lock("ochild_bool locked by test_option_child");
    CPPUNIT_ASSERT_THROW(ochild.set(Option::Priority::RUNTIME, true), UserAssertionError);
    CPPUNIT_ASSERT_THROW(ochild.set(Option::Priority::RUNTIME, false), UserAssertionError);
    CPPUNIT_ASSERT_THROW(ochild.set(Option::Priority::RUNTIME, std::string("true")), UserAssertionError);
}

void OptionTest::test_options_enum() {
    const std::vector<std::string> ENUM_VALS{"aa", "bb", "cc", "dd", "ee"};

    OptionEnum option("bb", ENUM_VALS);
    CPPUNIT_ASSERT_EQUAL(std::string("bb"), option.get_default_value());
    CPPUNIT_ASSERT_EQUAL(std::string("bb"), option.get_value());
    CPPUNIT_ASSERT_EQUAL(Option::Priority::DEFAULT, option.get_priority());

    for (auto & val : ENUM_VALS) {
        option.set(Option::Priority::COMMANDLINE, val);
        CPPUNIT_ASSERT_EQUAL(val, option.get_value());
    }

    CPPUNIT_ASSERT_THROW(option.set(Option::Priority::RUNTIME, "not_allowed"), OptionValueNotAllowedError);

    option.set(Option::Priority::RUNTIME, "aa");
    option.lock("option locked by test_option_enum");
    CPPUNIT_ASSERT_THROW(option.set(Option::Priority::RUNTIME, "aa"), UserAssertionError);
    CPPUNIT_ASSERT_THROW(option.set(Option::Priority::RUNTIME, "dd"), UserAssertionError);
}

void OptionTest::test_options_number() {
    using NumberType = int32_t;
    constexpr NumberType DEFAULT{10};
    constexpr NumberType MIN{-25};
    constexpr NumberType MAX{50};

    OptionNumber<NumberType> option(DEFAULT, MIN, MAX);
    CPPUNIT_ASSERT_EQUAL(DEFAULT, option.get_default_value());
    CPPUNIT_ASSERT_EQUAL(DEFAULT, option.get_value());
    CPPUNIT_ASSERT_EQUAL(Option::Priority::DEFAULT, option.get_priority());

    for (auto val = MIN; val <= MAX; ++val) {
        option.set(Option::Priority::COMMANDLINE, val);
        CPPUNIT_ASSERT_EQUAL(val, option.get_value());
    }

    CPPUNIT_ASSERT_THROW(option.set(Option::Priority::RUNTIME, MIN - 1), OptionValueNotAllowedError);
    CPPUNIT_ASSERT_THROW(option.set(Option::Priority::RUNTIME, MAX + 1), OptionValueNotAllowedError);

    option.set(Option::Priority::RUNTIME, 1);
    option.lock("option locked by test_option_number");
    CPPUNIT_ASSERT_THROW(option.set(Option::Priority::RUNTIME, 1), UserAssertionError);
    CPPUNIT_ASSERT_THROW(option.set(Option::Priority::RUNTIME, 2), UserAssertionError);
}

void OptionTest::test_options_path() {
    const std::string DEFAULT{"/default_path"};
    constexpr bool EXISTS{false};
    constexpr bool ABSOLUTE{true};

    OptionPath option(DEFAULT, EXISTS, ABSOLUTE);
    CPPUNIT_ASSERT_EQUAL(DEFAULT, option.get_default_value());
    CPPUNIT_ASSERT_EQUAL(DEFAULT, option.get_value());
    CPPUNIT_ASSERT_EQUAL(Option::Priority::DEFAULT, option.get_priority());

    option.set(Option::Priority::RUNTIME, "/path2");
    CPPUNIT_ASSERT_EQUAL(std::string("/path2"), option.get_value());
    CPPUNIT_ASSERT_EQUAL(Option::Priority::RUNTIME, option.get_priority());

    CPPUNIT_ASSERT_THROW(option.set(Option::Priority::RUNTIME, "not_absolute"), OptionValueNotAllowedError);

    option.lock("option locked by test_option_path");
    CPPUNIT_ASSERT_THROW(option.set(Option::Priority::RUNTIME, "/path2"), UserAssertionError);
    CPPUNIT_ASSERT_THROW(option.set(Option::Priority::RUNTIME, "/next"), UserAssertionError);
}


void OptionTest::test_options_seconds() {
    constexpr OptionSeconds::ValueType DEFAULT{10};
    constexpr OptionSeconds::ValueType MIN{8};
    constexpr OptionSeconds::ValueType MAX{14};
    constexpr OptionSeconds::ValueType NEVER{-1};

    OptionSeconds option(DEFAULT, MIN, MAX);
    CPPUNIT_ASSERT_EQUAL(DEFAULT, option.get_default_value());
    CPPUNIT_ASSERT_EQUAL(DEFAULT, option.get_value());
    CPPUNIT_ASSERT_EQUAL(Option::Priority::DEFAULT, option.get_priority());

    for (auto val = MIN; val <= MAX; ++val) {
        option.set(Option::Priority::COMMANDLINE, val);
        CPPUNIT_ASSERT_EQUAL(val, option.get_value());
    }

    CPPUNIT_ASSERT_THROW(option.set(Option::Priority::RUNTIME, MIN - 1), OptionValueNotAllowedError);
    CPPUNIT_ASSERT_THROW(option.set(Option::Priority::RUNTIME, MAX + 1), OptionValueNotAllowedError);

    option.set(Option::Priority::RUNTIME, 12);
    option.lock("option locked by test_option_seconds");
    CPPUNIT_ASSERT_THROW(option.set(Option::Priority::RUNTIME, 12), UserAssertionError);
    CPPUNIT_ASSERT_THROW(option.set(Option::Priority::RUNTIME, 10), UserAssertionError);

    OptionSeconds option2(DEFAULT);
    option2.set(Option::Priority::RUNTIME, "5s");
    CPPUNIT_ASSERT_EQUAL(5, option2.get_value());
    option2.set(Option::Priority::RUNTIME, "5m");
    CPPUNIT_ASSERT_EQUAL(5 * 60, option2.get_value());
    option2.set(Option::Priority::RUNTIME, "5h");
    CPPUNIT_ASSERT_EQUAL(5 * 60 * 60, option2.get_value());
    option2.set(Option::Priority::RUNTIME, "5d");
    CPPUNIT_ASSERT_EQUAL(5 * 60 * 60 * 24, option2.get_value());
    option2.set(Option::Priority::RUNTIME, "5S");
    CPPUNIT_ASSERT_EQUAL(5, option2.get_value());
    option2.set(Option::Priority::RUNTIME, "5M");
    CPPUNIT_ASSERT_EQUAL(5 * 60, option2.get_value());
    option2.set(Option::Priority::RUNTIME, "5H");
    CPPUNIT_ASSERT_EQUAL(5 * 60 * 60, option2.get_value());
    option2.set(Option::Priority::RUNTIME, "5D");
    CPPUNIT_ASSERT_EQUAL(5 * 60 * 60 * 24, option2.get_value());

    option2.set(Option::Priority::RUNTIME, "never");
    CPPUNIT_ASSERT_EQUAL(NEVER, option2.get_value());
    option2.set(Option::Priority::RUNTIME, NEVER);
    CPPUNIT_ASSERT_EQUAL(NEVER, option2.get_value());

    CPPUNIT_ASSERT_THROW(option2.set(Option::Priority::RUNTIME, -2), OptionValueNotAllowedError);
    CPPUNIT_ASSERT_THROW(option2.set(Option::Priority::RUNTIME, "-2"), OptionInvalidValueError);
    CPPUNIT_ASSERT_THROW(option2.set(Option::Priority::RUNTIME, ""), OptionInvalidValueError);
    CPPUNIT_ASSERT_THROW(option2.set(Option::Priority::RUNTIME, "5g"), OptionInvalidValueError);
}

void OptionTest::test_options_string() {
    const std::string DEFAULT{"default"};
    const std::string REGEX{"d.*t"};

    {
        constexpr bool ICASE{false};
        OptionString option(DEFAULT, REGEX, ICASE);
        CPPUNIT_ASSERT_EQUAL(DEFAULT, option.get_default_value());
        CPPUNIT_ASSERT_EQUAL(DEFAULT, option.get_value());
        CPPUNIT_ASSERT_EQUAL(Option::Priority::DEFAULT, option.get_priority());

        option.set(Option::Priority::COMMANDLINE, "donut");
        CPPUNIT_ASSERT_EQUAL(DEFAULT, option.get_default_value());
        CPPUNIT_ASSERT_EQUAL(std::string("donut"), option.get_value());

        // The test is case sensitive (icase = false) - uppercase 'T' is not allowed.
        CPPUNIT_ASSERT_THROW(option.set(Option::Priority::RUNTIME, "donuT"), OptionValueNotAllowedError);

        CPPUNIT_ASSERT_THROW(option.set(Option::Priority::RUNTIME, "drain"), OptionValueNotAllowedError);

        option.lock("option locked by test_option_string");
        CPPUNIT_ASSERT_THROW(option.set(Option::Priority::RUNTIME, "doXXnut"), UserAssertionError);
        CPPUNIT_ASSERT_THROW(option.set(Option::Priority::RUNTIME, "invalid"), UserAssertionError);
    }

    {
        constexpr bool ICASE{true};
        OptionString option(DEFAULT, REGEX, ICASE);
        CPPUNIT_ASSERT_EQUAL(DEFAULT, option.get_default_value());
        CPPUNIT_ASSERT_EQUAL(DEFAULT, option.get_value());
        CPPUNIT_ASSERT_EQUAL(Option::Priority::DEFAULT, option.get_priority());

        option.set(Option::Priority::COMMANDLINE, "donut");
        CPPUNIT_ASSERT_EQUAL(DEFAULT, option.get_default_value());
        CPPUNIT_ASSERT_EQUAL(std::string("donut"), option.get_value());

        // The test is not case sensitive (ICASE = true) - uppercase 'T' is fine.
        option.set(Option::Priority::RUNTIME, "donuT");
        CPPUNIT_ASSERT_EQUAL(std::string("donuT"), option.get_value());
        CPPUNIT_ASSERT_EQUAL(Option::Priority::RUNTIME, option.get_priority());

        CPPUNIT_ASSERT_THROW(option.set(Option::Priority::RUNTIME, "drain"), OptionValueNotAllowedError);

        option.lock("option locked by test_option_string");
        CPPUNIT_ASSERT_THROW(option.set(Option::Priority::RUNTIME, "doXXnut"), UserAssertionError);
        CPPUNIT_ASSERT_THROW(option.set(Option::Priority::RUNTIME, "invalid"), UserAssertionError);
    }
}

void OptionTest::test_options_string_list() {
    const std::vector<std::string> DEFAULT{"dval1X", "dval2X"};
    const std::string REGEX{"[d|c].*X"};

    {
        constexpr bool ICASE{false};
        OptionStringList option(DEFAULT, REGEX, ICASE);
        CPPUNIT_ASSERT_EQUAL(DEFAULT, option.get_default_value());
        CPPUNIT_ASSERT_EQUAL(DEFAULT, option.get_value());
        CPPUNIT_ASSERT_EQUAL(Option::Priority::DEFAULT, option.get_priority());

        option.set(Option::Priority::RUNTIME, std::vector<std::string>{"donutX", "cakeX"});
        CPPUNIT_ASSERT_EQUAL(DEFAULT, option.get_default_value());
        CPPUNIT_ASSERT_EQUAL((std::vector<std::string>{"donutX", "cakeX"}), option.get_value());
        CPPUNIT_ASSERT_EQUAL(Option::Priority::RUNTIME, option.get_priority());

        // The test is case sensitive (icase = false) - lowercase 'x' is not allowed.
        CPPUNIT_ASSERT_THROW(
            option.set(Option::Priority::RUNTIME, std::vector<std::string>{"donutX", "cakex"}),
            OptionValueNotAllowedError);

        CPPUNIT_ASSERT_THROW(
            option.set(Option::Priority::RUNTIME, std::vector<std::string>{"donutX", "drain"}),
            OptionValueNotAllowedError);

        option.set(Option::Priority::RUNTIME, "dfirstX, dsecondX");
        CPPUNIT_ASSERT_EQUAL(DEFAULT, option.get_default_value());
        CPPUNIT_ASSERT_EQUAL((std::vector<std::string>{"dfirstX", "dsecondX"}), option.get_value());

        // The test is case sensitive (icase = false) - uppercase 'D' and lowercase 'x' is not allowed.
        CPPUNIT_ASSERT_THROW(option.set(Option::Priority::RUNTIME, "Dfirstx, DsecondX"), OptionValueNotAllowedError);

        CPPUNIT_ASSERT_THROW(option.set(Option::Priority::RUNTIME, "donutX, drain"), OptionValueNotAllowedError);

        option.lock("option locked by test_option_string_list");
        CPPUNIT_ASSERT_THROW(option.set(Option::Priority::RUNTIME, "doXXnut"), UserAssertionError);
        CPPUNIT_ASSERT_THROW(option.set(Option::Priority::RUNTIME, "invalid"), UserAssertionError);
    }

    {
        constexpr bool ICASE{true};
        OptionStringList option(DEFAULT, REGEX, ICASE);
        CPPUNIT_ASSERT_EQUAL(DEFAULT, option.get_default_value());
        CPPUNIT_ASSERT_EQUAL(DEFAULT, option.get_value());
        CPPUNIT_ASSERT_EQUAL(Option::Priority::DEFAULT, option.get_priority());

        option.set(Option::Priority::RUNTIME, std::vector<std::string>{"donutX", "cakeX"});
        CPPUNIT_ASSERT_EQUAL(DEFAULT, option.get_default_value());
        CPPUNIT_ASSERT_EQUAL((std::vector<std::string>{"donutX", "cakeX"}), option.get_value());
        CPPUNIT_ASSERT_EQUAL(Option::Priority::RUNTIME, option.get_priority());

        // The test is not case sensitive (ICASE = true) - lowercase 'x' is fine.
        option.set(Option::Priority::RUNTIME, std::vector<std::string>{"donutX", "cakex"});
        CPPUNIT_ASSERT_EQUAL(DEFAULT, option.get_default_value());
        CPPUNIT_ASSERT_EQUAL((std::vector<std::string>{"donutX", "cakex"}), option.get_value());
        CPPUNIT_ASSERT_EQUAL(Option::Priority::RUNTIME, option.get_priority());

        CPPUNIT_ASSERT_THROW(
            option.set(Option::Priority::RUNTIME, std::vector<std::string>{"donutX", "drain"}),
            OptionValueNotAllowedError);

        option.set(Option::Priority::RUNTIME, "dfirstX, dsecondX");
        CPPUNIT_ASSERT_EQUAL(DEFAULT, option.get_default_value());
        CPPUNIT_ASSERT_EQUAL((std::vector<std::string>{"dfirstX", "dsecondX"}), option.get_value());

        // The test is not case sensitive (ICASE = true) - uppercase 'D' and lowercase 'x' is fine.
        option.set(Option::Priority::RUNTIME, "Dfirstx, DsecondX");
        CPPUNIT_ASSERT_EQUAL(DEFAULT, option.get_default_value());
        CPPUNIT_ASSERT_EQUAL((std::vector<std::string>{"Dfirstx", "DsecondX"}), option.get_value());

        CPPUNIT_ASSERT_THROW(option.set(Option::Priority::RUNTIME, "donutX, drain"), OptionValueNotAllowedError);

        option.lock("option locked by test_option_string_list");
        CPPUNIT_ASSERT_THROW(option.set(Option::Priority::RUNTIME, "doXXnut"), UserAssertionError);
        CPPUNIT_ASSERT_THROW(option.set(Option::Priority::RUNTIME, "invalid"), UserAssertionError);
    }
}

void OptionTest::test_options_string_list_delimiters() {
    OptionStringList option("Dfirstx , DsecondX");
    CPPUNIT_ASSERT_EQUAL((std::vector<std::string>{"Dfirstx", "DsecondX"}), option.get_value());

    option.set(Option::Priority::RUNTIME, "Dfirstx ,DsecondX");
    CPPUNIT_ASSERT_EQUAL((std::vector<std::string>{"Dfirstx", "DsecondX"}), option.get_value());

    option.set(Option::Priority::RUNTIME, "Dfirstx ,DsecondX     ");
    CPPUNIT_ASSERT_EQUAL((std::vector<std::string>{"Dfirstx", "DsecondX"}), option.get_value());

    option.set(Option::Priority::RUNTIME, "Dfirstx \n,DsecondX");
    CPPUNIT_ASSERT_EQUAL((std::vector<std::string>{"Dfirstx", "DsecondX"}), option.get_value());

    option.set(Option::Priority::RUNTIME, "Dfirstx \n\n,DsecondX\n");
    CPPUNIT_ASSERT_EQUAL((std::vector<std::string>{"Dfirstx", "DsecondX"}), option.get_value());

    option.set(Option::Priority::RUNTIME, "Dfirstx DsecondX");
    CPPUNIT_ASSERT_EQUAL((std::vector<std::string>{"Dfirstx", "DsecondX"}), option.get_value());

    option.set(Option::Priority::RUNTIME, "Dfirstx");
    CPPUNIT_ASSERT_EQUAL((std::vector<std::string>{"Dfirstx"}), option.get_value());

    option.set(Option::Priority::RUNTIME, ", DsecondX");
    CPPUNIT_ASSERT_EQUAL((std::vector<std::string>{"", "DsecondX"}), option.get_value());

    option.set(Option::Priority::RUNTIME, "asdasd, ,DsecondX");
    CPPUNIT_ASSERT_EQUAL((std::vector<std::string>{"asdasd", "DsecondX"}), option.get_value());

    option.set(Option::Priority::RUNTIME, "asdasd,,,DsecondX");
    CPPUNIT_ASSERT_EQUAL((std::vector<std::string>{"asdasd", "DsecondX"}), option.get_value());

    option.set(Option::Priority::RUNTIME, "asdasd,\n,,DsecondX");
    CPPUNIT_ASSERT_EQUAL((std::vector<std::string>{"asdasd", "DsecondX"}), option.get_value());

    option.set(Option::Priority::RUNTIME, "asdasd,\n,   ,DsecondX\n");
    CPPUNIT_ASSERT_EQUAL((std::vector<std::string>{"asdasd", "DsecondX"}), option.get_value());

    option.set(Option::Priority::RUNTIME, "asdasd,\n,   ,DsecondX\nasdasd");
    CPPUNIT_ASSERT_EQUAL((std::vector<std::string>{"asdasd", "DsecondX", "asdasd"}), option.get_value());

    option.set(Option::Priority::RUNTIME, "asdasd,\n,   ,DsecondX asdasd");
    CPPUNIT_ASSERT_EQUAL((std::vector<std::string>{"asdasd", "DsecondX", "asdasd"}), option.get_value());

    option.set(Option::Priority::RUNTIME, "asdasd,\n,   ,DsecondX    asdasd");
    CPPUNIT_ASSERT_EQUAL((std::vector<std::string>{"asdasd", "DsecondX", "asdasd"}), option.get_value());

    option.set(Option::Priority::RUNTIME, "asdasd,\n,   ,DsecondX\n\n\nasdasd");
    CPPUNIT_ASSERT_EQUAL((std::vector<std::string>{"asdasd", "DsecondX", "asdasd"}), option.get_value());

    option.set(Option::Priority::RUNTIME, "  ,  Dfirstx , DsecondX  ");
    CPPUNIT_ASSERT_EQUAL((std::vector<std::string>{"", "Dfirstx", "DsecondX"}), option.get_value());

    option.set(Option::Priority::RUNTIME, "     Dfirstx , DsecondX");
    CPPUNIT_ASSERT_EQUAL((std::vector<std::string>{"Dfirstx", "DsecondX"}), option.get_value());

    option.set(Option::Priority::RUNTIME, "     Dfirstx   , DsecondX");
    CPPUNIT_ASSERT_EQUAL((std::vector<std::string>{"Dfirstx", "DsecondX"}), option.get_value());

    option.set(Option::Priority::RUNTIME, "     Dfirstx\n\n , DsecondX");
    CPPUNIT_ASSERT_EQUAL((std::vector<std::string>{"Dfirstx", "DsecondX"}), option.get_value());
}

void OptionTest::test_options_string_list_custom_delimiters() {
    OptionStringList option({"x?"}, "", false, ",\n");
    CPPUNIT_ASSERT_EQUAL((std::vector<std::string>{"x?"}), option.get_value());

    option.set(Option::Priority::RUNTIME, "x? < 1.0    ");
    CPPUNIT_ASSERT_EQUAL((std::vector<std::string>{"x? < 1.0"}), option.get_value());

    option.set(Option::Priority::RUNTIME, "   x? < 1.0,");
    CPPUNIT_ASSERT_EQUAL((std::vector<std::string>{"x? < 1.0"}), option.get_value());

    option.set(Option::Priority::RUNTIME, "x? < 1.0    ,");
    CPPUNIT_ASSERT_EQUAL((std::vector<std::string>{"x? < 1.0"}), option.get_value());

    option.set(Option::Priority::RUNTIME, "x? < 1.0    ,   ");
    CPPUNIT_ASSERT_EQUAL((std::vector<std::string>{"x? < 1.0"}), option.get_value());


    OptionStringList option3((std::vector<std::string>){"dval1X", "dval2X"}, "", true, ";");
    option3.set(Option::Priority::RUNTIME, "   aa; b;    ccc,;  \n");
    CPPUNIT_ASSERT_EQUAL((std::vector<std::string>{"aa", "b", "ccc,"}), option3.get_value());

    option3.set(Option::Priority::RUNTIME, "     Dfirstx , DsecondX");
    CPPUNIT_ASSERT_EQUAL((std::vector<std::string>{"Dfirstx , DsecondX"}), option3.get_value());

    option3.set(Option::Priority::RUNTIME, "     Dfirstx   ; DsecondX");
    CPPUNIT_ASSERT_EQUAL((std::vector<std::string>{"Dfirstx", "DsecondX"}), option3.get_value());

    option3.set(Option::Priority::RUNTIME, "     Dfirstx\n\n ; DsecondX");
    CPPUNIT_ASSERT_EQUAL((std::vector<std::string>{"Dfirstx", "DsecondX"}), option3.get_value());
}

void OptionTest::test_options_string_set() {
    const OptionStringSet::ValueType initial{"x", "y", "z"};
    OptionStringSet option(initial);
    CPPUNIT_ASSERT(initial == option.get_value());

    option.set(Option::Priority::RUNTIME, "a, b, c");
    CPPUNIT_ASSERT((OptionStringSet::ValueType{"a", "b", "c"}) == option.get_value());

    option.set(Option::Priority::RUNTIME, "a, b, a, a");
    CPPUNIT_ASSERT((OptionStringSet::ValueType{"a", "b"}) == option.get_value());
}

void OptionTest::test_options_list_add() {
    OptionStringSet option("1, 2, 3");
    CPPUNIT_ASSERT((OptionStringSet::ValueType{"1", "2", "3"}) == option.get_value());

    OptionStringSet::ValueType another_set{"4", "5", "6"};
    option.add(libdnf5::Option::Priority::RUNTIME, another_set);
    CPPUNIT_ASSERT((OptionStringSet::ValueType{"1", "2", "3", "4", "5", "6"}) == option.get_value());

    OptionStringSet::ValueType set_with_existing_values{"7", "5", "4"};
    option.add(libdnf5::Option::Priority::RUNTIME, set_with_existing_values);
    CPPUNIT_ASSERT((OptionStringSet::ValueType{"1", "2", "3", "4", "5", "6", "7"}) == option.get_value());
}

void OptionTest::test_options_list_add_item() {
    const OptionStringSet::ValueType initial{"item1"};
    OptionStringSet option(initial);
    CPPUNIT_ASSERT(initial == option.get_value());

    option.add_item(libdnf5::Option::Priority::RUNTIME, "item2");
    CPPUNIT_ASSERT((OptionStringSet::ValueType{"item1", "item2"}) == option.get_value());

    option.add_item(libdnf5::Option::Priority::RUNTIME, "item1");
    CPPUNIT_ASSERT((OptionStringSet::ValueType{"item1", "item2"}) == option.get_value());
}

void OptionTest::test_options_string_append_list() {
    OptionStringAppendList option("Pkg1, Pkg2");
    CPPUNIT_ASSERT_EQUAL((std::vector<std::string>{"Pkg1", "Pkg2"}), option.get_value());
    // setting a new value will append to current value
    option.set(Option::Priority::COMMANDLINE, "Pkg3");
    CPPUNIT_ASSERT_EQUAL((std::vector<std::string>{"Pkg1", "Pkg2", "Pkg3"}), option.get_value());
    // values are evaluated ordered by the priority
    option.set(Option::Priority::MAINCONFIG, "Pkg4");
    CPPUNIT_ASSERT_EQUAL((std::vector<std::string>{"Pkg1", "Pkg2", "Pkg4", "Pkg3"}), option.get_value());
    // I can clear the option using an empty first item (values with higher priority
    // are also appended)
    option.set(Option::Priority::MAINCONFIG, ",Pkg5");
    CPPUNIT_ASSERT_EQUAL((std::vector<std::string>{"Pkg5", "Pkg3"}), option.get_value());
    // emty item on other than first place is skipped and does not clear the value
    option.set(Option::Priority::COMMANDLINE, "Pkg6, ,Pkg7");
    CPPUNIT_ASSERT_EQUAL((std::vector<std::string>{"Pkg5", "Pkg3", "Pkg6", "Pkg7"}), option.get_value());
    // I can clear the option an using empty value
    option.set(Option::Priority::COMMANDLINE, "");
    CPPUNIT_ASSERT_EQUAL((std::vector<std::string>{}), option.get_value());
}
