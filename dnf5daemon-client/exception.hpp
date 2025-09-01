// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 2.1 of the License, or
// (at your option) any later version.
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// You should have received a copy of the GNU Lesser General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.

#ifndef DNF5DAEMON_CLIENT_EXCEPTION_HPP
#define DNF5DAEMON_CLIENT_EXCEPTION_HPP

#include <libdnf5-cli/exception.hpp>
#include <libdnf5-cli/exit-codes.hpp>
#include <libdnf5/common/exception.hpp>

namespace dnfdaemon::client {

/// Base class for dnf5daemon-client exceptions.
class Error : public libdnf5::Error {
public:
    using libdnf5::Error::Error;
    const char * get_domain_name() const noexcept override { return "dnfdaemon::client"; }
    const char * get_name() const noexcept override { return "Error"; }
};

/// Exception is thrown when the user is not root.
class UnprivilegedUserError : public Error {
public:
    UnprivilegedUserError();
    const char * get_name() const noexcept override { return "UnprivilegedUserError"; }
};

}  // namespace dnfdaemon::client

#endif  // DNF5DAEMON_CLIENT_EXCEPTION_HPP
