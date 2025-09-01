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


#include "test_vars.hpp"


CPPUNIT_TEST_SUITE_REGISTRATION(VarsTest);

using namespace std::literals::string_literals;

// TODO possibly test the automatically detected vars (they depend on the host)

void VarsTest::setUp() {
    TestCaseFixture::setUp();
    base = get_preconfigured_base();
}


void VarsTest::test_vars() {
    base->get_config().get_varsdir_option().set(
        std::vector<std::string>{PROJECT_SOURCE_DIR "/test/libdnf5/conf/data/vars"});
    // Load all variables.
    base->setup();

    CPPUNIT_ASSERT_EQUAL("foovalue123-bar"s, base->get_vars()->substitute("foo$var1-bar"));
    CPPUNIT_ASSERT_EQUAL("$$$value123456-$nn-${nnn}"s, base->get_vars()->substitute("$$$${var1}$var2-$nn-${nnn}"));
    CPPUNIT_ASSERT_EQUAL(
        "alternate-default-${nn:+n${nn:-${nnn:}"s,
        base->get_vars()->substitute("${var1:+alternate}-${unset:-default}-${nn:+n${nn:-${nnn:}"));
    CPPUNIT_ASSERT_EQUAL("456"s, base->get_vars()->substitute("${unset:-${var1:+${var2:+$var2}}}"));
}


void VarsTest::test_vars_multiple_dirs() {
    base->get_config().get_varsdir_option().set(std::vector<std::string>{
        PROJECT_SOURCE_DIR "/test/libdnf5/conf/data/vars",
        PROJECT_SOURCE_DIR "/test/libdnf5/conf/data/vars2",
    });
    // Load all variables.
    base->setup();

    CPPUNIT_ASSERT_EQUAL("av333bthe answer is here"s, base->get_vars()->substitute("a${var1}b${var42}"));
}


void VarsTest::test_vars_env() {
    base->get_config().get_varsdir_option().set(
        std::vector<std::string>{PROJECT_SOURCE_DIR "/test/libdnf5/conf/data/vars"});
    // Setting environment variables.
    // Environment variables have higher priority than variables from files.
    setenv("DNF0", "foo0", 1);
    setenv("DNF1", "foo1", 1);
    setenv("DNF9", "foo9", 1);
    setenv("DNF_VAR_var1", "testvar1", 1);
    setenv("DNF_VAR_var41", "testvar2", 1);

    // Load all variables.
    base->setup();

    // The variables var1 and var2 are defined in the files.
    // However, var1 was also an environment variable. The environment has a higher priority.
    CPPUNIT_ASSERT_EQUAL(
        "foo0-foo1-foo9-testvar1-testvar2-456"s,
        base->get_vars()->substitute("${DNF0}-${DNF1}-${DNF9}-${var1}-${var41}-${var2}"));
}


void VarsTest::test_vars_api() {
    base->setup();
    auto vars = base->get_vars();

    CPPUNIT_ASSERT(!vars->contains("test_var1"));

    vars->set("test_var1", "123", libdnf5::Vars::Priority::PLUGIN);
    CPPUNIT_ASSERT(vars->contains("test_var1"));

    {
        auto & [value, prioriry] = vars->get("test_var1");
        CPPUNIT_ASSERT_EQUAL_MESSAGE("value returned by vars->get()", "123"s, value);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("priority returned by vars->get()", libdnf5::Vars::Priority::PLUGIN, prioriry);
    }

    CPPUNIT_ASSERT_EQUAL_MESSAGE("value returned by vars->get_value()", "123"s, vars->get_value("test_var1"));

    CPPUNIT_ASSERT(vars->unset("test_var1"));
    CPPUNIT_ASSERT_MESSAGE("after vars->unset(\"test_var1\")", !vars->contains("test_var1"));
}


void VarsTest::test_vars_api_set_prio() {
    base->setup();
    auto vars = base->get_vars();

    CPPUNIT_ASSERT(!vars->contains("test_var1"));

    // set a new variable
    vars->set("test_var1", "123", libdnf5::Vars::Priority::PLUGIN);
    {
        auto & [value, prioriry] = vars->get("test_var1");
        CPPUNIT_ASSERT_EQUAL_MESSAGE("value returned by vars->get()", "123"s, value);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("priority returned by vars->get()", libdnf5::Vars::Priority::PLUGIN, prioriry);
    }

    // change the value using the same priority
    vars->set("test_var1", "345", libdnf5::Vars::Priority::PLUGIN);
    {
        auto & [value, prioriry] = vars->get("test_var1");
        CPPUNIT_ASSERT_EQUAL_MESSAGE("value returned by vars->get()", "345"s, value);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("priority returned by vars->get()", libdnf5::Vars::Priority::PLUGIN, prioriry);
    }

    // change the value using a higher priority
    vars->set("test_var1", "678", libdnf5::Vars::Priority::COMMANDLINE);
    {
        auto & [value, prioriry] = vars->get("test_var1");
        CPPUNIT_ASSERT_EQUAL_MESSAGE("value returned by vars->get()", "678"s, value);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "priority returned by vars->get()", libdnf5::Vars::Priority::COMMANDLINE, prioriry);
    }

    // changing the value using a lower priority is ignored
    vars->set("test_var1", "ignored_value", libdnf5::Vars::Priority::PLUGIN);
    {
        auto & [value, prioriry] = vars->get("test_var1");
        CPPUNIT_ASSERT_EQUAL_MESSAGE("value returned by vars->get()", "678"s, value);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "priority returned by vars->get()", libdnf5::Vars::Priority::COMMANDLINE, prioriry);
    }

    // change the value using default priority - default is RUNTIME, the highest priority
    vars->set("test_var1", "999");
    {
        auto & [value, prioriry] = vars->get("test_var1");
        CPPUNIT_ASSERT_EQUAL_MESSAGE("value returned by vars->get()", "999"s, value);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("priority returned by vars->get()", libdnf5::Vars::Priority::RUNTIME, prioriry);
    }
}


void VarsTest::test_vars_api_unset_prio() {
    base->setup();
    auto vars = base->get_vars();

    // set a new variable
    vars->set("test_var1", "123", libdnf5::Vars::Priority::COMMANDLINE);
    {
        auto & [value, prioriry] = vars->get("test_var1");
        CPPUNIT_ASSERT_EQUAL_MESSAGE("value returned by vars->get()", "123"s, value);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "priority returned by vars->get()", libdnf5::Vars::Priority::COMMANDLINE, prioriry);
    }

    // removing a variable using a lower priority is ignored
    CPPUNIT_ASSERT(!vars->unset("test_var1", libdnf5::Vars::Priority::PLUGIN));
    CPPUNIT_ASSERT_MESSAGE(
        "after vars->unset(\"test_var1\", libdnf5::Vars::Priority::PLUGIN)", vars->contains("test_var1"));
    {
        auto & [value, prioriry] = vars->get("test_var1");
        CPPUNIT_ASSERT_EQUAL_MESSAGE("value returned by vars->get()", "123"s, value);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "priority returned by vars->get()", libdnf5::Vars::Priority::COMMANDLINE, prioriry);
    }

    // removing a variable using the same priority
    CPPUNIT_ASSERT(vars->unset("test_var1", libdnf5::Vars::Priority::COMMANDLINE));
    CPPUNIT_ASSERT_MESSAGE(
        "after vars->unset(\"test_var1\", libdnf5::Vars::Priority::COMMANDLINE)", !vars->contains("test_var1"));


    // set a new variable
    vars->set("test_var2", "345", libdnf5::Vars::Priority::PLUGIN);
    CPPUNIT_ASSERT(vars->contains("test_var2"));

    // removing a variable using a higher priority
    CPPUNIT_ASSERT(vars->unset("test_var2", libdnf5::Vars::Priority::COMMANDLINE));
    CPPUNIT_ASSERT_MESSAGE(
        "after vars->unset(\"test_var2\", libdnf5::Vars::Priority::COMMANDLINE)", !vars->contains("test_var2"));


    // set a new variable
    vars->set("test_var3", "678", libdnf5::Vars::Priority::PLUGIN);
    CPPUNIT_ASSERT(vars->contains("test_var3"));

    // removing a variable using default priority - default is RUNTIME, the highest priority
    CPPUNIT_ASSERT(vars->unset("test_var3"));
    CPPUNIT_ASSERT_MESSAGE("after vars->unset(\"test_var3\")", !vars->contains("test_var3"));
}


void VarsTest::test_vars_api_releasever() {
    base->setup();
    auto vars = base->get_vars();

    // set the "releasever" variable
    vars->set("releasever", "40.12", libdnf5::Vars::Priority::PLUGIN);

    // check the value the of "releasever" variable
    {
        auto & [value, prioriry] = vars->get("releasever");
        CPPUNIT_ASSERT_EQUAL_MESSAGE("value returned by vars->get()", "40.12"s, value);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("priority returned by vars->get()", libdnf5::Vars::Priority::PLUGIN, prioriry);
    }

    // check the value the of auto-created read-only variables
    {
        auto & [value, prioriry] = vars->get("releasever_major");
        CPPUNIT_ASSERT_EQUAL_MESSAGE("value returned by vars->get()", "40"s, value);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("priority returned by vars->get()", libdnf5::Vars::Priority::PLUGIN, prioriry);
    }
    {
        auto & [value, prioriry] = vars->get("releasever_minor");
        CPPUNIT_ASSERT_EQUAL_MESSAGE("value returned by vars->get()", "12"s, value);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("priority returned by vars->get()", libdnf5::Vars::Priority::PLUGIN, prioriry);
    }

    // the "releasever" variable is read-write
    CPPUNIT_ASSERT(!vars->is_read_only("releasever"));

    // auto-created variables "releasever_major" and "releasever_minor" are read-only
    CPPUNIT_ASSERT(vars->is_read_only("releasever_major"));
    CPPUNIT_ASSERT(vars->is_read_only("releasever_minor"));

    // setting the value of a read-only variable throws exception
    CPPUNIT_ASSERT_THROW(
        vars->set("releasever_major", "40", libdnf5::Vars::Priority::PLUGIN), libdnf5::ReadOnlyVariableError);

    // removing read-only variable throws exception
    CPPUNIT_ASSERT_THROW(
        vars->unset("releasever_major", libdnf5::Vars::Priority::PLUGIN), libdnf5::ReadOnlyVariableError);

    // because the variable "releaver" is read-write, it can be removed
    CPPUNIT_ASSERT(vars->unset("releasever", libdnf5::Vars::Priority::PLUGIN));
    CPPUNIT_ASSERT_MESSAGE("after vars->unset(\"test_var3\")", !vars->contains("releasever"));
}
