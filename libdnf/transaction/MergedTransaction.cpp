/*
 * Copyright (C) 2017-2018 Red Hat, Inc.
 *
 * Licensed under the GNU Lesser General Public License Version 2.1
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "MergedTransaction.hpp"

namespace libdnf::transaction {

/**
 * Create a new MergedTransaction object with a single transaction
 * \param trans initial transaction
 */
MergedTransaction::MergedTransaction(TransactionPtr trans)
  : transactions{trans}
{
}

/**
 * Merge \trans into this transaction
 * Internally, transactions are kept in a sorted vector, what allows to
 *  easily access merged transaction properties on demand.
 * \param trans transaction to be merged with
 */
void
MergedTransaction::merge(TransactionPtr trans)
{
    bool inserted = false;
    for (auto it = transactions.begin(); it < transactions.end(); ++it) {
        if ((*it)->getId() > trans->getId()) {
            transactions.insert(it, trans);
            inserted = true;
            break;
        }
    }
    if (!inserted) {
        transactions.push_back(trans);
    }
}

/**
 * Get IDs of the transactions involved in the merged transaction
 * \return list of transaction IDs sorted in ascending order
 */
std::vector< int64_t >
MergedTransaction::listIds() const
{
    std::vector< int64_t > ids;
    for (auto t : transactions) {
        ids.push_back(t->getId());
    }
    return ids;
}

/**
 * Get UNIX IDs of users who performed the transaction.
 * \return list of user IDs sorted by transaction ID in ascending order
 */
std::vector< uint32_t >
MergedTransaction::listUserIds() const
{
    std::vector< uint32_t > users;
    for (auto t : transactions) {
        users.push_back(t->getUserId());
    }
    return users;
}

/**
 * Get list of commands that started the transaction
 * \return list of commands sorted by transaction ID in ascending order
 */
std::vector< std::string >
MergedTransaction::listCmdlines() const
{
    std::vector< std::string > cmdLines;
    for (auto t : transactions) {
        cmdLines.push_back(t->getCmdline());
    }
    return cmdLines;
}

std::vector< TransactionState >
MergedTransaction::listStates() const
{
    std::vector< TransactionState > result;
    for (auto t : transactions) {
        result.push_back(t->getState());
    }
    return result;
}

std::vector< std::string >
MergedTransaction::listReleasevers() const
{
    std::vector< std::string > result;
    for (auto t : transactions) {
        result.push_back(t->getReleasever());
    }
    return result;
}

int64_t
MergedTransaction::getDtBegin() const noexcept
{
    return transactions.front()->getDtBegin();
}
int64_t
MergedTransaction::getDtEnd() const noexcept
{
    return transactions.back()->getDtEnd();
}
const std::string &
MergedTransaction::getRpmdbVersionBegin() const noexcept
{
    return transactions.front()->getRpmdbVersionBegin();
}

const std::string &
MergedTransaction::getRpmdbVersionEnd() const noexcept
{
    return transactions.back()->getRpmdbVersionEnd();
}

std::set< RPMItemPtr >
MergedTransaction::getSoftwarePerformedWith() const
{
    std::set< RPMItemPtr > software;
    for (auto t : transactions) {
        auto tranSoft = t->getSoftwarePerformedWith();
        software.insert(tranSoft.begin(), tranSoft.end());
    }
    return software;
}

std::vector< std::pair< int, std::string > >
MergedTransaction::getConsoleOutput()
{
    std::vector< std::pair< int, std::string > > output;
    for (auto t : transactions) {
        auto tranOutput = t->getConsoleOutput();
        output.insert(output.end(), tranOutput.begin(), tranOutput.end());
    }
    return output;
}

/**
 * Get list of transaction items involved in the merged transaction
 * Actions are merged using following rules:
 * (old action) -> (new action) = (merged action)
 *
 * Erase/Obsolete -> Install/Obsoleting = Reinstall/Downgrade/Upgrade
 *
 * Reinstall/Reason change -> (new action) = (new action)
 *
 * Install -> Erase = (nothing)
 *
 * Install -> Upgrade/Downgrade = Install (with Upgrade version)
 *
 * Downgrade/Upgrade/Obsoleting -> Reinstall = (old action)
 *
 * Downgrade/Upgrade/Obsoleting -> Erase/Obsoleted = Erase/Obsolete (with old package)
 *
 * Downgrade/Upgrade/Obsoleting -> Downgraded/Upgrade =
 *      We have differentiate between original transaction, and new one.
 *      When a transaction package pair is not complete, then we are still in original one.
 *
 *      With complete transaction pair we need to get a new Upgrade/Downgrade package and
 *      compare versions with original package from pair.
 */
std::vector< TransactionItemBasePtr >
MergedTransaction::getItems()
{
    ItemPairMap itemPairMap;

    // iterate over transaction
    for (auto t : transactions) {
        auto transItems = t->getItems();
        // iterate over transaction items
        for (auto transItem : transItems) {
            // get item and its type
            auto mTransItem = std::dynamic_pointer_cast< TransactionItemBase >(transItem);
            mergeItem(itemPairMap, mTransItem);
        }
    }

    std::vector< TransactionItemBasePtr > items;
    for (const auto &row : itemPairMap) {
        ItemPair itemPair = row.second;
        items.push_back(itemPair.first);
        if (itemPair.second != nullptr) {
            items.push_back(itemPair.second);
        }
    }
    return items;
}

static std::string
getItemIdentifier(ItemPtr item)
{
    auto itemType = item->getItemType();
    std::string name;
    if (itemType == ItemType::RPM) {
        auto rpm = std::dynamic_pointer_cast< RPMItem >(item);
        name = rpm->getName() + "." + rpm->getArch();
    } else if (itemType == ItemType::GROUP) {
        auto group = std::dynamic_pointer_cast< CompsGroupItem >(item);
        name = group->getGroupId();
    } else if (itemType == ItemType::ENVIRONMENT) {
        auto env = std::dynamic_pointer_cast< CompsEnvironmentItem >(item);
        name = env->getEnvironmentId();
    }
    return name;
}

/**
 * Resolve the difference between RPMs in the first and second transaction item
 *  and create a ItemPair of Upgrade, Downgrade or reinstall.
 * Method is called when original package is being removed and than installed again.
 * \param previousItemPair original item pair
 * \param mTransItem new transaction item
 */
void
MergedTransaction::resolveRPMDifference(ItemPair &previousItemPair,
                                        TransactionItemBasePtr mTransItem)
{
    auto firstItem = previousItemPair.first->getItem();
    auto secondItem = mTransItem->getItem();

    auto firstRPM = std::dynamic_pointer_cast< RPMItem >(firstItem);
    auto secondRPM = std::dynamic_pointer_cast< RPMItem >(secondItem);

    if (firstRPM->getVersion() == secondRPM->getVersion() &&
        firstRPM->getEpoch() == secondRPM->getEpoch()) {
        // reinstall
        mTransItem->setAction(TransactionItemAction::REINSTALL);
        previousItemPair.first = mTransItem;
        previousItemPair.second = nullptr;
        return;
    } else if ((*firstRPM) < (*secondRPM)) {
        // Upgrade to secondRPM
        previousItemPair.first->setAction(TransactionItemAction::UPGRADED);
        mTransItem->setAction(TransactionItemAction::UPGRADE);
    } else {
        // Downgrade to secondRPM
        previousItemPair.first->setAction(TransactionItemAction::DOWNGRADED);
        mTransItem->setAction(TransactionItemAction::DOWNGRADE);
    }
    previousItemPair.second = mTransItem;
}

void
MergedTransaction::resolveErase(ItemPair &previousItemPair, TransactionItemBasePtr mTransItem)
{
    /*
     * The original item has been removed - it has to be installed now unless the rpmdb
     *  has changed. Resolve the difference between packages and mark it as Upgrade,
     *  Reinstall or Downgrade
     */
    if (mTransItem->getAction() == TransactionItemAction::INSTALL) {
        if (mTransItem->getItem()->getItemType() == ItemType::RPM) {
            // resolve the difference between RPM packages
            resolveRPMDifference(previousItemPair, mTransItem);
        } else {
            // difference between comps can't be resolved
            mTransItem->setAction(TransactionItemAction::REINSTALL);
        }
    }
    previousItemPair.first = mTransItem;
    previousItemPair.second = nullptr;
}

/**
 * Resolve altered - Upgrade(d)/Downgrade(d) transaction items.
 * If the new item is Erased or Obsoleted, than its action is transferred to the original pair.
 * When its being Downgraded/Upgraded and the pair is incomplete then we are in the same
 * transaction - new package is used to complete the pair. Items are stored in pairs (Upgrade,
 * Upgrade) or (Downgraded, Downgrade). With complete transaction pair we need to get the new
 * Upgrade/Downgrade item and compare its version with the original item from the pair.
 * \param previousItemPair original item pair
 * \param mTransItem new transaction item
 */
void
MergedTransaction::resolveAltered(ItemPair &previousItemPair, TransactionItemBasePtr mTransItem)
{
    auto newState = mTransItem->getAction();
    auto firstState = previousItemPair.first->getAction();

    if (newState == TransactionItemAction::REMOVE || newState == TransactionItemAction::OBSOLETED) {
        // package is being Erased
        // move Erased action to the previous state
        previousItemPair.first->setAction(newState);
        previousItemPair.second = nullptr;
    } else if (newState == TransactionItemAction::DOWNGRADED ||
               newState == TransactionItemAction::UPGRADED) {
        // check if the transaction pair is complete
        if (previousItemPair.second == nullptr) {
            // pair might be in a wrong order
            if (firstState == TransactionItemAction::DOWNGRADE ||
                firstState == TransactionItemAction::UPGRADE) {
                // fix the order
                previousItemPair.second = previousItemPair.first;
                previousItemPair.first = mTransItem;
            }
        }
        // XXX handle obsoleting state -> state is not supported anymore, so it can't
        // occur anymore - maybe we should set some "Obsoleting" flag or what
        // state of obsoleting package should be transferred to a new package
        /*
         * Otherwise we can just drop the package
         * Original package from new transaction should be the same as a new package
         * from previous transaction - unless the RPMDB has altered.
         */

    } else if (newState == TransactionItemAction::DOWNGRADE ||
               newState == TransactionItemAction::UPGRADE) {
        /*
         * Check whether second item is missing in transaction pair
         * When it does, complete the transaction pair.
         */
        if (previousItemPair.second == nullptr) {
            previousItemPair.second = mTransItem;
        } else {
            if (mTransItem->getItem()->getItemType() == ItemType::RPM) {
                // resolve the difference between RPM packages
                resolveRPMDifference(previousItemPair, mTransItem);
            } else {
                // difference between comps can't be resolved
                previousItemPair.second->setAction(TransactionItemAction::REINSTALL);
                previousItemPair.first = previousItemPair.second;
                previousItemPair.second = nullptr;
            }
        }
    }
}

/**
 * Merge transaction item into merged transaction set
 * \param itemPairMap merged transaction set
 * \param mTransItem transaction item
 */
void
MergedTransaction::mergeItem(ItemPairMap &itemPairMap, TransactionItemBasePtr mTransItem)
{
    // get item identifier
    std::string name = getItemIdentifier(mTransItem->getItem());

    auto previous = itemPairMap.find(name);
    if (previous == itemPairMap.end()) {
        itemPairMap[name] = ItemPair(mTransItem, nullptr);
        return;
    }

    ItemPair &previousItemPair = previous->second;

    auto firstState = previousItemPair.first->getAction();
    auto newState = mTransItem->getAction();

    if (firstState == TransactionItemAction::INSTALL && mTransItem->isBackwardAction()) {
        return;
    }

    switch (firstState) {
        case TransactionItemAction::REMOVE:
        case TransactionItemAction::OBSOLETED:
            resolveErase(previousItemPair, mTransItem);
            break;
        case TransactionItemAction::INSTALL:
            // the original package has been installed -> it may be either Removed, or altered
            if (newState == TransactionItemAction::REMOVE ||
                newState == TransactionItemAction::OBSOLETED) {
                // Install -> Remove = (nothing)
                itemPairMap.erase(name);
                break;
            }
            // altered -> transfer install to the altered package
            mTransItem->setAction(TransactionItemAction::INSTALL);

            // don't break
            // gcc doesn't support [[fallthrough]]
            __attribute__((fallthrough));
        case TransactionItemAction::REINSTALL:
        case TransactionItemAction::REASON_CHANGE:
            // The original item has been reinstalled or the reason has been changed
            // keep the new action
            previousItemPair.first = mTransItem;
            previousItemPair.second = nullptr;
            break;
        case TransactionItemAction::DOWNGRADE:
        case TransactionItemAction::DOWNGRADED:
        case TransactionItemAction::UPGRADE:
        case TransactionItemAction::UPGRADED:
        case TransactionItemAction::OBSOLETE:
            resolveAltered(previousItemPair, mTransItem);
            break;
        case TransactionItemAction::REINSTALLED:
            break;
    }
}

}  // namespace libdnf::transaction
