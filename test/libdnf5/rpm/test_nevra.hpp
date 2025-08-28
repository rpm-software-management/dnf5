// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef LIBDNF5_TEST_NEVRA_HPP
#define LIBDNF5_TEST_NEVRA_HPP


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
