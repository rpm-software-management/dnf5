// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 2.1 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.


#ifndef LIBDNF5_CLI_PROGRESSBAR_PROGRESS_BAR_HPP
#define LIBDNF5_CLI_PROGRESSBAR_PROGRESS_BAR_HPP

#include "libdnf5-cli/defs.h"

#include <chrono>
#include <string>
#include <vector>


namespace libdnf5::cli::progressbar {


enum class ProgressBarState : int {
    READY,    // no color
    STARTED,  // no color
    SUCCESS,  // green
    WARNING,  // yellow
    ERROR,    // red
};


enum class MessageType : int {
    INFO,     // no color
    SUCCESS,  // green
    WARNING,  // yellow
    ERROR,    // red
};


class LIBDNF_CLI_API ProgressBar {
public:
    using Message = std::pair<MessageType, std::string>;

    explicit ProgressBar(int64_t total_ticks);
    explicit ProgressBar(int64_t total_ticks, const std::string & description);
    virtual ~ProgressBar() = default;

    void reset();

    // ticks
    int64_t get_ticks() const noexcept { return ticks; }
    void set_ticks(int64_t value);
    void add_ticks(int64_t value);

    // total ticks
    int64_t get_total_ticks() const noexcept { return total_ticks; }
    void set_total_ticks(int64_t value);

    // number
    int32_t get_number() const noexcept { return number; }
    void set_number(int32_t value) { number = value; }

    // total
    int32_t get_total() const noexcept { return total; }
    void set_total(int32_t value) { total = value; }

    // workflow
    void start();
    ProgressBarState get_state() const noexcept { return state; }
    void set_state(ProgressBarState value) {
        update();
        state = value;
    }
    bool is_finished() const noexcept {
        return get_state() != ProgressBarState::READY && get_state() != ProgressBarState::STARTED;
    }
    bool is_failed() const noexcept {
        return get_state() == ProgressBarState::ERROR || get_state() == ProgressBarState::WARNING;
    }

    // description
    std::string get_description() const noexcept { return description; }
    void set_description(const std::string & value) { description = value; }

    // messages
    void add_message(MessageType type, const std::string & message) { messages.emplace_back(type, message); }
    /// remove the last message
    void pop_message();
    const std::vector<Message> & get_messages() const noexcept { return messages; }

    // auto-finish feature; turn off if you want to handle state manually
    bool get_auto_finish() const noexcept { return auto_finish; }
    void set_auto_finish(bool value) { auto_finish = value; }

    // stats
    void update();
    int32_t get_percent_done() const noexcept { return percent_done; }
    int64_t get_current_speed() const noexcept { return current_speed; }
    int64_t get_average_speed() const noexcept { return average_speed; }
    int64_t get_elapsed_seconds() const noexcept { return elapsed_seconds; }
    int64_t get_remaining_seconds() const noexcept { return remaining_seconds; }

    std::chrono::time_point<std::chrono::system_clock> get_begin() { return begin; }

    LIBDNF_CLI_API friend std::ostream & operator<<(std::ostream & os, ProgressBar & bar);

protected:
    virtual void to_stream(std::ostream & stream) = 0;

private:
    // ticks
    int64_t ticks = -1;
    int64_t total_ticks = -1;

    // numbers
    int32_t number = 0;
    int32_t total = 0;

    // description
    std::string description;

    // messages
    std::vector<Message> messages;

    ProgressBarState state = ProgressBarState::READY;

    int32_t percent_done = -1;
    int64_t elapsed_seconds = 0;
    int64_t remaining_seconds = 0;
    int64_t average_speed = 0;

    bool auto_finish = true;

    std::chrono::time_point<std::chrono::system_clock> begin = std::chrono::system_clock::from_time_t(0);
    std::chrono::time_point<std::chrono::system_clock> end = std::chrono::system_clock::from_time_t(0);

    // current (average) speed over the last second
    std::chrono::time_point<std::chrono::system_clock> current_speed_window_start = std::chrono::system_clock::now();
    int64_t current_speed = 0;
    int64_t current_speed_window_ticks = 0;
};


}  // namespace libdnf5::cli::progressbar


#endif  // #define LIBDNF5_CLI_PROGRESSBAR_PROGRESS_BAR_HPP
