/*
Copyright (C) 2021 Red Hat, Inc.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef DNFDAEMON_CLIENT_WRAPPERS_DBUS_GOAL_WRAPPER_HPP
#define DNFDAEMON_CLIENT_WRAPPERS_DBUS_GOAL_WRAPPER_HPP

#include <vector>

#include <libdnf/transaction/transaction_item_action.hpp>

#include "dbus_package_wrapper.hpp"

using namespace libdnf::transaction;

namespace dnfdaemon::client {

class DbusGoalWrapper {
public:
    DbusGoalWrapper(std::vector<dnfdaemon::DbusTransactionItem>);

    std::vector<DbusPackageWrapper> list_rpm_installs() const { return rpm_installs; };
    std::vector<DbusPackageWrapper> list_rpm_reinstalls() const { return rpm_reinstalls; };
    std::vector<DbusPackageWrapper> list_rpm_upgrades() const { return rpm_upgrades; };
    std::vector<DbusPackageWrapper> list_rpm_downgrades() const { return rpm_downgrades; };
    std::vector<DbusPackageWrapper> list_rpm_removes() const { return rpm_removes; };
    std::vector<DbusPackageWrapper> list_rpm_obsoleted() const { return rpm_obsoletes; };

private:
    std::vector<DbusPackageWrapper> rpm_installs;
    std::vector<DbusPackageWrapper> rpm_reinstalls;
    std::vector<DbusPackageWrapper> rpm_upgrades;
    std::vector<DbusPackageWrapper> rpm_downgrades;
    std::vector<DbusPackageWrapper> rpm_removes;
    std::vector<DbusPackageWrapper> rpm_obsoletes;
};

}  // namespace dnfdaemon::client

#endif  // DNFDAEMON_CLIENT_WRAPEPRS_DBUS_GOAL_WRAPPER_HPP
