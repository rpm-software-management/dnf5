// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef LIBDNF5_TRANSACTION_TRANSACTION_ERRORS_HPP
#define LIBDNF5_TRANSACTION_TRANSACTION_ERRORS_HPP

#include "libdnf5/common/exception.hpp"
#include "libdnf5/defs.h"

#include <string>


namespace libdnf5::transaction {

class LIBDNF_API InvalidTransactionState : public libdnf5::Error {
public:
    InvalidTransactionState(const std::string & state);

    const char * get_domain_name() const noexcept override { return "libdnf5::transaction"; }
    const char * get_name() const noexcept override { return "InvalidTransactionState"; }
};

}  // namespace libdnf5::transaction

#endif  // LIBDNF5_TRANSACTION_TRANSACTION_ERRORS_HPP
