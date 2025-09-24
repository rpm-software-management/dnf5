// Copyright Contributors to the DNF5 project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef LIBDNF5_BASE_ACTIVE_TRANSACTION_INFO_ERRORS_HPP
#define LIBDNF5_BASE_ACTIVE_TRANSACTION_INFO_ERRORS_HPP

#include "libdnf5/common/exception.hpp"
#include "libdnf5/defs.h"


namespace libdnf5::base {

class LIBDNF_API ActiveTransactionInfoParseError : public libdnf5::Error {
public:
    using libdnf5::Error::Error;
    const char * get_domain_name() const noexcept override { return "libdnf5::base"; }
    const char * get_name() const noexcept override { return "ActiveTransactionInfoParseError"; }
};

}  // namespace libdnf5::base

#endif  // LIBDNF5_BASE_ACTIVE_TRANSACTION_INFO_ERRORS_HPP
