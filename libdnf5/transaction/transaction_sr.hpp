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

#ifndef LIBDNF5_TRANSACTION_TRANSACTION_REPLAY_HPP
#define LIBDNF5_TRANSACTION_TRANSACTION_REPLAY_HPP

#include "libdnf5/transaction/transaction.hpp"

#include <libdnf5/comps/environment/environment.hpp>
#include <libdnf5/comps/group/group.hpp>
#include <libdnf5/transaction/transaction_item_action.hpp>

#include <filesystem>


namespace libdnf5::transaction {

class TransactionReplayError : public Error {
public:
    using Error::Error;
    const char * get_domain_name() const noexcept override { return "libdnf5::transaction"; }
    const char * get_name() const noexcept override { return "TransactionReplayError"; }
};


struct GroupReplay {
    TransactionItemAction action;
    TransactionItemReason reason;
    std::string group_id;
    // Path to serialized comps group relative to the transaction json file
    std::filesystem::path group_path;
    std::string repo_id;
    libdnf5::comps::PackageType package_types;
};

struct EnvironmentReplay {
    TransactionItemAction action;
    std::string environment_id;
    // Path to serialized comps environment relative to the transaction json file
    std::filesystem::path environment_path;
    std::string repo_id;
};

struct PackageReplay {
    TransactionItemAction action;
    TransactionItemReason reason;
    std::string group_id;
    // This nevra doesn't contain epoch if it is 0
    std::string nevra;
    // Path to rpm package relative to the transaction json file
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
TransactionReplay to_replay(libdnf5::transaction::Transaction & trans);

}  // namespace libdnf5::transaction

#endif  // LIBDNF5_TRANSACTION_TRANSACTION_REPLAY_HPP
