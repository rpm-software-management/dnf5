// Copyright Contributors to the DNF5 project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef LIBDNF5_BASE_VENDOR_CHANGE_MANAGER_ERRORS_HPP
#define LIBDNF5_BASE_VENDOR_CHANGE_MANAGER_ERRORS_HPP

#include "libdnf5/common/exception.hpp"
#include "libdnf5/defs.h"

namespace libdnf5::base {

/// Error related to VendorChangeManager
class LIBDNF_API VendorChangeManagerError : public Error {
public:
    using Error::Error;
    /// @return Error class' domain name"
    const char * get_domain_name() const noexcept override { return "libdnf5::base"; }
    /// @return Error class' name"
    const char * get_name() const noexcept override { return "VendorChangeManagerError"; }
};

}  // namespace libdnf5::base

#endif  // LIBDNF5_BASE_VENDOR_CHANGE_MANAGER_ERRORS_HPP
