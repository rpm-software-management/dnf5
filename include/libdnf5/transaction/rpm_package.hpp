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

#ifndef LIBDNF5_TRANSACTION_RPM_PACKAGE_HPP
#define LIBDNF5_TRANSACTION_RPM_PACKAGE_HPP

#include "transaction_item.hpp"

#include "libdnf5/defs.h"

#include <memory>


namespace libdnf5::transaction {

class Transaction;
class RpmDbUtils;


/// Package contains a copy of important data from rpm::Package that is used
/// to perform rpm transaction and then stored in the transaction (history) database.
///
// @replaces libdnf:transaction/RPMItem.hpp:class:RPMItem
class LIBDNF_API Package : public TransactionItem {
public:
    /// Get package name
    ///
    // @replaces libdnf:transaction/RPMItem.hpp:method:RPMItem.getName()
    const std::string & get_name() const noexcept;

    /// Get package epoch
    ///
    // @replaces libdnf:transaction/RPMItem.hpp:method:RPMItem.getEpoch()
    const std::string & get_epoch() const noexcept;

    /// Get package release
    ///
    // @replaces libdnf:transaction/RPMItem.hpp:method:RPMItem.getRelease()
    const std::string & get_release() const noexcept;

    /// Get package arch
    ///
    // @replaces libdnf:transaction/RPMItem.hpp:method:RPMItem.getArch()
    const std::string & get_arch() const noexcept;

    /// Get package version
    ///
    // @replaces libdnf:transaction/RPMItem.hpp:method:RPMItem.getVersion()
    const std::string & get_version() const noexcept;

    /// Get string representation of the object, which equals to package NEVRA
    ///
    // @replaces libdnf:transaction/RPMItem.hpp:method:RPMItem.toStr()
    std::string to_string() const;

    ~Package();
    Package(const Package & src);
    Package & operator=(const Package & src);
    Package(Package && src) noexcept;
    Package & operator=(Package && src) noexcept;

private:
    friend RpmDbUtils;
    friend Transaction;

    LIBDNF_LOCAL explicit Package(const Transaction & trans);

    /// Set package name
    ///
    // @replaces libdnf:transaction/RPMItem.hpp:method:RPMItem.setName(const std::string & value)
    LIBDNF_LOCAL void set_name(const std::string & value);

    /// Get package epoch as an integer
    LIBDNF_LOCAL uint32_t get_epoch_int() const;

    /// Set package epoch
    ///
    // @replaces libdnf:transaction/RPMItem.hpp:method:RPMItem.setEpoch(int32_t value)
    LIBDNF_LOCAL void set_epoch(const std::string & value);

    /// Set package version
    ///
    // @replaces libdnf:transaction/RPMItem.hpp:method:RPMItem.setVersion(const std::string & value)
    LIBDNF_LOCAL void set_version(const std::string & value);

    /// Set package release
    ///
    // @replaces libdnf:transaction/RPMItem.hpp:method:RPMItem.setRelease(const std::string & value)
    LIBDNF_LOCAL void set_release(const std::string & value);

    /// Set package arch
    ///
    // @replaces libdnf:transaction/RPMItem.hpp:method:RPMItem.setArch(const std::string & value)
    LIBDNF_LOCAL void set_arch(const std::string & value);

    /*
    // TODO(dmach): Implement TransactionSack.new_filter().filter_package_pattern()
    static std::vector<int64_t> searchTransactions(
        libdnf5::utils::SQLite3 & conn, const std::vector<std::string> & patterns);

    // TODO(dmach): Implement as a precomputed map of reasons associated to TransactionSack.
    static TransactionItemReason resolveTransactionItemReason(
        libdnf5::utils::SQLite3 & conn, const std::string & name, const std::string & arch, int64_t maxTransactionId);
    */

    LIBDNF_LOCAL bool operator<(const Package & other) const;

    class LIBDNF_LOCAL Impl;
    std::unique_ptr<Impl> p_impl;
};

}  // namespace libdnf5::transaction

#endif  // LIBDNF5_TRANSACTION_RPM_PACKAGE_HPP
