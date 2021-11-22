/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2.1 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef LIBDNF_RPM_REPO_ERRORS_HPP
#define LIBDNF_RPM_REPO_ERRORS_HPP

#include "libdnf/common/exception.hpp"


namespace libdnf::repo {

class RepoError : public Error {
    using Error::Error;
    const char * get_domain_name() const noexcept override { return "libdnf::repo"; }
    const char * get_name() const noexcept override { return "RepoError"; }
};


class RepoDownloadError : public RepoError {
public:
    using RepoError::RepoError;
    const char * get_name() const noexcept override { return "RepoDownloadError"; }
};


class RepoGpgError : public RepoError {
public:
    using RepoError::RepoError;
    const char * get_name() const noexcept override { return "RepoGpgError"; }
};


class RepoRpmError : public RepoError {
    using RepoError::RepoError;
    const char * get_name() const noexcept override { return "RepoRpmError"; }
};

}  // namespace libdnf::repo

#endif
