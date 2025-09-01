// Copyright Contributors to the DNF5 project.
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

#ifndef DNF5DAEMON_CLIENT_CALLBACKS_HPP
#define DNF5DAEMON_CLIENT_CALLBACKS_HPP

#include <libdnf5-cli/progressbar/download_progress_bar.hpp>
#include <libdnf5-cli/progressbar/multi_progress_bar.hpp>
#include <libdnf5/conf/option_bool.hpp>
#include <sdbus-c++/sdbus-c++.h>

#include <string>

namespace dnfdaemon::client {

class Context;

class DbusCallback {
public:
    explicit DbusCallback(Context & context, sdbus::IConnection & connection);
    virtual ~DbusCallback() = default;
    virtual void register_signals() = 0;

protected:
    bool signature_valid(sdbus::Signal & signal);
    sdbus::ObjectPath session_object_path;
    std::unique_ptr<sdbus::IProxy> session_proxy;
};


class DownloadCB final : public DbusCallback {
public:
    explicit DownloadCB(Context & context, sdbus::IConnection & connection);
    virtual ~DownloadCB() = default;

    void add_new_download(sdbus::Signal & signal);
    void progress(sdbus::Signal & signal);
    void end(sdbus::Signal & signal);
    void mirror_failure(sdbus::Signal & signal);
    void key_import(sdbus::Signal & signal);

    void reset_progress_bar();
    void set_number_widget_visible(bool value);
    void set_show_total_bar_limit(std::size_t limit);

    // methods required by cli::utils::userconfirm::userconfirm
    libdnf5::OptionBool get_assumeno_option() const { return assume_no; }
    libdnf5::OptionBool get_assumeyes_option() const { return assume_yes; }
    libdnf5::OptionBool get_defaultyes_option() const { return default_yes; }

    void register_signals() override;

private:
    libdnf5::cli::progressbar::DownloadProgressBar * find_progress_bar(const std::string & download_id);
    void print();

    bool printed{false};
    bool number_widget_visible{false};
    std::size_t show_total_bar_limit{static_cast<std::size_t>(-1)};
    std::unique_ptr<libdnf5::cli::progressbar::MultiProgressBar> multi_progress_bar;
    // map {download_id: progressbar}
    std::unordered_map<std::string, libdnf5::cli::progressbar::DownloadProgressBar *> progress_bars;
    libdnf5::OptionBool assume_yes{false};
    libdnf5::OptionBool assume_no{false};
    libdnf5::OptionBool default_yes{false};
};


class TransactionCB final : public DbusCallback {
public:
    explicit TransactionCB(Context & context, sdbus::IConnection & connection);
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

    void register_signals() override;

private:
    libdnf5::cli::progressbar::MultiProgressBar multi_progress_bar;
    libdnf5::cli::progressbar::DownloadProgressBar * active_progress_bar{nullptr};

    void new_progress_bar(uint64_t total, const std::string & description);
};

}  // namespace dnfdaemon::client

#endif
