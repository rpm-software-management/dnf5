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
    explicit AllowErasingOption(libdnf5::cli::session::Command & command);
    ~AllowErasingOption();
};


class SkipBrokenOption : public libdnf5::cli::session::BoolOption {
public:
    explicit SkipBrokenOption(dnf5::Command & command);
    ~SkipBrokenOption();
};


class SkipUnavailableOption : public libdnf5::cli::session::BoolOption {
public:
    explicit SkipUnavailableOption(dnf5::Command & command);
    ~SkipUnavailableOption();
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

/// Create the `--store` option for a command provided as an argument.
/// The value is stored in Context::transaction_store_path.
void create_store_option(dnf5::Command & command);


/// Create the `--offline` option for a command provided as an argument.
void create_offline_option(dnf5::Command & command);

}  // namespace dnf5

#endif  // DNF5_COMMANDS_SHARED_OPTIONS_HPP
