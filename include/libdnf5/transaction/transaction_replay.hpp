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

#ifndef LIBDNF5_TRANSACTION_TRANSACTION_REPLAY_HPP
#define LIBDNF5_TRANSACTION_TRANSACTION_REPLAY_HPP

#include <libdnf5/base/base.hpp>
#include <libdnf5/base/goal.hpp>
#include <libdnf5/comps/environment/environment.hpp>
#include <libdnf5/comps/group/group.hpp>

#include <filesystem>


namespace libdnf5::transaction {

class TransactionReplay {
public:
    //TODO(amatej): document API
    TransactionReplay(const libdnf5::BaseWeakPtr & base, std::filesystem::path filename, bool ignore_installed);

    // Use fill_goal and fix_reason to resolve the transaction again
    void fill_goal(libdnf5::Goal & goal);
    void fix_reasons(libdnf5::base::Transaction * transaction);

    // Use create_transaction to directly create transaction without resolving
    libdnf5::base::Transaction create_transaction();

private:
    friend libdnf5::base::Transaction::Impl;

    struct GroupAction {
        std::string action;
        libdnf5::transaction::TransactionItemReason reason;
        libdnf5::comps::Group group;
        libdnf5::comps::PackageType pkg_types;
    };
    struct EnvironmentAction {
        std::string action;
        libdnf5::transaction::TransactionItemReason reason;
        libdnf5::comps::Environment environment;
        libdnf5::comps::PackageType pkg_types;
    };
    struct PackageAction {
        std::string action;
        libdnf5::transaction::TransactionItemReason reason;
        libdnf5::rpm::Package package;
    };

    //TODO(amatej): ModuleActions

    std::vector<GroupAction> group_actions;
    std::vector<EnvironmentAction> environment_actions;
    std::vector<PackageAction> package_actions;
    libdnf5::BaseWeakPtr base;
    std::unordered_map<std::string, TransactionItemReason> cached_reasons;
};

}  // namespace libdnf5::transaction

#endif  // LIBDNF5_TRANSACTION_TRANSACTION_REPLAY_HPP
