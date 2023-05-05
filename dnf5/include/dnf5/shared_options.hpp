/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef DNF5_COMMANDS_SHARED_OPTIONS_HPP
#define DNF5_COMMANDS_SHARED_OPTIONS_HPP

#include "utils/bgettext/bgettext-lib.h"

#include <dnf5/context.hpp>
#include <libdnf-cli/session.hpp>

namespace dnf5 {

class AllowErasingOption : public libdnf::cli::session::BoolOption {
public:
    explicit AllowErasingOption(libdnf::cli::session::Command & command)
        : BoolOption(
              command, "allowerasing", '\0', _("Allow erasing of installed packages to resolve problems"), false) {}
};

class SkipBrokenOption : public libdnf::cli::session::BoolOption {
public:
    explicit SkipBrokenOption(dnf5::Command & command)
        : BoolOption(
              command,
              "skip-broken",
              '\0',
              _("Allow resolving of depsolve problems by skipping packages"),
              false,
              &command.get_context().base.get_config().get_skip_broken_option()) {}
};

class SkipUnavailableOption : public libdnf::cli::session::BoolOption {
public:
    explicit SkipUnavailableOption(dnf5::Command & command)
        : BoolOption(
              command,
              "skip-unavailable",
              '\0',
              _("Allow skipping unavailable packages"),
              false,
              &command.get_context().base.get_config().get_skip_unavailable_option()) {}
};

/// Create two options (`--allow-downgrade` and `--no-allow-downgrade`) for a command provided as an argument command.
/// The values are atored in the `allow_downgrade` configuration option
void create_allow_downgrade_options(dnf5::Command & command);


}  // namespace dnf5

#endif  // DNF5_COMMANDS_SHARED_OPTIONS_HPP
