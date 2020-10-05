/*
Copyright (C) 2017-2020 Red Hat, Inc.

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


#ifndef LIBDNF_TRANSACTION_RPM_PACKAGE_HPP
#define LIBDNF_TRANSACTION_RPM_PACKAGE_HPP


#include "transaction_item.hpp"
#include "transaction_item_reason.hpp"

#include <memory>
#include <vector>


namespace libdnf::transaction {


class Transaction;


class Package : public TransactionItem {
public:
    explicit Package(Transaction & trans);

    const std::string & get_name() const noexcept { return name; }
    void set_name(const std::string & value) { name = value; }

    const std::string & get_epoch() const noexcept { return epoch; }
    uint32_t get_epoch_int() const;
    void set_epoch(const std::string & value) { epoch = value; }

    const std::string & get_version() const noexcept { return version; }
    void set_version(const std::string & value) { version = value; }

    const std::string & get_release() const noexcept { return release; }
    void set_release(const std::string & value) { release = value; }

    const std::string & get_arch() const noexcept { return arch; }
    void set_arch(const std::string & value) { arch = value; }

    static std::vector<int64_t> searchTransactions(
        libdnf::utils::SQLite3 & conn, const std::vector<std::string> & patterns);
    static TransactionItemReason resolveTransactionItemReason(
        libdnf::utils::SQLite3 & conn, const std::string & name, const std::string & arch, int64_t maxTransactionId);

    std::string to_string() const;

    bool operator<(const Package & other) const;

private:
    std::string name;
    std::string epoch;
    std::string version;
    std::string release;
    std::string arch;
};


}  // namespace libdnf::transaction


#endif  // LIBDNF_TRANSACTION_RPM_PACKAGE_HPP
