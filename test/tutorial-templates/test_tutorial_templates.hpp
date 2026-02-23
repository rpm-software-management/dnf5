// Copyright Contributors to the DNF5 project.
// SPDX-License-Identifier: GPL-2.0-or-later


#ifndef TEST_TUTORIAL_TEST_TUTORIAL_HPP
#define TEST_TUTORIAL_TEST_TUTORIAL_HPP

#include "libdnf5/utils/fs/temp.hpp"

#include <cppunit/extensions/HelperMacros.h>

class TutorialTemplatesTest : public CppUnit::TestCase {
    CPPUNIT_TEST_SUITE(TutorialTemplatesTest);

    CPPUNIT_TEST(dnf5);

    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;
    void tearDown() override;

    void dnf5();

private:
    std::string baseurl = PROJECT_BINARY_DIR "/test/data/repos-rpm/rpm-repo1/";

    std::unique_ptr<libdnf5::utils::fs::TempDir> temp;
    std::filesystem::path installroot;
    std::filesystem::path cachedir;
};

#endif  // TEST_TUTORIAL_TEST_TUTORIAL_HPP
