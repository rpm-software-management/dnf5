// Copyright Contributors to the DNF5 project.
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This file is part of DNF5: https://github.com/rpm-software-management/dnf5/
//
// DNF5 is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// DNF5 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with DNF5.  If not, see <https://www.gnu.org/licenses/>.

#ifndef DNF5_TEST_AUTOMATIC_EMAILMESSAGE_HPP
#define DNF5_TEST_AUTOMATIC_EMAILMESSAGE_HPP

// Note 1
#include <cppunit/extensions/HelperMacros.h>

class EmailMessageTest : public CPPUNIT_NS::TestFixture {
    CPPUNIT_TEST_SUITE(EmailMessageTest);
    CPPUNIT_TEST(test_email_emitter);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() {}
    void tearDown() {}

    void test_email_emitter();
};

#endif  // DNF5_TEST_AUTOMATIC_EMAILMESSAGE_HPP
