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

#include "rpm_log_guard.hpp"

#include <rpm/rpmlog.h>


namespace libdnf5::rpm {

std::mutex RpmLogGuardBase::rpm_log_mutex;

static int rpmlog_callback(rpmlogRec rec, rpmlogCallbackData data) {
    Logger::Level level = Logger::Level::DEBUG;  // default for the case of an unknown value below
    switch (rpmlogRecPriority(rec)) {
        case RPMLOG_EMERG:
        case RPMLOG_ALERT:
        case RPMLOG_CRIT:
            level = Logger::Level::CRITICAL;
            break;
        case RPMLOG_ERR:
            level = Logger::Level::ERROR;
            break;
        case RPMLOG_WARNING:
            level = Logger::Level::WARNING;
            break;
        case RPMLOG_NOTICE:
            level = Logger::Level::NOTICE;
            break;
        case RPMLOG_INFO:
            level = Logger::Level::INFO;
            break;
        case RPMLOG_DEBUG:
            // TRACE fits better as rpm DEBUG is quite low-level
            level = Logger::Level::TRACE;
            break;
    }

    std::string_view msg(rpmlogRecMessage(rec));
    if (!msg.empty() && msg[msg.length() - 1] == '\n') {
        msg.remove_suffix(1);
    }


    RpmLogGuard * log_guard = static_cast<RpmLogGuard *>(data);
    log_guard->add_rpm_log(msg);
    auto & logger = *log_guard->get_base()->get_logger();
    logger.log(level, "[rpm] {}", msg);
    return 0;
}

RpmLogGuardBase::~RpmLogGuardBase() {
    // Reset the RPM log callback to nullptr to prevent undefined behavior if the
    // callback data retains a reference to the RpmLogGuard object after it has
    // been destroyed.
    // The class doesn't reset the original callback upon destruction, because
    // although `rpmlogSetCallback()` returns the old callback, there's no way
    // to also retrieve the `rpmlogCallbackData` associated with it, so the
    // data can't be reset.
    rpmlogSetCallback(nullptr, nullptr);
}

RpmLogGuard::RpmLogGuard(const BaseWeakPtr & base) : RpmLogGuardBase(), base(base) {
    rpmlogSetCallback(&rpmlog_callback, this);

    // TODO(lukash) once Logger supports log mask, propagate to rpm
    // logs everything up to RPMLOG_INFO -> everything except RPMLOG_DEBUG, which is fairly low-level
    // and causes librpm to not remove auxiliary temporary files.
    old_log_mask = rpmlogSetMask(RPMLOG_UPTO(RPMLOG_PRI(RPMLOG_INFO)));
}


RpmLogGuard::~RpmLogGuard() {
    rpmlogSetMask(old_log_mask);
}

BaseWeakPtr & RpmLogGuard::get_base() {
    return base;
}

void RpmLogGuard::add_rpm_log(std::string_view log) {
    rpm_logs_buffer.emplace_back(log);
}

std::vector<std::string> RpmLogGuard::extract_rpm_logs_buffer() {
    auto retval = std::move(rpm_logs_buffer);
    rpm_logs_buffer.clear();
    return retval;
}

static int rpmlog_callback_strings(rpmlogRec rec, rpmlogCallbackData data) {
    std::string msg(rpmlogRecMessage(rec));
    if (!msg.empty() && msg[msg.length() - 1] == '\n') {
        msg.pop_back();
    }

    static_cast<RpmLogGuardStrings *>(data)->add_rpm_log(msg);
    return 0;
}

RpmLogGuardStrings::RpmLogGuardStrings() : RpmLogGuardBase() {
    rpmlogSetCallback(&rpmlog_callback_strings, this);
}

}  // namespace libdnf5::rpm
