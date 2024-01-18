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


#include "libdnf5/transaction/transaction.hpp"

#include "db/comps_environment.hpp"
#include "db/comps_group.hpp"
#include "db/db.hpp"
#include "db/rpm.hpp"
#include "db/trans.hpp"
#include "transaction/transaction_sr.hpp"

#include "libdnf5/transaction/comps_environment.hpp"
#include "libdnf5/transaction/comps_group.hpp"
#include "libdnf5/transaction/rpm_package.hpp"
#include "libdnf5/transaction/transaction_item.hpp"
#include "libdnf5/utils/bgettext/bgettext-mark-domain.h"

namespace libdnf5::transaction {

class Transaction::Impl {
public:
    Impl(const BaseWeakPtr & base, int64_t id = 0);

private:
    friend Transaction;

    int64_t id{0};

    int64_t dt_begin = 0;
    int64_t dt_end = 0;
    std::string rpmdb_version_begin;
    std::string rpmdb_version_end;
    // TODO(dmach): move to a new "vars" table?
    std::string releasever;
    uint32_t user_id = 0;
    std::string description;
    std::string comment;
    TransactionState state = State::STARTED;

    std::optional<std::vector<std::pair<int, std::string>>> console_output;

    std::optional<std::vector<CompsEnvironment>> comps_environments;
    std::optional<std::vector<CompsGroup>> comps_groups;
    std::optional<std::vector<Package>> packages;

    BaseWeakPtr base;
};

Transaction::Impl::Impl(const BaseWeakPtr & base, int64_t id) : id(id), base(base) {}

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
    : libdnf5::Error(M_("Invalid transaction state: {}"), state) {}


Transaction::Transaction(const BaseWeakPtr & base, int64_t id) : p_impl(std::make_unique<Impl>(base, id)) {}


Transaction::Transaction(const BaseWeakPtr & base) : p_impl(std::make_unique<Impl>(base)) {}

Transaction::~Transaction() = default;

Transaction::Transaction(const Transaction & src) : p_impl(new Impl(*src.p_impl)) {}
Transaction::Transaction(Transaction && src) noexcept = default;

Transaction & Transaction::operator=(const Transaction & src) {
    if (this != &src) {
        if (p_impl) {
            *p_impl = *src.p_impl;
        } else {
            p_impl = std::make_unique<Impl>(*src.p_impl);
        }
    }

    return *this;
}
Transaction & Transaction::operator=(Transaction && src) noexcept = default;

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
    if (p_impl->comps_environments) {
        return *p_impl->comps_environments;
    }

    p_impl->comps_environments =
        CompsEnvironmentDbUtils::get_transaction_comps_environments(*transaction_db_connect(*p_impl->base), *this);
    return *p_impl->comps_environments;
}


CompsEnvironment & Transaction::new_comps_environment() {
    if (!p_impl->comps_environments) {
        p_impl->comps_environments.emplace();
    }

    CompsEnvironment comps_env(*this);
    return p_impl->comps_environments->emplace_back(std::move(comps_env));
}


std::vector<CompsGroup> & Transaction::get_comps_groups() {
    if (p_impl->comps_groups) {
        return *p_impl->comps_groups;
    }

    p_impl->comps_groups =
        CompsGroupDbUtils::get_transaction_comps_groups(*transaction_db_connect(*p_impl->base), *this);
    return *p_impl->comps_groups;
}


CompsGroup & Transaction::new_comps_group() {
    if (!p_impl->comps_groups) {
        p_impl->comps_groups.emplace();
    }

    CompsGroup comps_group(*this);
    return p_impl->comps_groups->emplace_back(comps_group);
}


std::vector<Package> & Transaction::get_packages() {
    if (p_impl->packages) {
        return *p_impl->packages;
    }

    p_impl->packages = RpmDbUtils::get_transaction_packages(*transaction_db_connect(*p_impl->base), *this);
    return *p_impl->packages;
}


Package & Transaction::new_package() {
    if (!p_impl->packages) {
        p_impl->packages.emplace();
    }

    Package const pkg(*this);
    return p_impl->packages->emplace_back(pkg);
}


void Transaction::fill_transaction_packages(
    const std::vector<libdnf5::base::TransactionPackage> & transaction_packages) {
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


void Transaction::fill_transaction_environments(
    const std::vector<libdnf5::base::TransactionEnvironment> & transaction_environments,
    const std::set<std::string> & installed_group_ids) {
    for (auto & tsenv : transaction_environments) {
        auto & new_env = new_comps_environment();
        auto environment = tsenv.get_environment();
        new_env.set_environment_id(environment.get_environmentid());
        new_env.set_name(environment.get_name());
        new_env.set_translated_name(environment.get_translated_name());
        libdnf5::comps::PackageType package_types{libdnf5::comps::PackageType::MANDATORY};
        if (tsenv.get_with_optional()) {
            package_types |= libdnf5::comps::PackageType::OPTIONAL;
        }
        new_env.set_package_types(package_types);
        if (!tsenv.get_environment().get_repos().empty()) {
            new_env.set_repoid(*tsenv.get_environment().get_repos().begin());
        }
        for (const auto & group_id : environment.get_groups()) {
            auto & new_env_grp = new_env.new_group();
            new_env_grp.set_group_id(group_id);
            new_env_grp.set_group_type(libdnf5::comps::PackageType::MANDATORY);
            new_env_grp.set_installed(installed_group_ids.contains(group_id));
        }
        for (const auto & group_id : environment.get_optional_groups()) {
            auto & new_env_grp = new_env.new_group();
            new_env_grp.set_group_id(group_id);
            new_env_grp.set_group_type(libdnf5::comps::PackageType::OPTIONAL);
            new_env_grp.set_installed(installed_group_ids.contains(group_id));
        }
        new_env.set_action(tsenv.get_action());
        new_env.set_reason(tsenv.get_reason());
    }
}


void Transaction::fill_transaction_groups(
    const std::vector<libdnf5::base::TransactionGroup> & transaction_groups,
    const std::set<std::string> & installed_names) {
    for (auto & tsgrp : transaction_groups) {
        auto & new_grp = new_comps_group();
        auto group = tsgrp.get_group();
        new_grp.set_group_id(group.get_groupid());
        new_grp.set_name(group.get_name());
        new_grp.set_action(tsgrp.get_action());
        new_grp.set_reason(tsgrp.get_reason());
        new_grp.set_translated_name(group.get_translated_name());
        if (!tsgrp.get_group().get_repos().empty()) {
            new_grp.set_repoid(*tsgrp.get_group().get_repos().begin());
        }

        libdnf5::comps::PackageType package_types{0};
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
    if (p_impl->id != 0) {
        throw RuntimeError(M_("Transaction has already started!"));
    }

    auto conn = transaction_db_connect(*p_impl->base);
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

    auto conn = transaction_db_connect(*p_impl->base);
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

std::string Transaction::serialize() {
    TransactionReplay transaction_replay;

    for (const auto & pkg : get_packages()) {
        PackageReplay package_replay;

        package_replay.nevra = pkg.to_string();
        package_replay.action = pkg.get_action();
        package_replay.reason = pkg.get_reason();
        package_replay.repo_id = pkg.get_repoid();
        //TODO(amatej): Add the group_id for reason change?

        transaction_replay.packages.push_back(package_replay);
    }

    for (const auto & group : get_comps_groups()) {
        GroupReplay group_replay;

        group_replay.group_id = group.to_string();
        group_replay.action = group.get_action();
        group_replay.reason = group.get_reason();
        group_replay.repo_id = group.get_repoid();

        transaction_replay.groups.push_back(group_replay);
    }

    for (const auto & environment : get_comps_environments()) {
        EnvironmentReplay environment_replay;

        environment_replay.environment_id = environment.to_string();
        environment_replay.action = environment.get_action();
        environment_replay.repo_id = environment.get_repoid();

        transaction_replay.environments.push_back(environment_replay);
    }

    ////TODO(amatej): potentially add modules

    return json_serialize(transaction_replay);
}


// Getters
int64_t Transaction::get_id() const noexcept {
    return p_impl->id;
}
int64_t Transaction::get_dt_start() const noexcept {
    return p_impl->dt_begin;
}
int64_t Transaction::get_dt_end() const noexcept {
    return p_impl->dt_end;
}
const std::string & Transaction::get_rpmdb_version_begin() const noexcept {
    return p_impl->rpmdb_version_begin;
}
const std::string & Transaction::get_rpmdb_version_end() const noexcept {
    return p_impl->rpmdb_version_end;
}
const std::string & Transaction::get_releasever() const noexcept {
    return p_impl->releasever;
}
uint32_t Transaction::get_user_id() const noexcept {
    return p_impl->user_id;
}
const std::string & Transaction::get_description() const noexcept {
    return p_impl->description;
}
const std::string & Transaction::get_comment() const noexcept {
    return p_impl->comment;
}
TransactionState Transaction::get_state() const noexcept {
    return p_impl->state;
}


// Setters
void Transaction::set_id(int64_t value) {
    p_impl->id = value;
}
void Transaction::set_comment(const std::string & value) {
    p_impl->comment = value;
}
void Transaction::set_dt_start(int64_t value) {
    p_impl->dt_begin = value;
}
void Transaction::set_dt_end(int64_t value) {
    p_impl->dt_end = value;
}
void Transaction::set_description(const std::string & value) {
    p_impl->description = value;
}
void Transaction::set_user_id(uint32_t value) {
    p_impl->user_id = value;
}
void Transaction::set_releasever(const std::string & value) {
    p_impl->releasever = value;
}
void Transaction::set_rpmdb_version_end(const std::string & value) {
    p_impl->rpmdb_version_end = value;
}
void Transaction::set_rpmdb_version_begin(const std::string & value) {
    p_impl->rpmdb_version_begin = value;
}
void Transaction::set_state(State value) {
    p_impl->state = value;
}

}  // namespace libdnf5::transaction
