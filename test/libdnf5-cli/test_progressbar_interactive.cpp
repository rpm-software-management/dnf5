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


#include "test_progressbar_interactive.hpp"

#include "../shared/private_accessor.hpp"
#include "../shared/utils.hpp"
#include "utils/string.hpp"

#include <fmt/format.h>
#include <libdnf5-cli/progressbar/download_progress_bar.hpp>
#include <libdnf5-cli/progressbar/multi_progress_bar.hpp>


CPPUNIT_TEST_SUITE_REGISTRATION(ProgressbarInteractiveTest);

namespace {

// We look for control sequnces (such as move cursor up N times and carriage return)
// and perform them. It basically simulates a terminal emulator.
//
// It can look like: "\x1b[9A\r" = move cursor 9 times up followed by carriage return
std::string perform_control_sequences(std::string target) {
    const char * raw_string = target.c_str();

    std::string amount_str = "";
    size_t amount = 1;
    enum ControlSequence {
        EMPTY,
        ESC,
        CONTROL_SEQUENCE_INTRO,
        CONTROL_SEQUENCE_AMOUNT,
    };
    ControlSequence state = EMPTY;
    std::vector<std::string> output = {{}};
    size_t current_row = 0;
    size_t current_column = 0;
    for (size_t input_pos = 0; input_pos < strlen(raw_string); input_pos++) {
        char current_value = raw_string[input_pos];
        if (current_value == '\n') {
            current_row++;
            current_column = 0;
        } else if (current_value == '\r') {
            current_column = 0;
        } else if (current_value == '\x1b') {
            state = ESC;
        } else if (current_value == '[' && state == ESC) {
            state = CONTROL_SEQUENCE_INTRO;
            amount = 1;
            amount_str.clear();
        } else if ((state == CONTROL_SEQUENCE_INTRO || state == CONTROL_SEQUENCE_AMOUNT) && isdigit(current_value)) {
            // current_value has to be one of: 0123456789
            amount_str += current_value;
            amount = (size_t)stoi(amount_str);
            state = CONTROL_SEQUENCE_AMOUNT;
        } else if ((state == CONTROL_SEQUENCE_INTRO || state == CONTROL_SEQUENCE_AMOUNT) && current_value == 'A') {
            if (amount > current_row) {
                CPPUNIT_FAIL(fmt::format("Cursor up control sequnce outside of output"));
            }
            current_row -= amount;
            state = EMPTY;
        } else if ((state == CONTROL_SEQUENCE_INTRO || state == CONTROL_SEQUENCE_AMOUNT) && current_value == 'B') {
            current_row += amount;
            state = EMPTY;
        } else if (state == CONTROL_SEQUENCE_AMOUNT && amount == 2 && current_value == 'K') {
            // erase the entire line
            if (current_row < output.size()) {
                output[current_row].clear();
            }
            state = EMPTY;
        } else if (state == CONTROL_SEQUENCE_AMOUNT && current_value == 'm') {
            // This is a color control sequence, just skip it
            state = EMPTY;
        } else if (state == CONTROL_SEQUENCE_INTRO || state == CONTROL_SEQUENCE_AMOUNT) {
            CPPUNIT_FAIL(fmt::format(
                "Unknown control sequence: \\x1b[{}{} at position: {}", amount_str, current_value, input_pos));
        } else {
            while (current_row >= output.size()) {
                output.push_back({});
            }
            while (current_column >= output[current_row].length()) {
                output[current_row].push_back(' ');
            }
            output[current_row][current_column] = current_value;
            current_column++;
        }
    }

    while (current_row >= output.size()) {
        output.push_back({});
    }

    return libdnf5::utils::string::join(output, "\n");
}

// Allows accessing private methods
create_private_getter_template;
create_getter(to_stream, &libdnf5::cli::progressbar::DownloadProgressBar::to_stream);

}  //namespace

void ProgressbarInteractiveTest::setUp() {
    // MultiProgressBar behaves differently depending on interactivity
    setenv("DNF5_FORCE_INTERACTIVE", "1", 1);
    // Force columns to 70 to make output independ of where it is run
    setenv("FORCE_COLUMNS", "70", 1);
    // Wide characters do not work at all until we set locales in the code
    setlocale(LC_ALL, "C.UTF-8");
}

void ProgressbarInteractiveTest::tearDown() {
    unsetenv("DNF5_FORCE_INTERACTIVE");
    unsetenv("FORCE_COLUMNS");
}

void ProgressbarInteractiveTest::test_perform_control_sequences() {
    // This tests the perform_control_sequences testing utility.

    std::string expected = "mmmmm\ndfdfdfdf";
    CPPUNIT_ASSERT_EQUAL(expected, perform_control_sequences("asd\ndfdfdfdf\x1b[1A\rmmmmm"));

    expected = "aaa\nmmmmbb\ncccccccc";
    CPPUNIT_ASSERT_EQUAL(expected, perform_control_sequences("aaa\nbbbbbb\ncccccccc\x1b[1A\rmmmm\n"));

    expected = "aaaa\nmmmmbb\ncccccccc\ndddddd";
    CPPUNIT_ASSERT_EQUAL(expected, perform_control_sequences("aaaa\nbbbbbb\ncccccccc\ndddddd\x1b[2A\rmmmm\n"));

    expected = "aaaa\nb\nmmmm\nxxx\nb\nbb\ncccccccc\ndddddd";
    CPPUNIT_ASSERT_EQUAL(
        expected, perform_control_sequences("aaaa\nb\nb\nb\nb\nbb\ncccccccc\ndddddd\x1b[5A\rmmmm\nxxx\n"));

    expected = "aaa";
    CPPUNIT_ASSERT_EQUAL(expected, perform_control_sequences("xxx\x1b[2K\raaa"));

    expected = "xxx\naaa";
    CPPUNIT_ASSERT_EQUAL(expected, perform_control_sequences("xxx\n\x1b[2K\raaa"));

    expected = "xxx\naaa\nssss";
    CPPUNIT_ASSERT_EQUAL(expected, perform_control_sequences("xxx\nfffff\x1b[2K\raaa\nssss"));
}

void ProgressbarInteractiveTest::test_download_progress_bar() {
    // In interactive mode not finished download progress bar is printed

    auto download_progress_bar = std::make_unique<libdnf5::cli::progressbar::DownloadProgressBar>(10, "test");
    download_progress_bar->set_ticks(4);
    download_progress_bar->set_state(libdnf5::cli::progressbar::ProgressBarState::STARTED);
    auto download_progress_bar_raw = download_progress_bar.get();

    std::ostringstream oss;
    (*download_progress_bar.*get(to_stream{}))(oss);
    Pattern expected = "\\[0/0\\] test                     40% | ????? ??B\\/s |   4.0   B | ???????";
    ASSERT_MATCHES(expected, oss.str());

    download_progress_bar_raw->set_ticks(10);
    download_progress_bar_raw->set_state(libdnf5::cli::progressbar::ProgressBarState::SUCCESS);
    oss.str("");
    (*download_progress_bar.*get(to_stream{}))(oss);

    expected = "\\[0/0\\] test                    100% | ????? ??B\\/s |  10.0   B | ???????";
    ASSERT_MATCHES(expected, oss.str());
}

void ProgressbarInteractiveTest::test_download_progress_bar_with_messages() {
    // In interactive mode not finished download progress bar with messages is printed

    auto download_progress_bar = std::make_unique<libdnf5::cli::progressbar::DownloadProgressBar>(10, "test");
    download_progress_bar->set_ticks(4);
    download_progress_bar->set_state(libdnf5::cli::progressbar::ProgressBarState::STARTED);
    download_progress_bar->add_message(libdnf5::cli::progressbar::MessageType::INFO, "test message1");
    download_progress_bar->add_message(libdnf5::cli::progressbar::MessageType::INFO, "test message2");
    download_progress_bar->add_message(libdnf5::cli::progressbar::MessageType::INFO, "test もで 諤奯ゞ");

    std::ostringstream oss;
    (*download_progress_bar.*get(to_stream{}))(oss);
    Pattern expected =
        "\\[0/0\\] test                     40% | ????? ??B\\/s |   4.0   B | ???????\n"
        ">>> test message1                                                     \n"
        ">>> test message2                                                     \n"
        ">>> test もで 諤奯ゞ                                                  ";
    ASSERT_MATCHES(expected, oss.str());

    download_progress_bar->pop_message();
    download_progress_bar->pop_message();
    download_progress_bar->pop_message();

    oss.str("");
    (*download_progress_bar.*get(to_stream{}))(oss);
    expected = "\\[0/0\\] test                     40% | ????? ??B\\/s |   4.0   B | ???????";
    ASSERT_MATCHES(expected, oss.str());
}

void ProgressbarInteractiveTest::test_multi_progress_bar_with_total_finished() {
    // In interactive mode finished multi progressbar ends with a new line.

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
        "\\[1/2\\] test1                   100% | ????? ??B\\/s |  10.0   B |  ??????\n"
        "\\[2/2\\] test2                   100% | ????? ??B\\/s |  10.0   B |  ??????\n"
        "----------------------------------------------------------------------\n"
        "\\[2/2\\] Total                   100% | ????? ??B\\/s |  20.0   B |  ??????\n";

    ASSERT_MATCHES(expected, perform_control_sequences(oss.str()));
}

void ProgressbarInteractiveTest::test_multi_progress_bar_with_messages_with_total() {
    // In interactive mode not finished progressbar with messages doesn't end with a new line.
    // However finished progressbar does end with a new line.
    // When messages are removed the multi progressbar shrinks, we cannot remove the already printed
    // line but ensure it doesn't contain garbage.

    auto download_progress_bar = std::make_unique<libdnf5::cli::progressbar::DownloadProgressBar>(10, "test");
    libdnf5::cli::progressbar::MultiProgressBar multi_progress_bar;
    auto download_progress_bar_raw = download_progress_bar.get();
    multi_progress_bar.add_bar(std::move(download_progress_bar));

    download_progress_bar_raw->set_ticks(4);
    download_progress_bar_raw->set_state(libdnf5::cli::progressbar::ProgressBarState::STARTED);
    download_progress_bar_raw->add_message(libdnf5::cli::progressbar::MessageType::INFO, "test message1");

    std::ostringstream oss;
    oss << multi_progress_bar;
    Pattern expected =
        "\\[1/1\\] test                     40% | ????? ??B\\/s |   4.0   B | ???????\n"
        ">>> test message1                                                     \n"
        "----------------------------------------------------------------------\n"
        "\\[0/1\\] Total                    40% | ????? ??B\\/s |   4.0   B | ???????";

    ASSERT_MATCHES(expected, perform_control_sequences(oss.str()));

    download_progress_bar_raw->pop_message();
    oss << multi_progress_bar;

    // Do several iterations, this simulates adding and removing scriplet messages.
    // (They are removed when a scriptlet succeeds and doesn't print any output.)
    download_progress_bar_raw->add_message(libdnf5::cli::progressbar::MessageType::INFO, "test message1");
    oss << multi_progress_bar;
    download_progress_bar_raw->pop_message();
    oss << multi_progress_bar;
    download_progress_bar_raw->add_message(libdnf5::cli::progressbar::MessageType::INFO, "test message1");
    oss << multi_progress_bar;
    download_progress_bar_raw->pop_message();
    oss << multi_progress_bar;

    // Output ends with an empty line because the progressbar has previously already
    // extended to 4 lines and there is not way to remove the line
    expected =
        "\\[1/1\\] test                     40% | ????? ??B\\/s |   4.0   B | ???????\n"
        "----------------------------------------------------------------------\n"
        "\\[0/1\\] Total                    40% | ????? ??B\\/s |   4.0   B | ???????\n"
        "";

    ASSERT_MATCHES(expected, perform_control_sequences(oss.str()));

    download_progress_bar_raw->add_message(libdnf5::cli::progressbar::MessageType::INFO, "test message1");
    download_progress_bar_raw->add_message(libdnf5::cli::progressbar::MessageType::INFO, "test message2");
    download_progress_bar_raw->add_message(libdnf5::cli::progressbar::MessageType::INFO, "test もで 諤奯ゞ");
    oss << multi_progress_bar;
    download_progress_bar_raw->pop_message();
    download_progress_bar_raw->pop_message();
    download_progress_bar_raw->pop_message();
    download_progress_bar_raw->set_ticks(10);
    download_progress_bar_raw->set_state(libdnf5::cli::progressbar::ProgressBarState::SUCCESS);
    oss << multi_progress_bar;
    // Simulate appending after the finished MultiProgressBar to ensure cursor is at correct postion
    oss << "Complete!";

    expected =
        "\\[1/1\\] test                    100% | ????? ??B\\/s |  10.0   B | ???????\n"
        "----------------------------------------------------------------------\n"
        "\\[1/1\\] Total                   100% | ????? ??B\\/s |  10.0   B | ???????\n"
        "Complete!\n"
        "\n"
        "";

    ASSERT_MATCHES(expected, perform_control_sequences(oss.str()));
}

void ProgressbarInteractiveTest::test_multi_progress_bars_with_messages_with_total() {
    // Same as above but with multiple bars in multi progressbar

    auto download_progress_bar1 = std::make_unique<libdnf5::cli::progressbar::DownloadProgressBar>(10, "test1");
    download_progress_bar1->set_ticks(10);
    download_progress_bar1->set_state(libdnf5::cli::progressbar::ProgressBarState::SUCCESS);

    auto download_progress_bar2 = std::make_unique<libdnf5::cli::progressbar::DownloadProgressBar>(10, "test2");
    download_progress_bar2->set_auto_finish(false);
    download_progress_bar2->start();

    libdnf5::cli::progressbar::MultiProgressBar multi_progress_bar;
    multi_progress_bar.add_bar(std::move(download_progress_bar1));
    auto download_progress_bar2_raw = download_progress_bar2.get();
    multi_progress_bar.add_bar(std::move(download_progress_bar2));

    download_progress_bar2_raw->set_ticks(4);
    download_progress_bar2_raw->set_state(libdnf5::cli::progressbar::ProgressBarState::STARTED);
    download_progress_bar2_raw->add_message(libdnf5::cli::progressbar::MessageType::INFO, "test message1");
    download_progress_bar2_raw->add_message(libdnf5::cli::progressbar::MessageType::INFO, "test message2");
    download_progress_bar2_raw->add_message(libdnf5::cli::progressbar::MessageType::INFO, "test もで 諤奯ゞ");

    std::ostringstream oss;
    oss << multi_progress_bar;
    Pattern expected =
        "\\[1/2\\] test1                   100% | ????? ??B\\/s |  10.0   B | ???????\n"
        "\\[2/2\\] test2                    40% | ????? ??B\\/s |   4.0   B | ???????\n"
        ">>> test message1                                                     \n"
        ">>> test message2                                                     \n"
        ">>> test もで 諤奯ゞ                                                  \n"
        "----------------------------------------------------------------------\n"
        "\\[1/2\\] Total                    70% | ????? ??B\\/s |  14.0   B | ???????";

    ASSERT_MATCHES(expected, perform_control_sequences(oss.str()));

    download_progress_bar2_raw->pop_message();
    download_progress_bar2_raw->pop_message();
    download_progress_bar2_raw->pop_message();
    oss << multi_progress_bar;
    expected =
        "\\[1/2\\] test1                   100% | ????? ??B\\/s |  10.0   B | ???????\n"
        "\\[2/2\\] test2                    40% | ????? ??B\\/s |   4.0   B | ???????\n"
        "----------------------------------------------------------------------\n"
        "\\[1/2\\] Total                    70% | ????? ??B\\/s |  14.0   B | ???????\n"
        "\n"
        "\n";
    ASSERT_MATCHES(expected, perform_control_sequences(oss.str()));

    // new iteration
    download_progress_bar2_raw->add_message(libdnf5::cli::progressbar::MessageType::INFO, "test message1");
    oss << multi_progress_bar;
    download_progress_bar2_raw->pop_message();
    oss << multi_progress_bar;

    ASSERT_MATCHES(expected, perform_control_sequences(oss.str()));
}

void ProgressbarInteractiveTest::test_multi_progress_bar_with_messages() {
    // With single bar and Total disabled

    auto download_progress_bar = std::make_unique<libdnf5::cli::progressbar::DownloadProgressBar>(10, "test");

    libdnf5::cli::progressbar::MultiProgressBar multi_progress_bar;
    multi_progress_bar.set_total_bar_visible_limit(libdnf5::cli::progressbar::MultiProgressBar::NEVER_VISIBLE_LIMIT);
    auto download_progress_bar_raw = download_progress_bar.get();
    multi_progress_bar.add_bar(std::move(download_progress_bar));

    download_progress_bar_raw->set_ticks(4);
    download_progress_bar_raw->set_state(libdnf5::cli::progressbar::ProgressBarState::STARTED);
    download_progress_bar_raw->add_message(libdnf5::cli::progressbar::MessageType::INFO, "test message1");

    std::ostringstream oss;
    oss << multi_progress_bar;
    Pattern expected =
        "\\[1/1\\] test                     40% | ????? ??B\\/s |   4.0   B | ???????\n"
        ">>> test message1                                                     ";

    ASSERT_MATCHES(expected, perform_control_sequences(oss.str()));

    download_progress_bar_raw->pop_message();

    oss << multi_progress_bar;
    expected =
        "\\[1/1\\] test                     40% | ????? ??B\\/s |   4.0   B | ???????\n"
        "";

    ASSERT_MATCHES(expected, perform_control_sequences(oss.str()));

    // Do several iterations
    download_progress_bar_raw->add_message(libdnf5::cli::progressbar::MessageType::INFO, "test message1");
    oss << multi_progress_bar;
    download_progress_bar_raw->pop_message();
    oss << multi_progress_bar;
    download_progress_bar_raw->add_message(libdnf5::cli::progressbar::MessageType::INFO, "test message1");
    oss << multi_progress_bar;
    download_progress_bar_raw->pop_message();
    oss << multi_progress_bar;

    expected =
        "\\[1/1\\] test                     40% | ????? ??B\\/s |   4.0   B | ???????\n"
        "";
    ASSERT_MATCHES(expected, perform_control_sequences(oss.str()));
}

void ProgressbarInteractiveTest::test_multi_progress_bars_with_messages() {
    // With multiple bars and Total disabled

    auto download_progress_bar1 = std::make_unique<libdnf5::cli::progressbar::DownloadProgressBar>(10, "test1");
    download_progress_bar1->set_ticks(10);
    download_progress_bar1->set_state(libdnf5::cli::progressbar::ProgressBarState::SUCCESS);

    auto download_progress_bar2 = std::make_unique<libdnf5::cli::progressbar::DownloadProgressBar>(10, "test2");

    libdnf5::cli::progressbar::MultiProgressBar multi_progress_bar;
    multi_progress_bar.set_total_bar_visible_limit(libdnf5::cli::progressbar::MultiProgressBar::NEVER_VISIBLE_LIMIT);
    multi_progress_bar.add_bar(std::move(download_progress_bar1));
    auto download_progress_bar2_raw = download_progress_bar2.get();
    multi_progress_bar.add_bar(std::move(download_progress_bar2));

    download_progress_bar2_raw->set_ticks(4);
    download_progress_bar2_raw->set_state(libdnf5::cli::progressbar::ProgressBarState::STARTED);
    download_progress_bar2_raw->add_message(libdnf5::cli::progressbar::MessageType::INFO, "test message1");
    download_progress_bar2_raw->add_message(libdnf5::cli::progressbar::MessageType::INFO, "test message2");
    download_progress_bar2_raw->add_message(libdnf5::cli::progressbar::MessageType::INFO, "test こんにちは世界！");

    std::ostringstream oss;
    oss << multi_progress_bar;
    Pattern expected =
        "\\[1/2\\] test1                   100% | ????? ??B\\/s |  10.0   B | ???????\n"
        "\\[2/2\\] test2                    40% | ????? ??B\\/s |   4.0   B | ???????\n"
        ">>> test message1                                                     \n"
        ">>> test message2                                                     \n"
        ">>> test こんにちは世界！                                             ";

    ASSERT_MATCHES(expected, perform_control_sequences(oss.str()));

    download_progress_bar2_raw->pop_message();
    download_progress_bar2_raw->pop_message();
    download_progress_bar2_raw->pop_message();
    oss << multi_progress_bar;

    expected =
        "\\[1/2\\] test1                   100% | ????? ??B\\/s |  10.0   B | ???????\n"
        "\\[2/2\\] test2                    40% | ????? ??B\\/s |   4.0   B | ???????\n"
        "\n"
        "\n";
    ASSERT_MATCHES(expected, perform_control_sequences(oss.str()));

    // new iteration
    download_progress_bar2_raw->add_message(libdnf5::cli::progressbar::MessageType::INFO, "test message1");
    oss << multi_progress_bar;
    download_progress_bar2_raw->pop_message();
    oss << multi_progress_bar;

    expected =
        "\\[1/2\\] test1                   100% | ????? ??B\\/s |  10.0   B | ???????\n"
        "\\[2/2\\] test2                    40% | ????? ??B\\/s |   4.0   B | ???????\n"
        "\n"
        "\n";

    ASSERT_MATCHES(expected, perform_control_sequences(oss.str()));

    download_progress_bar2_raw->add_message(libdnf5::cli::progressbar::MessageType::INFO, "test message1");
    download_progress_bar2_raw->add_message(libdnf5::cli::progressbar::MessageType::INFO, "test message1");
    download_progress_bar2_raw->add_message(libdnf5::cli::progressbar::MessageType::INFO, "test message1");
    oss << multi_progress_bar;
    download_progress_bar2_raw->pop_message();
    download_progress_bar2_raw->pop_message();
    download_progress_bar2_raw->pop_message();
    download_progress_bar2_raw->set_ticks(10);
    download_progress_bar2_raw->set_state(libdnf5::cli::progressbar::ProgressBarState::SUCCESS);
    oss << multi_progress_bar;
    // Simulate appending after the finished MultiProgressBar to ensure cursor is at correct postion
    oss << "Complete!";

    expected =
        "\\[1/2\\] test1                   100% | ????? ??B\\/s |  10.0   B | ???????\n"
        "\\[2/2\\] test2                   100% | ????? ??B\\/s |  10.0   B | ???????\n"
        "Complete!\n"
        "\n"
        "";

    ASSERT_MATCHES(expected, perform_control_sequences(oss.str()));
}

void ProgressbarInteractiveTest::test_multi_progress_bar_with_short_messages() {
    // With single bar and Total disabled
    // First print long message and remove it (successful scriptlet for package with long name),
    // then print short msg (different package that prints something to the log and the message stays).

    auto download_progress_bar = std::make_unique<libdnf5::cli::progressbar::DownloadProgressBar>(10, "test");

    libdnf5::cli::progressbar::MultiProgressBar multi_progress_bar;
    multi_progress_bar.set_total_bar_visible_limit(libdnf5::cli::progressbar::MultiProgressBar::NEVER_VISIBLE_LIMIT);
    auto download_progress_bar_raw = download_progress_bar.get();
    multi_progress_bar.add_bar(std::move(download_progress_bar));

    download_progress_bar_raw->set_ticks(4);
    download_progress_bar_raw->set_state(libdnf5::cli::progressbar::ProgressBarState::STARTED);
    download_progress_bar_raw->add_message(
        libdnf5::cli::progressbar::MessageType::INFO, "test loooooooooooooooooooooooooooooooooooooooooong message");

    std::ostringstream oss;
    oss << multi_progress_bar;
    Pattern expected =
        "\\[1/1\\] test                     40% | ????? ??B\\/s |   4.0   B | ???????\n"
        ">>> test loooooooooooooooooooooooooooooooooooooooooong message        ";

    ASSERT_MATCHES(expected, perform_control_sequences(oss.str()));

    download_progress_bar_raw->pop_message();
    download_progress_bar_raw->add_message(libdnf5::cli::progressbar::MessageType::INFO, "test short message");

    oss << multi_progress_bar;
    expected =
        "\\[1/1\\] test                     40% | ????? ??B\\/s |   4.0   B | ???????\n"
        ">>> test short message                                                ";

    ASSERT_MATCHES(expected, perform_control_sequences(oss.str()));
}
