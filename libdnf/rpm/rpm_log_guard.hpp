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

#ifndef LIBDNF_RPM_RPM_LOG_GUARD_HPP
#define LIBDNF_RPM_RPM_LOG_GUARD_HPP

#include "libdnf/base/base.hpp"
#include "libdnf/logger/logger.hpp"

#include <mutex>


namespace libdnf::rpm {

class RpmLogGuard {
public:
    RpmLogGuard(const BaseWeakPtr & base);
    ~RpmLogGuard();

private:
    static std::mutex rpm_log_mutex;

    std::lock_guard<std::mutex> rpm_log_mutex_guard;
    int old_log_mask{0};
};

}  // namespace libdnf::rpm

#endif
