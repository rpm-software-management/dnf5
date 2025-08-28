// Copyright Contributors to the DNF5 project.
// SPDX-License-Identifier: GPL-2.0-or-later


#ifndef LIBDNF5_TEST_UTILS_URL_HPP
#define LIBDNF5_TEST_UTILS_URL_HPP


#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>


class UtilsUrlTest : public CppUnit::TestCase {
    CPPUNIT_TEST_SUITE(UtilsUrlTest);

    CPPUNIT_TEST(test_url_encode);
    CPPUNIT_TEST(test_url_decode);
    CPPUNIT_TEST(test_encode_decode_roundtrip);
    CPPUNIT_TEST(test_url_path_encode);
    CPPUNIT_TEST(test_is_url);

    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;
    void tearDown() override;

    void test_url_encode();
    void test_url_decode();
    void test_encode_decode_roundtrip();
    void test_url_path_encode();
    void test_is_url();

private:
};


#endif  // LIBDNF5_TEST_UTILS_URL_HPP
