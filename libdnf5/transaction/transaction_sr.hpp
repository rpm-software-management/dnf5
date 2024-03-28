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

#include <libdnf5/comps/environment/environment.hpp>
#include <libdnf5/comps/group/group.hpp>
#include <libdnf5/transaction/transaction_item_action.hpp>

#include <filesystem>


namespace libdnf5::transaction {


struct GroupReplay {
    TransactionItemAction action;
    TransactionItemReason reason;
    std::string group_id;
    std::filesystem::path group_path;
    std::string repo_id;
};

struct EnvironmentReplay {
    TransactionItemAction action;
    std::string environment_id;
    std::filesystem::path environment_path;
    std::string repo_id;
};

struct PackageReplay {
    TransactionItemAction action;
    TransactionItemReason reason;
    std::string group_id;
    std::string nevra;
    std::filesystem::path package_path;
    std::string repo_id;
};

struct TransactionReplay {
    std::vector<GroupReplay> groups;
    std::vector<EnvironmentReplay> environments;
    std::vector<PackageReplay> packages;
    //TODO(amatej): ModuleActions
};

TransactionReplay parse_transaction_replay(const std::string & json_serialized_transaction);
std::string json_serialize(const TransactionReplay & transaction_replay);

}  // namespace libdnf5::transaction

#endif  // LIBDNF5_TRANSACTION_TRANSACTION_REPLAY_HPP
