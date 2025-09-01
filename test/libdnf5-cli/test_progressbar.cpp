// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.


#include "test_progressbar.hpp"

#include "../shared/utils.hpp"

#include <libdnf5-cli/progressbar/download_progress_bar.hpp>
#include <libdnf5-cli/progressbar/multi_progress_bar.hpp>


CPPUNIT_TEST_SUITE_REGISTRATION(ProgressbarTest);

void ProgressbarTest::setUp() {
    // MultiProgressBar behaves differently depending on interactivity
    setenv("DNF5_FORCE_INTERACTIVE", "0", 1);
    // Force columns to 70 to make output independ of where it is run
    setenv("FORCE_COLUMNS", "70", 1);
}

void ProgressbarTest::tearDown() {
    unsetenv("DNF5_FORCE_INTERACTIVE");
    unsetenv("FORCE_COLUMNS");
}

void ProgressbarTest::test_download_progress_bar() {
    // In non interactive mode download progress bar is printed only when finished.

    auto download_progress_bar = std::make_unique<libdnf5::cli::progressbar::DownloadProgressBar>(10, "test");
    download_progress_bar->set_ticks(4);
    download_progress_bar->set_state(libdnf5::cli::progressbar::ProgressBarState::STARTED);
    auto download_progress_bar_raw = download_progress_bar.get();

    std::ostringstream oss;
    oss << *download_progress_bar;
    Pattern expected = "";
    ASSERT_MATCHES(expected, oss.str());

    download_progress_bar_raw->set_ticks(10);
    download_progress_bar_raw->set_state(libdnf5::cli::progressbar::ProgressBarState::SUCCESS);
    oss << *download_progress_bar;

    expected = "\\[0/0\\] test                    100% | ????? ??B\\/s |  10.0   B | ???????";
    ASSERT_MATCHES(expected, oss.str());
}

void ProgressbarTest::test_multi_progress_bar() {
    // In non interactive mode finished multiline progressbar ends with a new line.

    auto download_progress_bar1 = std::make_unique<libdnf5::cli::progressbar::DownloadProgressBar>(10, "test1");
    download_progress_bar1->set_ticks(10);
    download_progress_bar1->set_state(libdnf5::cli::progressbar::ProgressBarState::SUCCESS);

    auto download_progress_bar2 = std::make_unique<libdnf5::cli::progressbar::DownloadProgressBar>(10, "test2");
    download_progress_bar2->set_ticks(10);
    download_progress_bar2->set_state(libdnf5::cli::progressbar::ProgressBarState::SUCCESS);

    libdnf5::cli::progressbar::MultiProgressBar multi_progress_bar;
    multi_progress_bar.add_bar(std::move(download_progress_bar1));
    multi_progress_bar.add_bar(std::move(download_progress_bar2));
    std::ostringstream oss;
    oss << multi_progress_bar;

    Pattern expected =
        "\\[1/2\\] test1                   100% | ????? ??B\\/s |  10.0   B | ???????\n"
        "\\[2/2\\] test2                   100% | ????? ??B\\/s |  10.0   B | ???????\n"
        "----------------------------------------------------------------------\n"
        "\\[2/2\\] Total                   100% | ????? ??B\\/s |  20.0   B | ???????\n";
    ASSERT_MATCHES(expected, oss.str());
}

void ProgressbarTest::test_multi_progress_bar_unfinished() {
    // In non-interacitve mode:
    // - unfinished progressbars are not printed
    // - total is not printed until all progressbars are finished

    auto download_progress_bar1 = std::make_unique<libdnf5::cli::progressbar::DownloadProgressBar>(10, "test1");
    download_progress_bar1->set_ticks(10);
    download_progress_bar1->set_state(libdnf5::cli::progressbar::ProgressBarState::SUCCESS);

    auto download_progress_bar2 = std::make_unique<libdnf5::cli::progressbar::DownloadProgressBar>(10, "test2");
    download_progress_bar2->set_ticks(4);
    download_progress_bar2->set_state(libdnf5::cli::progressbar::ProgressBarState::STARTED);
    auto download_progress_bar2_raw = download_progress_bar2.get();

    libdnf5::cli::progressbar::MultiProgressBar multi_progress_bar;
    multi_progress_bar.add_bar(std::move(download_progress_bar1));
    multi_progress_bar.add_bar(std::move(download_progress_bar2));
    std::ostringstream oss;

    oss << multi_progress_bar;
    Pattern expected = "\\[1/2\\] test1                   100% | ????? ??B\\/s |  10.0   B | ???????\n";
    ASSERT_MATCHES(expected, oss.str());

    // More iterations
    download_progress_bar2_raw->set_ticks(5);
    oss << multi_progress_bar;
    download_progress_bar2_raw->set_ticks(6);
    oss << multi_progress_bar;
    expected = "\\[1/2\\] test1                   100% | ????? ??B\\/s |  10.0   B | ???????\n";
    ASSERT_MATCHES(expected, oss.str());

    // Next iteration
    download_progress_bar2_raw->set_ticks(10);
    download_progress_bar2_raw->set_state(libdnf5::cli::progressbar::ProgressBarState::SUCCESS);
    oss << multi_progress_bar;
    expected =
        "\\[1/2\\] test1                   100% | ????? ??B\\/s |  10.0   B | ???????\n"
        "\\[2/2\\] test2                   100% | ????? ??B\\/s |  10.0   B | ???????\n"
        "----------------------------------------------------------------------\n"
        "\\[2/2\\] Total                   100% | ????? ??B\\/s |  20.0   B | ???????\n";
    ASSERT_MATCHES(expected, oss.str());
}
