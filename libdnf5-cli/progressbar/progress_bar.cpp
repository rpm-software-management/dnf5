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


#include "libdnf5-cli/progressbar/progress_bar.hpp"


namespace libdnf5::cli::progressbar {

class ProgressBar::Impl {
public:
    explicit Impl(int64_t total_ticks) : total_ticks{total_ticks} {}
    Impl(int64_t total_ticks, const std::string & description) : total_ticks{total_ticks}, description{description} {}

    // ticks
    int64_t ticks = -1;
    int64_t total_ticks = -1;

    // numbers
    int32_t number = 0;
    int32_t total = 0;

    // description
    std::string description;

    // messages
    std::vector<ProgressBar::Message> messages;

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


ProgressBar::~ProgressBar() = default;

ProgressBar::ProgressBar(int64_t total_ticks) : p_impl(new Impl(total_ticks)) {}

ProgressBar::ProgressBar(int64_t total_ticks, const std::string & description)
    : p_impl(new Impl(total_ticks, description)) {}

ProgressBar::ProgressBar(const ProgressBar & src) = default;
ProgressBar & ProgressBar::operator=(const ProgressBar & src) = default;
ProgressBar::ProgressBar(ProgressBar && src) noexcept = default;
ProgressBar & ProgressBar::operator=(ProgressBar && src) noexcept = default;


int64_t ProgressBar::get_ticks() const noexcept {
    return p_impl->ticks;
}

int64_t ProgressBar::get_total_ticks() const noexcept {
    return p_impl->total_ticks;
}

int32_t ProgressBar::get_number() const noexcept {
    return p_impl->number;
}

void ProgressBar::set_number(int32_t value) {
    p_impl->number = value;
}

int32_t ProgressBar::get_total() const noexcept {
    return p_impl->total;
}

void ProgressBar::set_total(int32_t value) {
    p_impl->total = value;
}

ProgressBarState ProgressBar::get_state() const noexcept {
    return p_impl->state;
}

void ProgressBar::set_state(ProgressBarState value) {
    update();
    p_impl->state = value;
}

bool ProgressBar::is_finished() const noexcept {
    return get_state() != ProgressBarState::READY && get_state() != ProgressBarState::STARTED;
}

bool ProgressBar::is_failed() const noexcept {
    return get_state() == ProgressBarState::ERROR || get_state() == ProgressBarState::WARNING;
}

std::string ProgressBar::get_description() const noexcept {
    return p_impl->description;
}

void ProgressBar::set_description(const std::string & value) {
    p_impl->description = value;
}

void ProgressBar::add_message(MessageType type, const std::string & message) {
    p_impl->messages.emplace_back(type, message);
}

const std::vector<ProgressBar::Message> & ProgressBar::get_messages() const noexcept {
    return p_impl->messages;
}

bool ProgressBar::get_auto_finish() const noexcept {
    return p_impl->auto_finish;
}

void ProgressBar::set_auto_finish(bool value) {
    p_impl->auto_finish = value;
}

int32_t ProgressBar::get_percent_done() const noexcept {
    return p_impl->percent_done;
}

int64_t ProgressBar::get_current_speed() const noexcept {
    return p_impl->current_speed;
}

int64_t ProgressBar::get_average_speed() const noexcept {
    return p_impl->average_speed;
}

int64_t ProgressBar::get_elapsed_seconds() const noexcept {
    return p_impl->elapsed_seconds;
}

int64_t ProgressBar::get_remaining_seconds() const noexcept {
    return p_impl->remaining_seconds;
}

std::chrono::time_point<std::chrono::system_clock> ProgressBar::get_begin() {
    return p_impl->begin;
}

void ProgressBar::reset() {
    p_impl->ticks = -1;
    p_impl->total_ticks = -1;
    p_impl->number = 0;
    p_impl->total = 0;
    p_impl->description = "";
    p_impl->messages.clear();
    p_impl->state = ProgressBarState::READY;
    p_impl->percent_done = -1;
    p_impl->elapsed_seconds = 0;
    p_impl->remaining_seconds = 0;
    p_impl->average_speed = 0;
    p_impl->auto_finish = true;
    p_impl->begin = std::chrono::system_clock::from_time_t(0);
    p_impl->end = std::chrono::system_clock::from_time_t(0);
    p_impl->current_speed_window_start = std::chrono::system_clock::now();
    p_impl->current_speed = 0;
    p_impl->current_speed_window_ticks = 0;
}


void ProgressBar::set_ticks(int64_t value) {
    if (is_finished()) {
        return;
    }
    auto new_ticks = value;
    if (p_impl->total_ticks >= 0) {
        new_ticks = std::min(new_ticks, p_impl->total_ticks);
    }
    if (new_ticks >= p_impl->ticks) {
        p_impl->current_speed_window_ticks += new_ticks - p_impl->ticks;
    } else {
        p_impl->current_speed_window_ticks = 0;
    }
    p_impl->ticks = new_ticks;
}


void ProgressBar::add_ticks(int64_t value) {
    set_ticks(p_impl->ticks + value);
}


void ProgressBar::start() {
    if (p_impl->begin == std::chrono::system_clock::from_time_t(0)) {
        p_impl->begin = std::chrono::system_clock::now();
        p_impl->state = ProgressBarState::STARTED;
    }
}


void ProgressBar::update() {
    if (is_finished()) {
        return;
    }

    if (p_impl->total_ticks < 0) {
        // unknown total ticks
        p_impl->percent_done = -1;
    } else if (p_impl->total_ticks == 0) {
        // can't divide by zero, consider progressbar 100% complete
        p_impl->percent_done = 100;
    } else if (p_impl->ticks >= p_impl->total_ticks) {
        p_impl->percent_done = 100;
    } else {
        p_impl->percent_done =
            static_cast<int32_t>(static_cast<float>(p_impl->ticks) / static_cast<float>(p_impl->total_ticks) * 100);
    }

    auto now = std::chrono::high_resolution_clock::now();

    // compute the current speed (ticks per second)
    auto delta = now - p_impl->current_speed_window_start;
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(delta).count();

    if (ms > 950) {
        p_impl->current_speed = p_impl->current_speed_window_ticks * 1000 / ms;
        // reset the window
        p_impl->current_speed_window_ticks = 0;
        p_impl->current_speed_window_start = now;
    } else if (p_impl->current_speed == 0 && ms != 0) {
        p_impl->current_speed = p_impl->current_speed_window_ticks * 1000 / ms;
    }

    // compute average speed
    delta = now - p_impl->begin;
    ms = std::chrono::duration_cast<std::chrono::milliseconds>(delta).count();
    if (ms == 0) {
        p_impl->average_speed = 0;
    } else {
        p_impl->average_speed = p_impl->ticks * 1000 / ms;
    }

    // compute elapsed seconds
    // round the result to display 00m00s less frequently
    p_impl->elapsed_seconds = std::chrono::round<std::chrono::seconds>(delta).count();

    // compute remaining time
    if (p_impl->total_ticks >= 0) {
        int64_t remaining_ticks = p_impl->total_ticks - p_impl->ticks;
        if (p_impl->current_speed != 0) {
            p_impl->remaining_seconds = remaining_ticks / p_impl->current_speed;
        }
    } else {
        // unknown total ticks
        p_impl->remaining_seconds = -1;
    }

    if (p_impl->total_ticks >= 0 && p_impl->ticks >= p_impl->total_ticks) {
        if (p_impl->auto_finish && p_impl->state == ProgressBarState::STARTED) {
            // avoid calling set_state() because it triggers update() and ends up in an endless recursion
            p_impl->state = ProgressBarState::SUCCESS;
        }
        p_impl->end = now;
        p_impl->percent_done = 100;
        p_impl->remaining_seconds = 0;
    }
}


void ProgressBar::set_total_ticks(int64_t value) {
    if (is_finished()) {
        return;
    }
    p_impl->total_ticks = value;
}


void ProgressBar::pop_message() {
    if (!p_impl->messages.empty()) {
        p_impl->messages.pop_back();
    }
}


std::ostream & operator<<(std::ostream & os, ProgressBar & bar) {
    bar.to_stream(os);
    return os;
}


}  // namespace libdnf5::cli::progressbar
