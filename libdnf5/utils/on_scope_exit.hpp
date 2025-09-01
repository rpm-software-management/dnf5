// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 2.1 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.

#ifndef _LIBDNF5_UTILS_ON_SCOPE_EXIT_HPP
#define _LIBDNF5_UTILS_ON_SCOPE_EXIT_HPP

#include <utility>

namespace libdnf5::utils {

/// The class template OnScopeExit is a general-purpose scope guard
/// intended to call its exit function when a scope is exited.
template <typename TExitFunction>
    requires requires(TExitFunction f) {
        { f() } noexcept;
    }
class OnScopeExit {
public:
    OnScopeExit(TExitFunction && function) noexcept : exit_function{std::move(function)} {}

    ~OnScopeExit() noexcept { exit_function(); }

    OnScopeExit(const OnScopeExit &) = delete;
    OnScopeExit(OnScopeExit &&) = delete;
    OnScopeExit & operator=(const OnScopeExit &) = delete;
    OnScopeExit & operator=(OnScopeExit &&) = delete;

private:
    TExitFunction exit_function;
};

}  // namespace libdnf5::utils

#endif
