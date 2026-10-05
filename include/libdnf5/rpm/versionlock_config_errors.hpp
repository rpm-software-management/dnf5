// Copyright Contributors to the DNF5 project
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef LIBDNF5_RPM_VERSIONLOCK_CONFIG_ERRORS_HPP
#define LIBDNF5_RPM_VERSIONLOCK_CONFIG_ERRORS_HPP

#include "libdnf5/common/exception.hpp"
#include "libdnf5/defs.h"


namespace libdnf5::rpm {

/// An error thrown when the versionlock configuration file cannot be parsed.
class LIBDNF_API VersionlockParseError : public libdnf5::Error {
public:
    using libdnf5::Error::Error;
    const char * get_domain_name() const noexcept override { return "libdnf5::rpm"; }
    const char * get_name() const noexcept override { return "VersionlockParseError"; }
};

}  // namespace libdnf5::rpm

#endif  // LIBDNF5_RPM_VERSIONLOCK_CONFIG_ERRORS_HPP
