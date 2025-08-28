// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef LIBDNF5_RPM_REPO_ERRORS_HPP
#define LIBDNF5_RPM_REPO_ERRORS_HPP

#include "libdnf5/common/exception.hpp"
#include "libdnf5/defs.h"


namespace libdnf5::repo {

class LIBDNF_API RepoError : public Error {
    using Error::Error;
    const char * get_domain_name() const noexcept override { return "libdnf5::repo"; }
    const char * get_name() const noexcept override { return "RepoError"; }
};


class LIBDNF_API RepoCacheonlyError : public RepoError {
public:
    using RepoError::RepoError;
    const char * get_name() const noexcept override { return "RepoCacheonlyError"; }
};


class LIBDNF_API RepoDownloadError : public RepoError {
public:
    using RepoError::RepoError;
    const char * get_name() const noexcept override { return "RepoDownloadError"; }
};


class LIBDNF_API RepoPgpError : public RepoError {
public:
    using RepoError::RepoError;
    const char * get_name() const noexcept override { return "RepoPgpError"; }
};


class LIBDNF_API RepoRpmError : public RepoError {
    using RepoError::RepoError;
    const char * get_name() const noexcept override { return "RepoRpmError"; }
};


class LIBDNF_API RepoCompsError : public RepoError {
    using RepoError::RepoError;
    const char * get_name() const noexcept override { return "RepoCompsError"; }
};


class LIBDNF_API RepoIdAlreadyExistsError : public RepoError {
    using RepoError::RepoError;
    const char * get_name() const noexcept override { return "RepoIdAlreadyExistsError"; }
};

}  // namespace libdnf5::repo

#endif
