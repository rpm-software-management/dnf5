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

#ifndef LIBDNF5_RPM_REPO_ERRORS_HPP
#define LIBDNF5_RPM_REPO_ERRORS_HPP

#include "libdnf5/common/exception.hpp"


namespace libdnf5::repo {

class RepoError : public Error {
    using Error::Error;
    const char * get_domain_name() const noexcept override { return "libdnf5::repo"; }
    const char * get_name() const noexcept override { return "RepoError"; }
};


class RepoCacheonlyError : public RepoError {
public:
    using RepoError::RepoError;
    const char * get_name() const noexcept override { return "RepoCacheonlyError"; }
};


class RepoDownloadError : public RepoError {
public:
    using RepoError::RepoError;
    const char * get_name() const noexcept override { return "RepoDownloadError"; }
};


class RepoPgpError : public RepoError {
public:
    using RepoError::RepoError;
    const char * get_name() const noexcept override { return "RepoPgpError"; }
};


class RepoRpmError : public RepoError {
    using RepoError::RepoError;
    const char * get_name() const noexcept override { return "RepoRpmError"; }
};

class RepoIdAlreadyExistsError : public RepoError {
    using RepoError::RepoError;
    const char * get_name() const noexcept override { return "RepoIdAlreadyExistsError"; }
};

}  // namespace libdnf5::repo

#endif
