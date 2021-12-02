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


#include "test_option.hpp"

#include "libdnf/conf/option_bool.hpp"
#include "libdnf/conf/option_child.hpp"
#include "libdnf/conf/option_enum.hpp"
#include "libdnf/conf/option_number.hpp"
#include "libdnf/conf/option_path.hpp"
#include "libdnf/conf/option_seconds.hpp"
#include "libdnf/conf/option_string.hpp"
#include "libdnf/conf/option_string_list.hpp"

#include "utils.hpp"


CPPUNIT_TEST_SUITE_REGISTRATION(OptionTest);

using namespace libdnf;

static const std::vector<std::string> DEFAULT_TRUE_VALUES{"1", "yes", "true", "on"};
static const std::vector<std::string> DEFAULT_FALSE_VALUES{"0", "no", "false", "off"};

static void common_test_option_bool(OptionBool & option, const std::vector<std::string> & true_values, const std::vector<std::string> & false_values) {
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

    CPPUNIT_ASSERT_THROW(option.set(Option::Priority::RUNTIME, std::string("invalid")), Option::InvalidValue);


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

    CPPUNIT_ASSERT_THROW(option2.set(Option::Priority::RUNTIME, std::string("invalid")), Option::InvalidValue);

    option.lock("option locked by test_options_bool");
    CPPUNIT_ASSERT_THROW(option.set(Option::Priority::RUNTIME, true), AssertionError);
    CPPUNIT_ASSERT_THROW(option.set(Option::Priority::RUNTIME, false), AssertionError);
    CPPUNIT_ASSERT_THROW(option.set(Option::Priority::RUNTIME, std::string("true")), AssertionError);
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

    CPPUNIT_ASSERT_THROW(ochild.set(Option::Priority::COMMANDLINE, std::string("invalid")), Option::InvalidValue);

    ochild.set(Option::Priority::RUNTIME, true);
    ochild.lock("ochild_bool locked by test_option_child");
    CPPUNIT_ASSERT_THROW(ochild.set(Option::Priority::RUNTIME, true), AssertionError);
    CPPUNIT_ASSERT_THROW(ochild.set(Option::Priority::RUNTIME, false), AssertionError);
    CPPUNIT_ASSERT_THROW(ochild.set(Option::Priority::RUNTIME, std::string("true")), AssertionError);
}

void OptionTest::test_options_enum() {
    const std::vector<std::string> ENUM_VALS{"aa", "bb", "cc", "dd", "ee"};

    OptionEnum<std::string> option("bb", ENUM_VALS);
    CPPUNIT_ASSERT_EQUAL(std::string("bb"), option.get_default_value());
    CPPUNIT_ASSERT_EQUAL(std::string("bb"), option.get_value());
    CPPUNIT_ASSERT_EQUAL(Option::Priority::DEFAULT, option.get_priority());

    for (auto & val : ENUM_VALS) {
        option.set(Option::Priority::COMMANDLINE, val);
        CPPUNIT_ASSERT_EQUAL(val, option.get_value());
    }

    CPPUNIT_ASSERT_THROW(option.set(Option::Priority::RUNTIME, "not_allowed"), OptionEnum<std::string>::NotAllowedValue);

    option.set(Option::Priority::RUNTIME, "aa");
    option.lock("option locked by test_option_enum");
    CPPUNIT_ASSERT_THROW(option.set(Option::Priority::RUNTIME, "aa"), AssertionError);
    CPPUNIT_ASSERT_THROW(option.set(Option::Priority::RUNTIME, "dd"), AssertionError);
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

    CPPUNIT_ASSERT_THROW(option.set(Option::Priority::RUNTIME, MIN - 1), OptionNumber<NumberType>::NotAllowedValue);
    CPPUNIT_ASSERT_THROW(option.set(Option::Priority::RUNTIME, MAX + 1), OptionNumber<NumberType>::NotAllowedValue);

    option.set(Option::Priority::RUNTIME, 1);
    option.lock("option locked by test_option_number");
    CPPUNIT_ASSERT_THROW(option.set(Option::Priority::RUNTIME, 1), AssertionError);
    CPPUNIT_ASSERT_THROW(option.set(Option::Priority::RUNTIME, 2), AssertionError);
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

    CPPUNIT_ASSERT_THROW(option.set(Option::Priority::RUNTIME, "not_absolute"), OptionPath::NotAllowedValue);

    option.lock("option locked by test_option_path");
    CPPUNIT_ASSERT_THROW(option.set(Option::Priority::RUNTIME, "/path2"), AssertionError);
    CPPUNIT_ASSERT_THROW(option.set(Option::Priority::RUNTIME, "/next"), AssertionError);
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

    CPPUNIT_ASSERT_THROW(option.set(Option::Priority::RUNTIME, MIN - 1), OptionSeconds::NotAllowedValue);
    CPPUNIT_ASSERT_THROW(option.set(Option::Priority::RUNTIME, MAX + 1), OptionSeconds::NotAllowedValue);

    option.set(Option::Priority::RUNTIME, 12);
    option.lock("option locked by test_option_seconds");
    CPPUNIT_ASSERT_THROW(option.set(Option::Priority::RUNTIME, 12), AssertionError);
    CPPUNIT_ASSERT_THROW(option.set(Option::Priority::RUNTIME, 10), AssertionError);

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

    CPPUNIT_ASSERT_THROW(option2.set(Option::Priority::RUNTIME, -2), OptionSeconds::NotAllowedValue);
    CPPUNIT_ASSERT_THROW(option2.set(Option::Priority::RUNTIME, "-2"), OptionSeconds::InvalidValue);
    CPPUNIT_ASSERT_THROW(option2.set(Option::Priority::RUNTIME, ""), OptionSeconds::InvalidValue);
    CPPUNIT_ASSERT_THROW(option2.set(Option::Priority::RUNTIME, "5g"), OptionSeconds::UnknownUnit);
}

void OptionTest::test_options_string() {
    const std::string DEFAULT{"default"};
    const std::string REGEX{"d.*t"};
    constexpr bool ICASE{true};

    OptionString option(DEFAULT, REGEX, ICASE);
    CPPUNIT_ASSERT_EQUAL(DEFAULT, option.get_default_value());
    CPPUNIT_ASSERT_EQUAL(DEFAULT, option.get_value());
    CPPUNIT_ASSERT_EQUAL(Option::Priority::DEFAULT, option.get_priority());

    option.set(Option::Priority::COMMANDLINE, "donut");
    CPPUNIT_ASSERT_EQUAL(DEFAULT, option.get_default_value());
    CPPUNIT_ASSERT_EQUAL(std::string("donut"), option.get_value());

    option.set(Option::Priority::RUNTIME, "do iT");
    CPPUNIT_ASSERT_EQUAL(std::string("do iT"), option.get_value());
    CPPUNIT_ASSERT_EQUAL(Option::Priority::RUNTIME, option.get_priority());

    CPPUNIT_ASSERT_THROW(option.set(Option::Priority::RUNTIME, "drain"), OptionString::NotAllowedValue);

    option.lock("option locked by test_option_string");
    CPPUNIT_ASSERT_THROW(option.set(Option::Priority::RUNTIME, "do iT"), AssertionError);
    CPPUNIT_ASSERT_THROW(option.set(Option::Priority::RUNTIME, "doXXnut"), AssertionError);
}

void OptionTest::test_options_string_list() {
    const std::vector<std::string> DEFAULT{"dval1X", "dval2X"};
    const std::string REGEX{"[d|c].*X"};
    constexpr bool ICASE{true};

    OptionStringList option(DEFAULT, REGEX, ICASE);
    CPPUNIT_ASSERT_EQUAL(DEFAULT, option.get_default_value());
    CPPUNIT_ASSERT_EQUAL(DEFAULT, option.get_value());
    CPPUNIT_ASSERT_EQUAL(Option::Priority::DEFAULT, option.get_priority());

    option.set(Option::Priority::RUNTIME, std::vector<std::string>{"donutX", "cakex"});
    CPPUNIT_ASSERT_EQUAL(DEFAULT, option.get_default_value());
    CPPUNIT_ASSERT_EQUAL((std::vector<std::string>{"donutX", "cakex"}), option.get_value());
    CPPUNIT_ASSERT_EQUAL(Option::Priority::RUNTIME, option.get_priority());

    option.set(Option::Priority::RUNTIME, "Dfirstx, DsecondX");
    CPPUNIT_ASSERT_EQUAL(DEFAULT, option.get_default_value());
    CPPUNIT_ASSERT_EQUAL((std::vector<std::string>{"Dfirstx", "DsecondX"}), option.get_value());

    CPPUNIT_ASSERT_THROW(option.set(Option::Priority::RUNTIME, std::vector<std::string>{"donutX", "drain"}), OptionStringList::NotAllowedValue);
    CPPUNIT_ASSERT_THROW(option.set(Option::Priority::RUNTIME, "donutX, drain"), OptionStringList::NotAllowedValue);

    option.lock("option locked by test_option_string_list");
    CPPUNIT_ASSERT_THROW(option.set(Option::Priority::RUNTIME, "do iT"), AssertionError);
    CPPUNIT_ASSERT_THROW(option.set(Option::Priority::RUNTIME, "doXXnut"), AssertionError);
}
