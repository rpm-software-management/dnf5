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
