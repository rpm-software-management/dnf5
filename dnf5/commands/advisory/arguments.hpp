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


#ifndef DNF5_COMMANDS_ADVISORY_ARGUMENTS_HPP
#define DNF5_COMMANDS_ADVISORY_ARGUMENTS_HPP


#include "utils/bgettext/bgettext-lib.h"

#include <libdnf-cli/session.hpp>


namespace dnf5 {


class AdvisoryAllOption : public libdnf::cli::session::BoolOption {
public:
    explicit AdvisoryAllOption(libdnf::cli::session::Command & command)
        : BoolOption(command, "all", '\0', _("Show advisories about any version of installed packages."), false) {}
};


class AdvisoryAvailableOption : public libdnf::cli::session::BoolOption {
public:
    explicit AdvisoryAvailableOption(libdnf::cli::session::Command & command)
        : BoolOption(
              command, "available", '\0', _("Show advisories about newer versions of installed packages."), false) {}
};


class AdvisoryInstalledOption : public libdnf::cli::session::BoolOption {
public:
    explicit AdvisoryInstalledOption(libdnf::cli::session::Command & command)
        : BoolOption(
              command,
              "installed",
              '\0',
              _("Show advisories about equal and older versions of installed packages."),
              false) {}
};


class AdvisoryUpdatesOption : public libdnf::cli::session::BoolOption {
public:
    explicit AdvisoryUpdatesOption(libdnf::cli::session::Command & command)
        : BoolOption(
              command,
              "updates",
              '\0',
              _("Show advisories about newer versions of installed packages for which a newer version is available."),
              false) {}
};


class AdvisorySpecArguments : public libdnf::cli::session::StringArgumentList {
public:
    explicit AdvisorySpecArguments(libdnf::cli::session::Command & command)
        : StringArgumentList(command, "advisory-spec", _("List of patterns matched against advisory names.")) {}
};


}  // namespace dnf5


#endif  // DNF5_COMMANDS_ADVISORY_ARGUMENTS_HPP
