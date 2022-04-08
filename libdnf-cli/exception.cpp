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


#include "libdnf-cli/exception.hpp"

#include "utils/bgettext/bgettext-lib.h"
#include "utils/string.hpp"

namespace libdnf::cli {

const char * GoalResolveError::what() const noexcept {
    try {
        if (resolve_logs.empty()) {
            return TM_(std::runtime_error::what(), 1);
        } else {
            message = fmt::format(
                "{}:\n{}", TM_(std::runtime_error::what(), 1), libdnf::utils::string::join(resolve_logs, "\n"));
        }
    } catch (...) {
        message = TM_(std::runtime_error::what(), 1);
    }
    return message.c_str();
}

}  // namespace libdnf::cli
