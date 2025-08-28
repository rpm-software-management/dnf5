// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later


#include "libdnf5-cli/exception.hpp"

#include "utils/string.hpp"

#include <libdnf5/utils/bgettext/bgettext-lib.h>
#include <libdnf5/utils/bgettext/bgettext-mark-domain.h>

namespace libdnf5::cli {

AbortedByUserError::AbortedByUserError() : Error(M_("Operation aborted by the user.")) {}

GoalResolveError::GoalResolveError(const std::vector<std::string> resolve_logs)
    : Error(M_("Failed to resolve the transaction")),
      resolve_logs(resolve_logs) {}

const char * GoalResolveError::what() const noexcept {
    try {
        if (resolve_logs.empty()) {
            message = TM_(format, 1);
        } else {
            message = fmt::format("{}:\n{}", TM_(format, 1), libdnf5::utils::string::join(resolve_logs, "\n"));
        }
    } catch (...) {
        message = TM_(format, 1);
    }
    return message.c_str();
}

SilentCommandExitError::SilentCommandExitError(int exit_code) : Error(EMPTY_MESSAGE), exit_code(exit_code) {}

}  // namespace libdnf5::cli
