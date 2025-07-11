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

#include "libdnf5/common/impl_ptr.hpp"

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
    virtual ~ProgressBar();
    ProgressBar(const ProgressBar & src);
    ProgressBar & operator=(const ProgressBar & src);
    ProgressBar(ProgressBar && src) noexcept;
    ProgressBar & operator=(ProgressBar && src) noexcept;

    void reset();

    // ticks
    int64_t get_ticks() const noexcept;
    void set_ticks(int64_t value);
    void add_ticks(int64_t value);

    // total ticks
    int64_t get_total_ticks() const noexcept;
    void set_total_ticks(int64_t value);

    // number
    int32_t get_number() const noexcept;
    void set_number(int32_t value);

    // total
    int32_t get_total() const noexcept;
    void set_total(int32_t value);

    // workflow
    void start();
    ProgressBarState get_state() const noexcept;
    void set_state(ProgressBarState value);
    bool is_finished() const noexcept;
    bool is_failed() const noexcept;

    // description
    std::string get_description() const noexcept;
    void set_description(const std::string & value);

    // messages
    void add_message(MessageType type, const std::string & message);
    /// remove the last message
    void pop_message();
    const std::vector<Message> & get_messages() const noexcept;
    const std::string & get_message_prefix() const noexcept;

    /// Calculate number of lines occupied by messages when printed on terminal of the given width.
    /// Takes new lines and wide utf characters into account.
    std::size_t calculate_messages_terminal_lines(std::size_t terminal_width);

    // auto-finish feature; turn off if you want to handle state manually
    bool get_auto_finish() const noexcept;
    void set_auto_finish(bool value);

    // stats
    void update();
    int32_t get_percent_done() const noexcept;
    int64_t get_current_speed() const noexcept;
    int64_t get_average_speed() const noexcept;
    int64_t get_elapsed_seconds() const noexcept;
    int64_t get_remaining_seconds() const noexcept;

    std::chrono::time_point<std::chrono::system_clock> get_begin();

    LIBDNF_CLI_API friend std::ostream & operator<<(std::ostream & os, ProgressBar & bar);

protected:
    virtual void to_stream(std::ostream & stream) = 0;
    std::size_t get_message_padding(std::size_t terminal_width, std::string_view message, std::size_t message_index);

private:
    class LIBDNF_CLI_LOCAL Impl;
    ImplPtr<Impl> p_impl;
};


}  // namespace libdnf5::cli::progressbar


#endif  // #define LIBDNF5_CLI_PROGRESSBAR_PROGRESS_BAR_HPP
