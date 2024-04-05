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


#include <libdnf5-cli/session.hpp>
#include <libdnf5/utils/bgettext/bgettext-lib.h>


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


class AdvisoryNameArguments : public libdnf5::cli::session::StringArgumentList {
public:
    explicit AdvisoryNameArguments(libdnf5::cli::session::Command & command)
        : StringArgumentList(command, "advisory-name", _("List of patterns matched against advisory names.")) {}
};


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


class AdvisoryOption : public libdnf5::cli::session::AppendStringListOption {
public:
    explicit AdvisoryOption(libdnf5::cli::session::Command & command)
        : AppendStringListOption(
              command,
              "advisories",
              '\0',
              _("Limit to packages in advisories with specified name. List option."),
              _("ADVISORY_NAME,...")) {}
};


class SecurityOption : public libdnf5::cli::session::BoolOption {
public:
    explicit SecurityOption(libdnf5::cli::session::Command & command)
        : BoolOption(command, "security", '\0', _("Limit to packages in security advisories."), false) {}
};


class BugfixOption : public libdnf5::cli::session::BoolOption {
public:
    explicit BugfixOption(libdnf5::cli::session::Command & command)
        : BoolOption(command, "bugfix", '\0', _("Limit to packages in bugfix advisories."), false) {}
};


class EnhancementOption : public libdnf5::cli::session::BoolOption {
public:
    explicit EnhancementOption(libdnf5::cli::session::Command & command)
        : BoolOption(command, "enhancement", '\0', _("Limit to packages in enhancement advisories."), false) {}
};


class NewpackageOption : public libdnf5::cli::session::BoolOption {
public:
    explicit NewpackageOption(libdnf5::cli::session::Command & command)
        : BoolOption(command, "newpackage", '\0', _("Limit to packages in newpackage advisories."), false) {}
};


class AdvisorySeverityOption : public libdnf5::cli::session::AppendStringListOption {
public:
    explicit AdvisorySeverityOption(libdnf5::cli::session::Command & command)
        : AppendStringListOption(
              command,
              "advisory-severities",
              '\0',
              /* Note for translators: "critical" etc. quoted words are
                 literals that should not be translated. */
              _("Limit to packages in advisories with specified severity. List option. Can be "
                "\"critical\", \"important\", \"moderate\", \"low\", \"none\"."),
              _("ADVISORY_SEVERITY,..."),
              "critical|important|moderate|low|none",
              true) {}
};

class BzOption : public libdnf5::cli::session::AppendStringListOption {
public:
    explicit BzOption(libdnf5::cli::session::Command & command)
        : AppendStringListOption(
              command,
              "bzs",
              '\0',
              _("Limit to packages in advisories that fix a Bugzilla ID, Eg. 123123. List option."),
              _("BUGZILLA_ID,...")) {}
};

class CveOption : public libdnf5::cli::session::AppendStringListOption {
public:
    explicit CveOption(libdnf5::cli::session::Command & command)
        : AppendStringListOption(
              command,
              "cves",
              '\0',
              _("Limit to packages in advisories that fix a CVE (Common Vulnerabilities and Exposures) "
                "ID, Eg. CVE-2201-0123. List option."),
              _("CVE_ID,...")) {}
};

#endif  // DNF5_COMMANDS_ADVISORY_ARGUMENTS_HPP
