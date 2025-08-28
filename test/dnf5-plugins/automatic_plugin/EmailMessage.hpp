// Copyright Contributors to the DNF5 project.
// SPDX-License-Identifier: GPL-2.0-or-later

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
