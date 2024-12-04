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


#ifndef TEST_LIBDNF5_CLI_PROGRESSBAR_INTERACTIVE_HPP
#define TEST_LIBDNF5_CLI_PROGRESSBAR_INTERACTIVE_HPP

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

class ProgressbarInteractiveTest : public CppUnit::TestCase {
    CPPUNIT_TEST_SUITE(ProgressbarInteractiveTest);

    CPPUNIT_TEST(test_perform_control_sequences);
    CPPUNIT_TEST(test_download_progress_bar);
    CPPUNIT_TEST(test_download_progress_bar_with_messages);
    CPPUNIT_TEST(test_multi_progress_bar_with_total_finished);
    CPPUNIT_TEST(test_multi_progress_bar_with_messages_with_total);
    CPPUNIT_TEST(test_multi_progress_bars_with_messages_with_total);
    CPPUNIT_TEST(test_multi_progress_bar_with_messages);
    CPPUNIT_TEST(test_multi_progress_bars_with_messages);

    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;
    void tearDown() override;

    void test_perform_control_sequences();
    void test_download_progress_bar();
    void test_download_progress_bar_with_messages();
    void test_multi_progress_bar_with_total_finished();
    void test_multi_progress_bar_with_messages_with_total();
    void test_multi_progress_bars_with_messages_with_total();
    void test_multi_progress_bar_with_messages();
    void test_multi_progress_bars_with_messages();
};


#endif  // TEST_LIBDNF5_CLI_PROGRESSBAR_INTERACTIVE_HPP
