// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef LIBDNF5_COMPS_GROUP_PACKAGE_ERRORS_HPP
#define LIBDNF5_COMPS_GROUP_PACKAGE_ERRORS_HPP

#include "package_type.hpp"

#include "libdnf5/common/exception.hpp"
#include "libdnf5/defs.h"

#include <string>

namespace libdnf5::comps {

class LIBDNF_API InvalidPackageType : public libdnf5::Error {
public:
    InvalidPackageType(const std::string & type);
    InvalidPackageType(const PackageType type);

    const char * get_domain_name() const noexcept override { return "libdnf5::comps"; }
    const char * get_name() const noexcept override { return "InvalidPackageType"; }
};

}  // namespace libdnf5::comps

#endif
