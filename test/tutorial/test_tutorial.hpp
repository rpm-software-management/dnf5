// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later


#ifndef TEST_TUTORIAL_TEST_TUTORIAL_HPP
#define TEST_TUTORIAL_TEST_TUTORIAL_HPP


#include "libdnf5/utils/fs/temp.hpp"

#include <cppunit/extensions/HelperMacros.h>


class TutorialTest : public CppUnit::TestCase {
    CPPUNIT_TEST_SUITE(TutorialTest);

    CPPUNIT_TEST(test_create_base);
    CPPUNIT_TEST(test_load_repo);

    // don't register and run the test - it loads host system repos and fails
    // to detect releasever in the installroot
    //CPPUNIT_TEST(test_load_system_repos);

    CPPUNIT_TEST(test_query);
    CPPUNIT_TEST(test_transaction);
    CPPUNIT_TEST(test_force_arch);

    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;
    void tearDown() override;

    void test_create_base();
    void test_load_repo();
    void test_load_system_repos();
    void test_query();
    void test_transaction();
    void test_force_arch();

private:
    std::string baseurl = PROJECT_BINARY_DIR "/test/data/repos-rpm/rpm-repo1/";

    std::unique_ptr<libdnf5::utils::fs::TempDir> temp;
    std::filesystem::path installroot;
    std::filesystem::path cachedir;
};


#endif  // TEST_TUTORIAL_TEST_TUTORIAL_HPP
