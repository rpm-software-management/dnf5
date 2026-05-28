// Copyright Contributors to the DNF5 project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef TEST_LIBDNF5_BASE_INTERACTION_CALLBACKS_HPP
#define TEST_LIBDNF5_BASE_INTERACTION_CALLBACKS_HPP


#include "../shared/test_case_fixture.hpp"

#include <cppunit/extensions/HelperMacros.h>


class InteractionCallbacksTest : public TestCaseFixture {
    CPPUNIT_TEST_SUITE(InteractionCallbacksTest);
    CPPUNIT_TEST(test_message);
    CPPUNIT_TEST(test_confirm);
    CPPUNIT_TEST(test_choice);
    CPPUNIT_TEST(test_choice_with_default_option);
    CPPUNIT_TEST(test_input_text);
    CPPUNIT_TEST(test_input_text_with_validator);
    CPPUNIT_TEST(test_input_text_with_default_text);
    CPPUNIT_TEST(test_progress);
    CPPUNIT_TEST_SUITE_END();

public:
    void test_message();
    void test_confirm();
    void test_choice();
    void test_choice_with_default_option();
    void test_input_text();
    void test_input_text_with_validator();
    void test_input_text_with_default_text();
    void test_progress();
};


#endif  // TEST_LIBDNF5_BASE_INTERACTION_CALLBACKS_HPP
