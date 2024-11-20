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


#include "test_progressbar.hpp"

#include "../shared/private_accessor.hpp"

#include <fmt/format.h>
#include <fnmatch.h>
#include <libdnf5-cli/progressbar/download_progress_bar.hpp>
#include <libdnf5-cli/progressbar/multi_progress_bar.hpp>


CPPUNIT_TEST_SUITE_REGISTRATION(ProgressbarTest);

namespace {

// Allows accessing private methods
create_private_getter_template;
create_getter(to_stream, &libdnf5::cli::progressbar::DownloadProgressBar::to_stream);

}  //namespace

void ProgressbarTest::test_download_progress_bar() {
    auto download_progress_bar = std::make_unique<libdnf5::cli::progressbar::DownloadProgressBar>(10, "test");
    download_progress_bar->set_ticks(10);
    download_progress_bar->set_state(libdnf5::cli::progressbar::ProgressBarState::SUCCESS);

    std::ostringstream oss;
    (*download_progress_bar.*get(to_stream{}))(oss);
    // When running the tests binary directly (run_tests_cli) it uses terminal size to determine white space count.
    // To ensure the tests works match any number of white space.
    std::string expected = "\\[0/0\\] test      [ ]*      100% |   0.0   B\\/s |  10.0   B |  ?     ";
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        fmt::format("Expression: \"{}\" doesn't match output: \"{}\"", expected, oss.str()),
        fnmatch(expected.c_str(), oss.str().c_str(), FNM_EXTMATCH),
        0);
}

void ProgressbarTest::test_multi_progress_bar() {
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
    auto output = oss.str();

    // When running the tests binary directly (run_tests_cli) it uses terminal size to determine white space and dash count.
    // To ensure the tests works match any number of white space and dashes.
    std::string expected =
        "\\[1/2\\] test1     [ ]*      100% |   0.0   B\\/s |  10.0   B |  ?     \n"
        "\\[2/2\\] test2     [ ]*      100% |   0.0   B\\/s |  10.0   B |  ?     \n"
        "--------------------[-]*------------------------------------------------\n"
        "\\[2/2\\] Total     [ ]*      100% | ????? ??B\\/s |  20.0   B |  ??m??s\n";

    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        fmt::format("Expression: \"{}\" doesn't match output: \"{}\"", expected, oss.str()),
        fnmatch(expected.c_str(), oss.str().c_str(), FNM_EXTMATCH),
        0);
}
