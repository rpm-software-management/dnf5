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


#include "libdnf/transaction/transaction.hpp"

#include "db/comps_environment.hpp"
#include "db/comps_group.hpp"
#include "db/db.hpp"
#include "db/rpm.hpp"
#include "db/trans.hpp"
#include "db/trans_item.hpp"
#include "utils/bgettext/bgettext-mark-domain.h"

#include "libdnf/transaction/comps_environment.hpp"
#include "libdnf/transaction/comps_group.hpp"
#include "libdnf/transaction/rpm_package.hpp"
#include "libdnf/transaction/transaction_item.hpp"

namespace libdnf::transaction {

std::string transaction_state_to_string(TransactionState state) {
    switch (state) {
        case TransactionState::STARTED:
            return "Started";
        case TransactionState::OK:
            return "Ok";
        case TransactionState::ERROR:
            return "Error";
    }
    return "";
}


TransactionState transaction_state_from_string(const std::string & state) {
    if (state == "Started") {
        return TransactionState::STARTED;
    } else if (state == "Ok") {
        return TransactionState::OK;
    } else if (state == "Error") {
        return TransactionState::ERROR;
    }

    throw InvalidTransactionState(state);
}


InvalidTransactionState::InvalidTransactionState(const std::string & state)
    : libdnf::Error(M_("Invalid transaction state: {}"), state) {}


Transaction::Transaction(const BaseWeakPtr & base, int64_t id) : id(id), base(base) {}


Transaction::Transaction(const BaseWeakPtr & base) : base{base} {}


bool Transaction::operator==(const Transaction & other) const {
    return get_id() == other.get_id();
}


bool Transaction::operator<(const Transaction & other) const {
    return get_id() > other.get_id();
}


bool Transaction::operator>(const Transaction & other) const {
    return get_id() < other.get_id();
}


std::vector<CompsEnvironment> & Transaction::get_comps_environments() {
    if (comps_environments) {
        return *comps_environments;
    }

    comps_environments =
        CompsEnvironmentDbUtils::get_transaction_comps_environments(*transaction_db_connect(*base), *this);
    return *comps_environments;
}


CompsEnvironment & Transaction::new_comps_environment() {
    if (!comps_environments) {
        comps_environments.emplace();
    }

    CompsEnvironment comps_env(*this);
    return comps_environments->emplace_back(std::move(comps_env));
}


std::vector<CompsGroup> & Transaction::get_comps_groups() {
    if (comps_groups) {
        return *comps_groups;
    }

    comps_groups = CompsGroupDbUtils::get_transaction_comps_groups(*transaction_db_connect(*base), *this);
    return *comps_groups;
}


CompsGroup & Transaction::new_comps_group() {
    if (!comps_groups) {
        comps_groups.emplace();
    }

    CompsGroup comps_group(*this);
    return comps_groups->emplace_back(comps_group);
}


std::vector<Package> & Transaction::get_packages() {
    if (packages) {
        return *packages;
    }

    packages = RpmDbUtils::get_transaction_packages(*transaction_db_connect(*base), *this);
    return *packages;
}


Package & Transaction::new_package() {
    if (!packages) {
        packages.emplace();
    }

    Package const pkg(*this);
    return packages->emplace_back(pkg);
}


void Transaction::fill_transaction_packages(
    const std::vector<libdnf::base::TransactionPackage> & transaction_packages) {
    for (auto & tspkg : transaction_packages) {
        auto & new_pkg = new_package();
        auto source_pkg = tspkg.get_package();
        new_pkg.set_name(source_pkg.get_name());
        new_pkg.set_epoch(source_pkg.get_epoch());
        new_pkg.set_version(source_pkg.get_version());
        new_pkg.set_release(source_pkg.get_release());
        new_pkg.set_arch(source_pkg.get_arch());
        new_pkg.set_repoid(tspkg.get_package().get_repo_id());
        new_pkg.set_action(tspkg.get_action());
        new_pkg.set_reason(tspkg.get_reason());
    }
}


void Transaction::fill_transaction_groups(
    const std::vector<libdnf::base::TransactionGroup> & transaction_groups,
    const std::set<std::string> & installed_names) {
    for (auto & tsgrp : transaction_groups) {
        auto & new_grp = new_comps_group();
        auto group = tsgrp.get_group();
        new_grp.set_name(group.get_name());
        new_grp.set_action(tsgrp.get_action());
        new_grp.set_reason(tsgrp.get_reason());
        new_grp.set_translated_name(group.get_translated_name());

        libdnf::comps::PackageType package_types{0};
        for (const auto & group_package : group.get_packages()) {
            auto & new_grp_pkg = new_grp.new_package();
            auto name = group_package.get_name();
            new_grp_pkg.set_name(name);
            new_grp_pkg.set_package_type(group_package.get_type());
            new_grp_pkg.set_installed(installed_names.contains(name) ? true : false);
            package_types |= group_package.get_type();
        }

        new_grp.set_package_types(package_types);
    }
}

void Transaction::start() {
    if (id != 0) {
        throw RuntimeError(M_("Transaction has already started!"));
    }

    auto conn = transaction_db_connect(*base);
    conn->exec("BEGIN");
    try {
        auto query = TransactionDbUtils::trans_insert_new_query(*conn);
        TransactionDbUtils::trans_insert(*query, *this);

        CompsEnvironmentDbUtils::insert_transaction_comps_environments(*conn, *this);
        CompsGroupDbUtils::insert_transaction_comps_groups(*conn, *this);
        RpmDbUtils::insert_transaction_packages(*conn, *this);
        conn->exec("COMMIT");
    } catch (...) {
        conn->exec("ROLLBACK");
        throw;
    }
}


void Transaction::finish(TransactionState state) {
    // TODO(dmach): save item states
    /*
    // save states to the database before checking for UNKNOWN state
    for (auto i : getItems()) {
        i->saveState();
    }

    for (auto i : getItems()) {
        if (i->get_state() == TransactionItemState::STARTED) {
            throw std::runtime_error(
                fmt::format(_("TransactionItem state is not set: {}"), i->getItem()->toStr()));
        }
    }
    */

    auto conn = transaction_db_connect(*base);
    conn->exec("BEGIN");
    try {
        set_state(state);
        auto query = TransactionDbUtils::trans_update_new_query(*conn);
        TransactionDbUtils::trans_update(*query, *this);
        conn->exec("COMMIT");
    } catch (...) {
        conn->exec("ROLLBACK");
        throw;
    }
}

}  // namespace libdnf::transaction
