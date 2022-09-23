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


#ifndef TEST_LIBDNF_TESTCASE_FIXTURE_HPP
#define TEST_LIBDNF_TESTCASE_FIXTURE_HPP

#include "utils/fs/temp.hpp"

#include "libdnf/base/base.hpp"

#include <cppunit/TestCase.h>


class TestCaseFixture : public CppUnit::TestCase {
public:
    void setUp() override;
    void tearDown() override;

    std::unique_ptr<libdnf::Base> get_preconfigured_base();

    // Only gets created if get_preconfigured_base() is called
    std::unique_ptr<libdnf::utils::fs::TempDir> temp;
};


#endif  // TEST_LIBDNF_TESTCASE_FIXTURE_HPP
