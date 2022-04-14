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
#include "db/console_output.hpp"
#include "db/db.hpp"
#include "db/rpm.hpp"
#include "db/trans.hpp"
#include "db/trans_item.hpp"
#include "db/trans_with.hpp"
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


const std::set<std::string> & Transaction::get_runtime_packages() {
    if (runtime_packages) {
        return *runtime_packages;
    }

    runtime_packages = load_transaction_runtime_packages(*transaction_db_connect(*base), *this);
    return *runtime_packages;
}


void Transaction::add_runtime_package(const std::string & nevra) {
    if (!runtime_packages) {
        runtime_packages.emplace();
    }

    runtime_packages->insert(nevra);
}


const std::vector<std::pair<int, std::string>> & Transaction::get_console_output() {
    if (console_output) {
        return *console_output;
    }

    console_output = console_output_load(*transaction_db_connect(*base), *this);
    return *console_output;
}


void Transaction::add_console_output_line(int file_descriptor, const std::string & line) {
    if (!get_id()) {
        throw RuntimeError(M_("Cannot add console output to unsaved transaction"));
    }

    if (!console_output) {
        console_output.emplace();
    }

    auto conn = transaction_db_connect(*base);

    // save the line to the database
    console_output_insert_line(*conn, *this, file_descriptor, line);

    // also store the line in the console_output vector
    console_output->emplace_back(file_descriptor, line);
}


std::vector<CompsEnvironment> & Transaction::get_comps_environments() {
    if (comps_environments) {
        return *comps_environments;
    }

    comps_environments = get_transaction_comps_environments(*transaction_db_connect(*base), *this);
    return *comps_environments;
}


CompsEnvironment & Transaction::new_comps_environment() {
    if (!comps_environments) {
        comps_environments.emplace();
    }

    return comps_environments->emplace_back(*this);
}


std::vector<CompsGroup> & Transaction::get_comps_groups() {
    if (comps_groups) {
        return *comps_groups;
    }

    comps_groups = get_transaction_comps_groups(*transaction_db_connect(*base), *this);
    return *comps_groups;
}


CompsGroup & Transaction::new_comps_group() {
    if (!comps_groups) {
        comps_groups.emplace();
    }

    return comps_groups->emplace_back(*this);
}


std::vector<Package> & Transaction::get_packages() {
    if (packages) {
        return *packages;
    }

    packages = get_transaction_packages(*transaction_db_connect(*base), *this);
    return *packages;
}


Package & Transaction::new_package() {
    if (!packages) {
        packages.emplace();
    }

    return packages->emplace_back(*this);
}


void Transaction::fill_transaction_packages(
    const std::vector<libdnf::base::TransactionPackage> & transaction_packages) {
    for (auto & tspkg : transaction_packages) {
        auto & new_pkg = new_package();
        libdnf::rpm::copy_nevra_attributes(tspkg.get_package(), new_pkg);
        new_pkg.set_repoid(tspkg.get_package().get_repo_id());
        new_pkg.set_action(tspkg.get_action());
        new_pkg.set_reason(tspkg.get_reason());
    }
}


void Transaction::start() {
    if (id != 0) {
        throw RuntimeError(M_("Transaction has already started!"));
    }

    auto conn = transaction_db_connect(*base);
    conn->exec("BEGIN");
    try {
        auto query = trans_insert_new_query(*conn);
        trans_insert(*query, *this);

        save_transaction_runtime_packages(*conn, *this);

        insert_transaction_comps_environments(*conn, *this);
        insert_transaction_comps_groups(*conn, *this);
        insert_transaction_packages(*conn, *this);
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
        if (i->get_state() == TransactionItemState::UNKNOWN) {
            throw std::runtime_error(
                fmt::format(_("TransactionItem state is not set: {}"), i->getItem()->toStr()));
        }
    }
    */

    auto conn = transaction_db_connect(*base);
    conn->exec("BEGIN");
    try {
        set_state(state);
        auto query = trans_update_new_query(*conn);
        trans_update(*query, *this);
        conn->exec("COMMIT");
    } catch (...) {
        conn->exec("ROLLBACK");
        throw;
    }
}

}  // namespace libdnf::transaction
