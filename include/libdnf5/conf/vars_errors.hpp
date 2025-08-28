// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef LIBDNF5_CONF_VARS_ERRORS_HPP
#define LIBDNF5_CONF_VARS_ERRORS_HPP

#include "libdnf5/common/exception.hpp"
#include "libdnf5/defs.h"


namespace libdnf5 {

// Thrown when attempting to set a read-only variable
class LIBDNF_API ReadOnlyVariableError : public Error {
    using Error::Error;

    const char * get_domain_name() const noexcept override { return "libdnf5"; }
    const char * get_name() const noexcept override { return "ReadOnlyVariableError"; }
};

}  // namespace libdnf5

#endif
