// Copyright Contributors to the DNF5 project.
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

#ifndef LIBDNF5_TRANSACTION_ITEM_ERRORS_HPP
#define LIBDNF5_TRANSACTION_ITEM_ERRORS_HPP

#include "libdnf5/common/exception.hpp"
#include "libdnf5/defs.h"

#include <string>


namespace libdnf5::transaction {

class LIBDNF_API InvalidTransactionItemAction : public libdnf5::Error {
public:
    InvalidTransactionItemAction(const std::string & action);

    const char * get_domain_name() const noexcept override { return "libdnf5::transaction"; }
    const char * get_name() const noexcept override { return "InvalidTransactionItemAction"; }
};


class LIBDNF_API InvalidTransactionItemReason : public libdnf5::Error {
public:
    InvalidTransactionItemReason(const std::string & reason);

    const char * get_domain_name() const noexcept override { return "libdnf5::transaction"; }
    const char * get_name() const noexcept override { return "InvalidTransactionItemReason"; }
};


class LIBDNF_API InvalidTransactionItemState : public libdnf5::Error {
public:
    InvalidTransactionItemState(const std::string & state);

    const char * get_domain_name() const noexcept override { return "libdnf5::transaction"; }
    const char * get_name() const noexcept override { return "InvalidTransactionItemState"; }
};


class LIBDNF_API InvalidTransactionItemType : public libdnf5::Error {
public:
    InvalidTransactionItemType(const std::string & type);

    const char * get_domain_name() const noexcept override { return "libdnf5::transaction"; }
    const char * get_name() const noexcept override { return "InvalidTransactionItemType"; }
};

}  // namespace libdnf5::transaction

#endif
