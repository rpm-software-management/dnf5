// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#include "test_arch.hpp"

#include <libdnf5/rpm/arch.hpp>

CPPUNIT_TEST_SUITE_REGISTRATION(ArchTest);


void ArchTest::test_get_supported_arches() {
    std::vector<std::string> arches = libdnf5::rpm::get_supported_arches();
    // at least one architecture was found
    CPPUNIT_ASSERT(arches.size() > 0);
    // the arches vector is sorted and contains unique elements
    for (size_t i = 0; i < arches.size() - 1; ++i) {
        CPPUNIT_ASSERT(arches[i] < arches[i + 1]);
    }
}

void ArchTest::test_get_base_arch() {
    CPPUNIT_ASSERT_EQUAL(libdnf5::rpm::get_base_arch("x86_64"), std::string("x86_64"));
    CPPUNIT_ASSERT_EQUAL(libdnf5::rpm::get_base_arch("amd64"), std::string("x86_64"));
    CPPUNIT_ASSERT_EQUAL(libdnf5::rpm::get_base_arch("UNKNOWN_ARCH"), std::string(""));
}
