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


#include "libdnf5/transaction/transaction_item.hpp"

#include "libdnf5/transaction/transaction.hpp"
#include "libdnf5/transaction/transaction_item_action.hpp"


namespace libdnf5::transaction {

class TransactionItem::Impl {
public:
    Impl(const Transaction & trans);

private:
    friend TransactionItem;

    int64_t id = 0;
    Action action = Action::INSTALL;
    Reason reason = Reason::NONE;
    State state = State::STARTED;
    std::string repoid;
    int64_t item_id = 0;

    // TODO(lukash) this won't be safe in bindings (or in general when a
    // TransactionItem is kept around after a Transaction is destroyed), but we
    // can't easily use a WeakPtr here, since the Transactions are expected to
    // be at least movable, and the WeakPtrGuard would make the Transaction
    // unmovable
    const Transaction * trans = nullptr;
};

TransactionItem::~TransactionItem() = default;

TransactionItem::TransactionItem(const TransactionItem & mpkg) : p_impl(new Impl(*mpkg.p_impl)) {}
TransactionItem::TransactionItem(TransactionItem && mpkg) noexcept = default;

TransactionItem & TransactionItem::operator=(const TransactionItem & mpkg) {
    if (this != &mpkg) {
        if (p_impl) {
            *p_impl = *mpkg.p_impl;
        } else {
            p_impl = std::make_unique<Impl>(*mpkg.p_impl);
        }
    }

    return *this;
}
TransactionItem & TransactionItem::operator=(TransactionItem && mpkg) noexcept = default;

TransactionItem::Impl::Impl(const Transaction & trans) : trans(&trans) {}

std::string TransactionItem::get_action_name() {
    return transaction_item_action_to_string(p_impl->action);
}


std::string TransactionItem::get_action_short() {
    return transaction_item_action_to_letter(p_impl->action);
}


TransactionItem::TransactionItem(const Transaction & trans) : p_impl(std::make_unique<Impl>(trans)) {}


bool TransactionItem::is_inbound_action() const {
    return transaction_item_action_is_inbound(p_impl->action);
}


bool TransactionItem::is_outbound_action() const {
    return transaction_item_action_is_outbound(p_impl->action);
}


const Transaction & TransactionItem::get_transaction() const {
    libdnf_assert(p_impl->trans, "Transaction in TransactionItem was not set.");
    return *p_impl->trans;
}

TransactionItemAction TransactionItem::get_action() const noexcept {
    return p_impl->action;
}
TransactionItemReason TransactionItem::get_reason() const noexcept {
    return p_impl->reason;
}
const std::string & TransactionItem::get_repoid() const noexcept {
    return p_impl->repoid;
}
TransactionItemState TransactionItem::get_state() const noexcept {
    return p_impl->state;
}

int64_t TransactionItem::get_id() const noexcept {
    return p_impl->id;
}

void TransactionItem::set_id(int64_t value) {
    p_impl->id = value;
}

void TransactionItem::set_action(Action value) {
    p_impl->action = value;
}

void TransactionItem::set_reason(Reason value) {
    p_impl->reason = value;
}

void TransactionItem::set_state(State value) {
    p_impl->state = value;
}

void TransactionItem::set_repoid(const std::string & value) {
    p_impl->repoid = value;
}

int64_t TransactionItem::get_item_id() const noexcept {
    return p_impl->item_id;
}

void TransactionItem::set_item_id(int64_t value) {
    p_impl->item_id = value;
}

/*
void
TransactionItem::saveReplacedBy()
{
    if (replacedBy.empty()) {
        return;
    }
    const char *sql = "INSERT OR REPLACE INTO item_replaced_by VALUES (?, ?)";
    libdnf5::utils::SQLite3::Statement replacedByQuery(trans.get_connection(), sql);
    bool first = true;
    for (const auto &newItem : replacedBy) {
        if (!first) {
            // reset the prepared statement, so it can be executed again
            replacedByQuery.reset();
        }
        replacedByQuery.bindv(get_id(), newItem->get_id());
        replacedByQuery.step();
        first = false;
    }
}

void
TransactionItem::saveState()
{
    const char *sql = R"**(
        UPDATE
          trans_item
        SET
          state = ?
        WHERE
          id = ?
    )**";

    libdnf5::utils::SQLite3::Statement query(trans.get_connection(), sql);
    query.bindv(static_cast< int >(get_state()), get_id());
    query.step();
}

void
TransactionItem::dbUpdate()
{
    const char *sql = R"**(
        UPDATE
          trans_item
        SET
          trans_id=?,
          item_id=?,
          repo_id=?,
          action=?,
          reason=?,
          state=?
        WHERE
          id = ?
    )**";

    // try to find an existing repo
    auto query_repo_select_pkg = repo_select_pk_new_query(trans.get_connection());
    auto repo_id = repo_select_pk(*query_repo_select_pkg, get_repoid());

    if (!repo_id) {
        // if an existing repo was not found, insert a new record
        auto query_repo_insert = repo_insert_new_query(trans.get_connection());
        repo_id = repo_insert(*query_repo_insert, get_repoid());
    }


    libdnf5::utils::SQLite3::Statement query(trans.get_connection(), sql);
    query.bindv(trans.get_id(),
                getItem()->getId(),
                repo_id,
                static_cast< int >(get_action()),
                static_cast< int >(get_reason()),
                static_cast< int >(get_state()),
                get_id());
    query.step();
}

uint32_t
TransactionItem::getInstalledBy() const {
    return trans.get_user_id();
}
*/


}  // namespace libdnf5::transaction
