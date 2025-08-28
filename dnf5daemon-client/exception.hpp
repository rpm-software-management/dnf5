// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

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
