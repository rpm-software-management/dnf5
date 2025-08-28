// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later


#include "dnf5daemon-client/exception.hpp"

#include <libdnf5/utils/bgettext/bgettext-mark-domain.h>

namespace dnfdaemon::client {

UnprivilegedUserError::UnprivilegedUserError()
    : Error(M_("This command has to be run with superuser privileges (under the root user on most systems).")) {}

}  // namespace dnfdaemon::client
