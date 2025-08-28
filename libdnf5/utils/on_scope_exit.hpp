// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

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
