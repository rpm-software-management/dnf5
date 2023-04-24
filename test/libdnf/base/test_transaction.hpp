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


#ifndef TEST_LIBDNF_BASE_TRANSACTION_HPP
#define TEST_LIBDNF_BASE_TRANSACTION_HPP


#include "../shared/base_test_case.hpp"

#include <cppunit/extensions/HelperMacros.h>


class BaseTransactionTest : public BaseTestCase {
    CPPUNIT_TEST_SUITE(BaseTransactionTest);

#ifndef WITH_PERFORMANCE_TESTS
    CPPUNIT_TEST(test_check_gpg_signatures_no_gpgcheck);
    CPPUNIT_TEST(test_check_gpg_signatures_fail);
#endif

#ifdef WITH_PERFORMANCE_TESTS
#endif

    CPPUNIT_TEST_SUITE_END();

public:
    void test_check_gpg_signatures_no_gpgcheck();
    void test_check_gpg_signatures_fail();
};


#endif  // TEST_LIBDNF_BASE_TRANSACTION_HPP
