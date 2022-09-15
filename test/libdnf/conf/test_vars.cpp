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


#include "test_vars.hpp"


CPPUNIT_TEST_SUITE_REGISTRATION(VarsTest);

// TODO possibly test the automatically detected vars (they depend on the host)

void VarsTest::setUp() {
    TestCaseFixture::setUp();
    base = get_preconfigured_base();
}

void VarsTest::test_vars() {
    base->get_config().varsdir().set(
        libdnf::Option::Priority::RUNTIME, std::vector<std::string>{PROJECT_SOURCE_DIR "/test/libdnf/conf/data/vars"});
    // Load all variables.
    base->setup();

    CPPUNIT_ASSERT_EQUAL(std::string("foovalue123-bar"), base->get_vars()->substitute("foo$var1-bar"));
    CPPUNIT_ASSERT_EQUAL(
        std::string("$$$value123456-$nn-${nnn}"), base->get_vars()->substitute("$$$${var1}$var2-$nn-${nnn}"));
}

void VarsTest::test_vars_multiple_dirs() {
    base->get_config().varsdir().set(
        libdnf::Option::Priority::RUNTIME,
        std::vector<std::string>{
            PROJECT_SOURCE_DIR "/test/libdnf/conf/data/vars",
            PROJECT_SOURCE_DIR "/test/libdnf/conf/data/vars2",
        });
    // Load all variables.
    base->setup();

    CPPUNIT_ASSERT_EQUAL(std::string("av333bthe answer is here"), base->get_vars()->substitute("a${var1}b${var42}"));
}

void VarsTest::test_vars_env() {
    base->get_config().varsdir().set(
        libdnf::Option::Priority::RUNTIME, std::vector<std::string>{PROJECT_SOURCE_DIR "/test/libdnf/conf/data/vars"});
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
        std::string("foo0-foo1-foo9-testvar1-testvar2-456"),
        base->get_vars()->substitute("${DNF0}-${DNF1}-${DNF9}-${var1}-${var41}-${var2}"));
}
