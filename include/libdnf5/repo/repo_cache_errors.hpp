// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef LIBDNF5_REPO_REPO_CACHE_ERRORS_HPP
#define LIBDNF5_REPO_REPO_CACHE_ERRORS_HPP

#include "libdnf5/common/exception.hpp"
#include "libdnf5/defs.h"


namespace libdnf5::repo {

/// RepoCache exception
class LIBDNF_API RepoCacheError : public Error {
public:
    using Error::Error;
    const char * get_domain_name() const noexcept override { return "libdnf5::repo"; }
    const char * get_name() const noexcept override { return "RepoCacheError"; }
};

}  // namespace libdnf5::repo

#endif
