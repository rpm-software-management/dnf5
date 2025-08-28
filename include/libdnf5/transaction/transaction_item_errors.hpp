// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

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
