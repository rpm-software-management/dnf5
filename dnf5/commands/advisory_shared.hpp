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


#ifndef DNF5_COMMANDS_ARGUMENTS_HPP
#define DNF5_COMMANDS_ARGUMENTS_HPP


#include "utils/string.hpp"

#include <libdnf5-cli/session.hpp>
#include <libdnf5/advisory/advisory_query.hpp>
#include <libdnf5/base/base_weak.hpp>
#include <libdnf5/utils/bgettext/bgettext-lib.h>

#include <optional>


namespace dnf5 {

inline std::optional<libdnf5::advisory::AdvisoryQuery> advisory_query_from_cli_input(
    libdnf5::Base & base,
    const std::vector<std::string> & advisory_names,
    bool advisory_security,
    bool advisory_bugfix,
    bool advisory_enhancement,
    bool advisory_newpackage,
    const std::vector<std::string> & advisory_severities,
    const std::vector<std::string> & advisory_bzs,
    const std::vector<std::string> & advisory_cves) {
    std::vector<std::string> advisory_types;
    if (advisory_security) {
        advisory_types.emplace_back("security");
    }
    if (advisory_bugfix) {
        advisory_types.emplace_back("bugfix");
    }
    if (advisory_enhancement) {
        advisory_types.emplace_back("enhancement");
    }
    if (advisory_newpackage) {
        advisory_types.emplace_back("newpackage");
    }

    if (!advisory_types.empty() || !advisory_severities.empty() || !advisory_names.empty() || !advisory_bzs.empty() ||
        !advisory_cves.empty()) {
        auto advisories = libdnf5::advisory::AdvisoryQuery(base);
        // Filter by advisory name
        if (!advisory_names.empty()) {
            advisories.filter_name(advisory_names);
        }

        // Filter by advisory type
        if (!advisory_types.empty()) {
            advisories.filter_type(advisory_types);
        }

        // Filter by advisory severity
        if (!advisory_severities.empty()) {
            advisories.filter_severity(advisory_severities);
        }

        // Filter by advisory bz
        if (!advisory_bzs.empty()) {
            advisories.filter_reference(advisory_bzs, {"bugzilla"});
        }

        // Filter by advisory cve
        if (!advisory_cves.empty()) {
            advisories.filter_reference(advisory_cves, {"cve"});
        }

        return advisories;
    }

    return std::nullopt;
}

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


}  // namespace dnf5


#endif  // DNF5_COMMANDS_ARGUMENTS_HPP
