// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef LIBDNF5_RPM_RPM_LOG_GUARD_HPP
#define LIBDNF5_RPM_RPM_LOG_GUARD_HPP

#include "libdnf5/base/base.hpp"
#include "libdnf5/logger/logger.hpp"

#include <mutex>


namespace libdnf5::rpm {

class RpmLogGuardBase {
public:
    RpmLogGuardBase() : rpm_log_mutex_guard(rpm_log_mutex) {}
    virtual ~RpmLogGuardBase();

private:
    static std::mutex rpm_log_mutex;
    std::lock_guard<std::mutex> rpm_log_mutex_guard;
};

class RpmLogGuard : public RpmLogGuardBase {
public:
    RpmLogGuard(const BaseWeakPtr & base);
    ~RpmLogGuard();
    BaseWeakPtr & get_base();

    /// Add new RPM message to the buffer.
    void add_rpm_log(std::string_view log);

    /// Retrieve RPM messages that have been received since the last call of this method.
    std::vector<std::string> extract_rpm_logs_buffer();

private:
    int old_log_mask{0};
    std::vector<std::string> rpm_logs_buffer{};
    BaseWeakPtr base;
};

class RpmLogGuardStrings : public RpmLogGuardBase {
public:
    RpmLogGuardStrings();
    ~RpmLogGuardStrings() {};

    const std::vector<std::string> & get_rpm_logs() const { return rpm_logs; }
    void add_rpm_log(const std::string & log) { rpm_logs.emplace_back(log); }

private:
    std::vector<std::string> rpm_logs{};
};

}  // namespace libdnf5::rpm

#endif
