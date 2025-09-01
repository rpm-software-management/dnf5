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


#ifndef DNF5_COMMANDS_HISTORY_ARGUMENTS_HPP
#define DNF5_COMMANDS_HISTORY_ARGUMENTS_HPP

#include "dnf5/context.hpp"

#include <libdnf5-cli/session.hpp>
#include <libdnf5/utils/bgettext/bgettext-lib.h>


namespace dnf5 {


class TransactionSpecArguments : public libdnf5::cli::session::StringArgumentList {
public:
    explicit TransactionSpecArguments(
        libdnf5::cli::session::Command & command, int nrepeats = libdnf5::cli::ArgumentParser::PositionalArg::UNLIMITED)
        : StringArgumentList(command, "transaction-id", _("Transaction ID"), nrepeats) {}
};


class ReverseOption : public libdnf5::cli::session::BoolOption {
public:
    explicit ReverseOption(libdnf5::cli::session::Command & command)
        : BoolOption(command, "reverse", '\0', _("Reverse the order of transactions."), false) {}
};

class IgnoreInstalledOption : public libdnf5::cli::session::BoolOption {
public:
    explicit IgnoreInstalledOption(libdnf5::cli::session::Command & command)
        : BoolOption(
              command,
              "ignore-installed",
              '\0',
              _("Don't consider mismatches between installed and stored transaction packages as errors. This can "
                "result in an empty transaction because among other things the option can ignore failing Remove "
                "actions."),
              false) {}
};

class IgnoreExtrasOption : public libdnf5::cli::session::BoolOption {
public:
    explicit IgnoreExtrasOption(libdnf5::cli::session::Command & command)
        : BoolOption(
              command,
              "ignore-extras",
              '\0',
              _("Don't consider extra packages pulled into the transaction as errors."),
              false) {}
};

std::function<std::vector<std::string>(const char * arg)> create_history_id_autocomplete(Context & ctx);

class HistoryContainsPkgsOption : public libdnf5::cli::session::AppendStringListOption {
public:
    explicit HistoryContainsPkgsOption(libdnf5::cli::session::Command & command)
        : AppendStringListOption(
              command,
              "contains-pkgs",
              '\0',
              _("Show only transactions containing packages with specified names. List option, supports globs."),
              _("PACKAGE_NAME,...")) {}
};

}  // namespace dnf5


#endif  // DNF5_COMMANDS_ADVISORY_ARGUMENTS_HPP
