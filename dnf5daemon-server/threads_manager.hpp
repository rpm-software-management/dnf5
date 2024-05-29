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

#ifndef DNF5DAEMON_SERVER_THREADS_MANAGER_HPP
#define DNF5DAEMON_SERVER_THREADS_MANAGER_HPP

#include "dbus.hpp"
#include "utils.hpp"

#include <fmt/format.h>
#include <locale.h>
#include <sdbus-c++/sdbus-c++.h>

#include <atomic>
#include <iostream>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <vector>

class ThreadsManager {
public:
    ThreadsManager();
    virtual ~ThreadsManager();
    void register_thread(std::thread && thread);
    void mark_thread_finished(std::thread::id thread_id);
    void current_thread_finished() { mark_thread_finished(std::this_thread::get_id()); };
    void join_threads(const bool only_finished);
    void finish();

    template <class S>
    void handle_method(
        S & service,
        sdbus::MethodReply (S::*method)(sdbus::MethodCall &),
        sdbus::MethodCall & call,
        std::optional<std::string> thread_locale = std::nullopt) {
        auto worker = std::thread(
            [this](
                S & service,
                sdbus::MethodReply (S::*method)(sdbus::MethodCall &),
                sdbus::MethodCall call,
                std::optional<std::string> thread_locale = std::nullopt) {
                locale_t new_locale{nullptr};
                locale_t orig_locale{nullptr};
                if (thread_locale) {
                    orig_locale = set_thread_locale(thread_locale.value(), new_locale);
                }

                sdbus::MethodReply reply;
                try {
                    reply = (service.*method)(call);
                } catch (const sdbus::Error & ex) {
                    reply = call.createErrorReply(ex);
                } catch (const std::exception & ex) {
                    reply = call.createErrorReply(sdbus::Error(dnfdaemon::ERROR, ex.what()));
                } catch (...) {
                    reply = call.createErrorReply(sdbus::Error(dnfdaemon::ERROR, "Unknown exception caught"));
                }
                bool success = false;
                std::string error_msg;
                try {
                    reply.send();
                    success = true;
                } catch (const std::exception & e) {
                    error_msg = e.what();
                } catch (...) {
                    error_msg = "Unknown exception caught";
                }
                if (!success) {
                    std::cerr << fmt::format(
                                     "Error sending D-Bus reply to {}:{}() call: {}",
                                     call.getInterfaceName(),
                                     call.getMemberName(),
                                     error_msg)
                              << std::endl;
                }

                if (thread_locale) {
                    uselocale(orig_locale);
                    freelocale(new_locale);
                }
                current_thread_finished();
            },
            std::ref(service),
            method,
            call,
            thread_locale);
        register_thread(std::move(worker));
    }

    template <class S>
    void handle_method_fd(
        S & service,
        void (S::*method)(sdbus::MethodCall &, const std::string &),
        sdbus::MethodCall & call,
        std::optional<std::string> thread_locale = std::nullopt) {
        auto worker = std::thread(
            [this](
                S & service,
                void (S::*method)(sdbus::MethodCall &, const std::string &),
                sdbus::MethodCall call,
                std::optional<std::string> thread_locale = std::nullopt) {
                static unsigned int counter{0};
                locale_t new_locale{nullptr};
                locale_t orig_locale{nullptr};
                if (thread_locale) {
                    orig_locale = set_thread_locale(thread_locale.value(), new_locale);
                }

                sdbus::MethodReply reply = call.createReply();
                // generate unique transfer id based on client bus name and counter
                const std::string transfer_id = fmt::format("{}-{}", call.getSender(), ++counter);
                reply << transfer_id;

                std::string error_msg;
                try {
                    reply.send();
                } catch (const std::exception & e) {
                    error_msg = e.what();
                } catch (...) {
                    error_msg = "Unknown exception caught";
                }

                if (error_msg.empty()) {
                    try {
                        (service.*method)(call, transfer_id);
                    } catch (...) {
                        // TODO(mblaha): log the error
                    }
                } else {
                    std::cerr << fmt::format(
                                     "Error sending D-Bus reply to {}:{}() call: {}",
                                     call.getInterfaceName(),
                                     call.getMemberName(),
                                     error_msg)
                              << std::endl;
                }

                if (thread_locale) {
                    uselocale(orig_locale);
                    freelocale(new_locale);
                }

                current_thread_finished();
            },
            std::ref(service),
            method,
            call,
            thread_locale);
        register_thread(std::move(worker));
    }

    template <class S>
    void handle_signal(S & service, void (S::*method)(sdbus::Signal &), sdbus::Signal & signal) {
        auto worker = std::thread(
            [this](S & service, void (S::*method)(sdbus::Signal &), sdbus::Signal signal) {
                bool success = false;
                std::string error_msg;
                try {
                    (service.*method)(signal);
                    success = true;
                } catch (const std::exception & ex) {
                    error_msg = ex.what();
                } catch (...) {
                    error_msg = "Unknown exception caught";
                }
                if (!success) {
                    std::cerr << fmt::format(
                                     "Error handling signal {}:{}: {}",
                                     signal.getInterfaceName(),
                                     signal.getMemberName(),
                                     error_msg)
                              << std::endl;
                }
                current_thread_finished();
            },
            std::ref(service),
            method,
            signal);
        register_thread(std::move(worker));
    }

private:
    std::mutex running_threads_mutex;
    // flag whether to break the finished threads collector infinite loop
    std::atomic<bool> finish_collector{false};
    // thread that joins finished worker threads
    std::thread running_threads_collector;
    // vector of started worker threads
    std::vector<std::thread> running_threads{};
    // vector of finished threads id
    std::vector<std::thread::id> finished_threads{};
    static locale_t set_thread_locale(const std::string & thread_locale, locale_t & new_locale);
};

#endif
