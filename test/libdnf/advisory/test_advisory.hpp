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


#ifndef TEST_LIBDNF_ADVISORY_ADVISORY_HPP
#define TEST_LIBDNF_ADVISORY_ADVISORY_HPP


#include "../support.hpp"

#include "libdnf/advisory/advisory_collection.hpp"
#include "libdnf/advisory/advisory_sack.hpp"

#include <cppunit/extensions/HelperMacros.h>

class AdvisoryAdvisoryTest : public LibdnfTestCase {
    CPPUNIT_TEST_SUITE(AdvisoryAdvisoryTest);

    CPPUNIT_TEST(test_get_name);
    CPPUNIT_TEST(test_get_type);
    CPPUNIT_TEST(test_get_type_cstring);
    CPPUNIT_TEST(test_get_severity);
    CPPUNIT_TEST(test_get_references);
    CPPUNIT_TEST(test_get_collections);

    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;

    void test_get_name();
    void test_get_type();
    void test_get_type_cstring();
    void test_get_severity();
    //void test_filter_package();

    void test_get_references();
    void test_get_collections();
};


#endif  // TEST_LIBDNF_ADVISORY_ADVISORY_HPP
