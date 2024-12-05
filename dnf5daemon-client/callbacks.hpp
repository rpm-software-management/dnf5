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

#ifndef DNF5DAEMON_CLIENT_CALLBACKS_HPP
#define DNF5DAEMON_CLIENT_CALLBACKS_HPP

#include <libdnf5-cli/progressbar/download_progress_bar.hpp>
#include <libdnf5-cli/progressbar/multi_progress_bar.hpp>
#include <sdbus-c++/sdbus-c++.h>

#include <string>

namespace dnfdaemon::client {

class Context;

class DbusCallback {
public:
    explicit DbusCallback(Context & context) : context(context) {};
    virtual ~DbusCallback() = default;

protected:
    Context & context;

    bool signature_valid(sdbus::Signal & signal);
};


class DownloadCB final : public DbusCallback {
public:
    explicit DownloadCB(Context & context);
    virtual ~DownloadCB() = default;

    void add_new_download(sdbus::Signal & signal);
    void progress(sdbus::Signal & signal);
    void end(sdbus::Signal & signal);
    void mirror_failure(sdbus::Signal & signal);
    void key_import(sdbus::Signal & signal);

    void reset_progress_bar();
    void set_number_widget_visible(bool value);
    void set_show_total_bar_limit(std::size_t limit);

private:
    libdnf5::cli::progressbar::DownloadProgressBar * find_progress_bar(const std::string & download_id);
    void print();

    bool printed{false};
    bool number_widget_visible{false};
    std::size_t show_total_bar_limit{static_cast<std::size_t>(-1)};
    std::unique_ptr<libdnf5::cli::progressbar::MultiProgressBar> multi_progress_bar;
    // map {download_id: progressbar}
    std::unordered_map<std::string, libdnf5::cli::progressbar::DownloadProgressBar *> progress_bars;
};


class TransactionCB final : public DbusCallback {
public:
    explicit TransactionCB(Context & context);
    virtual ~TransactionCB() = default;

    void verify_start(sdbus::Signal & signal);
    void verify_end(sdbus::Signal & signal);
    void verify_progress(sdbus::Signal & signal);

    void transaction_start(sdbus::Signal & signal);
    void transaction_end(sdbus::Signal & signal);
    void transaction_progress(sdbus::Signal & signal);

    void action_start(sdbus::Signal & signal);
    void action_end(sdbus::Signal & signal);
    void action_progress(sdbus::Signal & signal);

    void script_start(sdbus::Signal & signal);
    void script_stop(sdbus::Signal & signal);
    void script_error(sdbus::Signal & signal);

    void unpack_error(sdbus::Signal & signal);

    void finished(sdbus::Signal & signal);

private:
    libdnf5::cli::progressbar::MultiProgressBar multi_progress_bar;
    libdnf5::cli::progressbar::DownloadProgressBar * active_progress_bar{nullptr};

    void new_progress_bar(uint64_t total, const std::string & description);
};

}  // namespace dnfdaemon::client

#endif
