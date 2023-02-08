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

#ifndef LIBDNF_TEST_NEVRA_HPP
#define LIBDNF_TEST_NEVRA_HPP


#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>


class NevraTest : public CppUnit::TestCase {
    CPPUNIT_TEST_SUITE(NevraTest);
    CPPUNIT_TEST(test_nevra);
    CPPUNIT_TEST(test_evrcmp);
    CPPUNIT_TEST(test_cmp_nevra);
    CPPUNIT_TEST(test_cmp_naevr);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;
    void tearDown() override;

    void test_nevra();
    void test_evrcmp();
    void test_cmp_nevra();
    void test_cmp_naevr();
};

#endif
