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

#include <dnf5/context.hpp>
#include <libdnf5-cli/session.hpp>
#include <libdnf5/utils/bgettext/bgettext-lib.h>

namespace dnf5 {

class AllowErasingOption : public libdnf5::cli::session::BoolOption {
public:
    explicit AllowErasingOption(libdnf5::cli::session::Command & command)
        : BoolOption(
              command, "allowerasing", '\0', _("Allow erasing of installed packages to resolve problems"), false) {}
};

class SkipBrokenOption : public libdnf5::cli::session::BoolOption {
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

class SkipUnavailableOption : public libdnf5::cli::session::BoolOption {
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
/// The values are stored in the `allow_downgrade` configuration option
void create_allow_downgrade_options(dnf5::Command & command);

/// Create the `--destdir` option for a command provided as an argument.
/// The values are stored in the `destdir` configuration option
void create_destdir_option(dnf5::Command & command);

/// Create the `--downloadonly` option for a command provided as an argument.
/// The values are stored in the `downloadonly` configuration option
void create_downloadonly_option(dnf5::Command & command);

/// Create the `--forcearch` option for a command provided as an argument.
/// The values are stored in the `forcearch` configuration option
[[deprecated("--forcearch is now a global argument")]] void create_forcearch_option(dnf5::Command & command);

/// Create the `--offline` option for a command provided as an argument.
void create_offline_option(dnf5::Command & command);

}  // namespace dnf5

#endif  // DNF5_COMMANDS_SHARED_OPTIONS_HPP
