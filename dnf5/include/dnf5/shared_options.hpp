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

#include "defs.h"

#include <dnf5/context.hpp>
#include <libdnf5-cli/session.hpp>
#include <libdnf5/utils/bgettext/bgettext-lib.h>

namespace dnf5 {

class DNF_API AllowErasingOption : public libdnf5::cli::session::BoolOption {
public:
    explicit AllowErasingOption(libdnf5::cli::session::Command & command);
    ~AllowErasingOption();
};


class DNF_API SkipBrokenOption : public libdnf5::cli::session::BoolOption {
public:
    explicit SkipBrokenOption(dnf5::Command & command);
    ~SkipBrokenOption();
};


class DNF_API SkipUnavailableOption : public libdnf5::cli::session::BoolOption {
public:
    explicit SkipUnavailableOption(dnf5::Command & command);
    ~SkipUnavailableOption();
};


/// Create two options (`--allow-downgrade` and `--no-allow-downgrade`) for a command provided as an argument command.
/// The values are stored in the `allow_downgrade` configuration option
DNF_API void create_allow_downgrade_options(dnf5::Command & command);

/// Create the `--destdir` option for a command provided as an argument.
/// The values are stored in the `destdir` configuration option
DNF_API void create_destdir_option(dnf5::Command & command);

/// Create the `--downloadonly` option for a command provided as an argument.
/// The values are stored in the `downloadonly` configuration option
DNF_API void create_downloadonly_option(dnf5::Command & command);

/// Create the `--store` option for a command provided as an argument.
/// The value is stored in Context::transaction_store_path.
DNF_API void create_store_option(dnf5::Command & command);

/// Create the `--offline` option for a command provided as an argument.
DNF_API void create_offline_option(dnf5::Command & command);

/// Create the `--json` option for a command provided as an argument.
DNF_API void create_json_option(dnf5::Command & command);


/// Creates a new named argument "--from-repo=REPO_ID,...".
/// When the argument is used, the list of REPO_IDs is split and the IDs are stored in the `repo_ids` vector.
/// If `detect_conflict` is true, a `libdnf5::cli::ArgumentParserConflictingArgumentsError` exception is thrown
/// if "--from-repo" was already used with a different value.
/// Newly created argument is registered to the `command` and returned.
DNF_API libdnf5::cli::ArgumentParser::NamedArg * create_from_repo_option(
    Command & command, std::vector<std::string> & repo_ids, bool detect_conflict);

/// Creates a new named argument "--installed-from-repo=REPO_ID,...".
/// When the argument is used, the list of REPO_IDs is split and the IDs are stored in the `repo_ids` vector.
/// If `detect_conflict` is true, a `libdnf5::cli::ArgumentParserConflictingArgumentsError` exception is thrown
/// if "--installed-from-repo" was already used with a different value.
/// Newly created argument is registered to the `command` and returned.
DNF_API libdnf5::cli::ArgumentParser::NamedArg * create_installed_from_repo_option(
    Command & command, std::vector<std::string> & repo_ids, bool detect_conflict);

}  // namespace dnf5

#endif  // DNF5_COMMANDS_SHARED_OPTIONS_HPP
