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


#ifndef TEST_TUTORIAL_TEST_TUTORIAL_HPP
#define TEST_TUTORIAL_TEST_TUTORIAL_HPP


#include "libdnf/utils/temp.hpp"

#include <cppunit/extensions/HelperMacros.h>


class TutorialTest : public CppUnit::TestCase {
    CPPUNIT_TEST_SUITE(TutorialTest);

    CPPUNIT_TEST(test_create_base);
    CPPUNIT_TEST(test_load_repo);

    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;
    void tearDown() override;

    void test_create_base();
    void test_load_repo();

private:
    std::string baseurl = PROJECT_BINARY_DIR "/test/data/repos-rpm/rpm-repo1/";

    std::unique_ptr<libdnf::utils::TempDir> temp;
    std::string installroot;
};


#endif  // TEST_TUTORIAL_TEST_TUTORIAL_HPP
