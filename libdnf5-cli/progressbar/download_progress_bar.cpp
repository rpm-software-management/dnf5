/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2.1 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/


#include "libdnf5-cli/progressbar/download_progress_bar.hpp"

#include "../utils/utf8.hpp"

#include "libdnf5-cli/tty.hpp"

#include <algorithm>
#include <iomanip>


namespace libdnf5::cli::progressbar {


// Columns (widgets):
// <numbers><description...................><%%%><progress............><speed.....><size.....><time....>

// Example:
// [  1/999] filename                                                 | 123.4 kB/s | 123.4 kB |  00m01s
// [  2/999] filename                       100% [====================] 123.4 kB/s | 123.4 kB | -00m02s
// ----------------------------------------------------------------------------------------------------
// [  1/999] Total                           10% [===                 ] 543.2 kB/s | 543.2 MB | -12m34s


DownloadProgressBar::DownloadProgressBar(int64_t download_size, const std::string & description)
    : ProgressBar(download_size, description) {
    number_widget.set_bar(this);
    description_widget.set_bar(this);
    percent_widget.set_bar(this);
    progress_widget.set_bar(this);
    speed_widget.set_bar(this);
    size_widget.set_bar(this);
    time_widget.set_bar(this);
}


static std::size_t get_bar_width(const std::vector<Widget *> & widgets) {
    std::size_t result = 0;
    for (Widget * widget : widgets) {
        result += widget->get_total_width();
    }
    return result;
}


std::ostream & operator<<(std::ostream & stream, DownloadProgressBar & bar) {
    bar.to_stream(stream);
    return stream;
}


void DownloadProgressBar::to_stream(std::ostream & stream) {
    update();

    if (!is_finished() && !tty::is_interactive()) {
        // don't spam non-interactive terminal with progressbar's progress
        // only print the final form
        return;
    }

    // set the default delimiters
    number_widget.set_delimiter_before("");
    description_widget.set_delimiter_before(" ");
    percent_widget.set_delimiter_before(" ");
    progress_widget.set_delimiter_before(" ");
    speed_widget.set_delimiter_before(" | ");
    size_widget.set_delimiter_before(" | ");
    time_widget.set_delimiter_before(" | ");

    // set the default description widget width
    // we'll increase the size if terminal width allows
    description_widget.set_width(21);

    // create vector of all widgets
    std::vector<Widget *> widgets = {
        &number_widget,
        &description_widget,
        &percent_widget,
        &progress_widget,
        &speed_widget,
        &size_widget,
        &time_widget,
    };

    // remove widgets that are not visible
    for (auto it = widgets.begin(); it != widgets.end();) {
        if ((*it)->get_visible()) {
            it++;
        } else {
            it = widgets.erase(it);
        }
    }

    std::size_t terminal_width = static_cast<std::size_t>(tty::get_width());

    // if bar doesn't fit terminal width, hide progress widget
    std::size_t bar_width = get_bar_width(widgets);
    if (bar_width > terminal_width) {
        widgets.erase(std::remove(widgets.begin(), widgets.end(), &progress_widget), widgets.end());
        bar_width = get_bar_width(widgets);
    }

    // if bar doesn't fit terminal width, hide speed widget
    if (bar_width > terminal_width) {
        widgets.erase(std::remove(widgets.begin(), widgets.end(), &speed_widget), widgets.end());
        speed_widget.set_delimiter_before(" ");
        bar_width = get_bar_width(widgets);
    }

    // if bar doesn't fit terminal width, hide time widget
    if (bar_width > terminal_width) {
        widgets.erase(std::remove(widgets.begin(), widgets.end(), &time_widget), widgets.end());
        bar_width = get_bar_width(widgets);
    }

    // if bar is finished, hide the progress widget
    if (get_state() != ProgressBarState::STARTED) {
        widgets.erase(std::remove(widgets.begin(), widgets.end(), &progress_widget), widgets.end());
        bar_width = get_bar_width(widgets);
    }

    // if bar doesn't fit terminal width, reduce description width
    if (bar_width > terminal_width) {
        description_widget.set_total_width(description_widget.get_total_width() + terminal_width - bar_width);
        bar_width = get_bar_width(widgets);
    }

    if (bar_width < terminal_width) {
        description_widget.set_total_width(description_widget.get_total_width() + terminal_width - bar_width);
        bar_width = get_bar_width(widgets);
    }

    bool color_used = false;

    if (tty::is_coloring_enabled()) {
        switch (get_state()) {
            case ProgressBarState::READY:
            case ProgressBarState::STARTED:
            case ProgressBarState::SUCCESS:
                break;
            case ProgressBarState::WARNING:
                stream << tty::yellow;
                color_used = true;
                break;
            case ProgressBarState::ERROR:
                stream << tty::red;
                color_used = true;
                break;
        }
    }

    for (Widget * widget : widgets) {
        stream << std::left;
        // if string is shorter, fill the widget's space with spaces
        stream << std::setw(static_cast<int>(widget->get_total_width()));
        // print only the part of the string that fits the widget width
        stream << widget->to_string().substr(0, widget->get_total_width());
    }

    if (color_used) {
        stream << tty::reset;
    }

    for (auto & msg : get_messages()) {
        auto message_type = msg.first;
        auto message = msg.second;

        const auto & prefix = ">>> ";
        const auto prefix_width = libdnf5::cli::utils::utf8::width(prefix);

        stream << std::endl;
        // print only part of the prefix that fits the terminal width
        stream << libdnf5::cli::utils::utf8::substr_width(prefix, 0, terminal_width);

        if (prefix_width < terminal_width) {
            // only proceed if there is at least some space for the message
            color_used = false;
            if (tty::is_coloring_enabled()) {
                // color the message in interactive terminal
                switch (message_type) {
                    case MessageType::INFO:
                        break;
                    case MessageType::SUCCESS:
                        stream << tty::green;
                        color_used = true;
                        break;
                    case MessageType::WARNING:
                        stream << tty::yellow;
                        color_used = true;
                        break;
                    case MessageType::ERROR:
                        stream << tty::red;
                        color_used = true;
                        break;
                }
            }

            // Add padding to fully fill the terminal_width, this is because MultiProgressBar
            // overrides its own messages, it doesn't clear the lines.
            // If the message is short some leftover characters could be still present after it.
            const auto message_width = libdnf5::cli::utils::utf8::width(message);
            const auto space_available = terminal_width - prefix_width;
            if (message_width < space_available) {
                message.append(space_available - message_width, ' ');
            }

            // print only part of the message that fits the terminal width
            stream << libdnf5::cli::utils::utf8::substr_width(message, 0, space_available);

            if (color_used) {
                stream << tty::reset;
            }
        }
    }
}


}  // namespace libdnf5::cli::progressbar
