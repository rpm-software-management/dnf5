/*
Copyright (C) 2020 Red Hat, Inc.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/


#ifndef TEST_LIBDNF_RPM_PACKAGE_SET_HPP
#define TEST_LIBDNF_RPM_PACKAGE_SET_HPP


#include "libdnf/base/base.hpp"
#include "libdnf/rpm/package_set.hpp"
#include "libdnf/rpm/repo_sack.hpp"
#include "libdnf/rpm/solv_sack.hpp"

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

#include <memory>

class RpmSolvQueryTest : public CppUnit::TestCase {
    CPPUNIT_TEST_SUITE(RpmSolvQueryTest);

#ifndef WITH_PERFORMANCE_TESTS
    CPPUNIT_TEST(test_size);
    CPPUNIT_TEST(test_ifilter_name);
#endif

#ifdef WITH_PERFORMANCE_TESTS
#endif

    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;
    void tearDown() override;

    void test_size();
    void test_ifilter_name();

private:
    std::unique_ptr<libdnf::Base> base;
    std::unique_ptr<libdnf::rpm::RepoSack> repo_sack;
    std::unique_ptr<libdnf::rpm::SolvSack> sack;
};


#endif  // TEST_LIBDNF_RPM_PACKAGE_SET_HPP
