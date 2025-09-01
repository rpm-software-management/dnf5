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


#ifndef DNF5_COMMANDS_ADVISORY_ARGUMENTS_HPP
#define DNF5_COMMANDS_ADVISORY_ARGUMENTS_HPP


#include <libdnf5-cli/session.hpp>
#include <libdnf5/utils/bgettext/bgettext-lib.h>

namespace dnf5 {


class AdvisoryAllOption : public libdnf5::cli::session::BoolOption {
public:
    explicit AdvisoryAllOption(libdnf5::cli::session::Command & command)
        : BoolOption(command, "all", '\0', _("Show advisories containing any version of installed packages."), false) {}
};


class AdvisoryAvailableOption : public libdnf5::cli::session::BoolOption {
public:
    explicit AdvisoryAvailableOption(libdnf5::cli::session::Command & command)
        : BoolOption(
              command,
              "available",
              '\0',
              _("Show advisories containing newer versions of installed packages. This is the default behavior."),
              false) {}
};


class AdvisoryInstalledOption : public libdnf5::cli::session::BoolOption {
public:
    explicit AdvisoryInstalledOption(libdnf5::cli::session::Command & command)
        : BoolOption(
              command,
              "installed",
              '\0',
              _("Show advisories containing equal and older versions of installed packages."),
              false) {}
};


class AdvisoryUpdatesOption : public libdnf5::cli::session::BoolOption {
public:
    explicit AdvisoryUpdatesOption(libdnf5::cli::session::Command & command)
        : BoolOption(
              command,
              "updates",
              '\0',
              _("Show advisories containing newer versions of installed packages for which a newer version is "
                "available."),
              false) {}
};


class AdvisoryContainsPkgsOption : public libdnf5::cli::session::AppendStringListOption {
public:
    explicit AdvisoryContainsPkgsOption(libdnf5::cli::session::Command & command)
        : AppendStringListOption(
              command,
              "contains-pkgs",
              '\0',
              _("Show only advisories containing packages with specified names. List option, supports globs."),
              _("PACKAGE_NAME,...")) {}
};


class AdvisorySpecArguments : public libdnf5::cli::session::StringArgumentList {
public:
    explicit AdvisorySpecArguments(libdnf5::cli::session::Command & command)
        : StringArgumentList(command, "advisory-spec", _("List of patterns matched against advisory names.")) {}
};


}  // namespace dnf5


class AdvisoryWithBzOption : public libdnf5::cli::session::BoolOption {
public:
    explicit AdvisoryWithBzOption(libdnf5::cli::session::Command & command)
        : BoolOption(command, "with-bz", '\0', _("Show only advisories referencing a bugzilla."), false) {}
};


class AdvisoryWithCveOption : public libdnf5::cli::session::BoolOption {
public:
    explicit AdvisoryWithCveOption(libdnf5::cli::session::Command & command)
        : BoolOption(command, "with-cve", '\0', _("Show only advisories referencing a CVE."), false) {}
};


#endif  // DNF5_COMMANDS_ADVISORY_ARGUMENTS_HPP
