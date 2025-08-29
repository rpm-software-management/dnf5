// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later


#ifndef TEST_LIBDNF5_RPM_PACKAGE_SET_HPP
#define TEST_LIBDNF5_RPM_PACKAGE_SET_HPP


#include "../shared/base_test_case.hpp"

#include <cppunit/extensions/HelperMacros.h>
#include <libdnf5/rpm/package_set.hpp>

#include <memory>


class RpmPackageSetTest : public BaseTestCase {
    CPPUNIT_TEST_SUITE(RpmPackageSetTest);

#ifndef WITH_PERFORMANCE_TESTS
    CPPUNIT_TEST(test_add);
    CPPUNIT_TEST(test_contains);
    CPPUNIT_TEST(test_remove);
    CPPUNIT_TEST(test_union);
    CPPUNIT_TEST(test_intersection);
    CPPUNIT_TEST(test_difference);
    CPPUNIT_TEST(test_iterator);
#endif

#ifdef WITH_PERFORMANCE_TESTS
#endif

    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;

    void test_add();
    void test_contains();
    void test_remove();

    void test_union();
    void test_intersection();
    void test_difference();

    void test_iterator();


private:
    std::unique_ptr<libdnf5::rpm::PackageSet> set1;
    std::unique_ptr<libdnf5::rpm::PackageSet> set2;
};


#endif  // TEST_LIBDNF5_RPM_PACKAGE_SET_HPP
