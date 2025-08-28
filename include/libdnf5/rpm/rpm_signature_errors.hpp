// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef LIBDNF5_RPM_RPM_SIGNATURE_ERRORS_HPP
#define LIBDNF5_RPM_RPM_SIGNATURE_ERRORS_HPP

#include "libdnf5/common/exception.hpp"
#include "libdnf5/defs.h"

namespace libdnf5::rpm {

class LIBDNF_API SignatureCheckError : public Error {
public:
    using Error::Error;
    const char * get_domain_name() const noexcept override { return "libdnf5::rpm"; }
    const char * get_name() const noexcept override { return "SignatureCheckError"; }
};

class LIBDNF_API KeyImportError : public Error {
public:
    using Error::Error;
    const char * get_domain_name() const noexcept override { return "libdnf5::rpm"; }
    const char * get_name() const noexcept override { return "KeyImportError"; }
};

}  // namespace libdnf5::rpm

#endif  // LIBDNF5_RPM_RPM_SIGNATURE_ERRORS_HPP
