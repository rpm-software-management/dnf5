// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef LIBDNF5_REPO_PACKAGE_DOWNLOADER_ERRORS_HPP
#define LIBDNF5_REPO_PACKAGE_DOWNLOADER_ERRORS_HPP

#include "libdnf5/common/exception.hpp"
#include "libdnf5/defs.h"


namespace libdnf5::repo {

class LIBDNF_API PackageDownloadError : public Error {
    using Error::Error;
    const char * get_domain_name() const noexcept override { return "libdnf5::repo"; }
    const char * get_name() const noexcept override { return "PackageDownloadError"; }
};

}  // namespace libdnf5::repo

#endif  // LIBDNF5_REPO_PACKAGE_DOWNLOADER_ERRORS_HPP
