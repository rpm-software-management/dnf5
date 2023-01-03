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


#include "utils/bgettext/bgettext-lib.h"
#include "utils/string.hpp"

#include "libdnf/base/base_weak.hpp"

#include <libdnf-cli/session.hpp>
#include <libdnf/advisory/advisory_query.hpp>

#include <optional>


namespace dnf5 {

inline std::optional<libdnf::advisory::AdvisoryQuery> advisory_query_from_cli_input(
    libdnf::Base & base,
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
        auto advisories = libdnf::advisory::AdvisoryQuery(base);
        advisories.clear();
        // Filter by advisory name
        if (!advisory_names.empty()) {
            auto advisories_names = libdnf::advisory::AdvisoryQuery(base);
            advisories_names.filter_name(advisory_names);
            advisories |= advisories_names;
        }

        // Filter by advisory type
        if (!advisory_types.empty()) {
            auto advisories_types = libdnf::advisory::AdvisoryQuery(base);
            advisories_types.filter_type(advisory_types);
            advisories |= advisories_types;
        }

        // Filter by advisory severity
        if (!advisory_severities.empty()) {
            auto advisories_severities = libdnf::advisory::AdvisoryQuery(base);
            advisories_severities.filter_severity(advisory_severities);
            advisories |= advisories_severities;
        }

        // Filter by advisory bz
        if (!advisory_bzs.empty()) {
            auto advisories_bzs = libdnf::advisory::AdvisoryQuery(base);
            advisories_bzs.filter_reference(advisory_bzs, {"bugzilla"});
            advisories |= advisories_bzs;
        }

        // Filter by advisory cve
        if (!advisory_cves.empty()) {
            auto advisories_cves = libdnf::advisory::AdvisoryQuery(base);
            advisories_cves.filter_reference(advisory_cves, {"cve"});
            advisories |= advisories_cves;
        }

        return advisories;
    }

    return std::nullopt;
}

class AdvisoryOption : public libdnf::cli::session::StringListOption {
public:
    explicit AdvisoryOption(libdnf::cli::session::Command & command)
        : StringListOption(
              command,
              "advisories",
              '\0',
              _("Consider only content contained in advisories with specified name. List option."),
              _("ADVISORY_NAME,...")) {}
};


class SecurityOption : public libdnf::cli::session::BoolOption {
public:
    explicit SecurityOption(libdnf::cli::session::Command & command)
        : BoolOption(command, "security", '\0', _("Consider only content contained in security advisories."), false) {}
};


class BugfixOption : public libdnf::cli::session::BoolOption {
public:
    explicit BugfixOption(libdnf::cli::session::Command & command)
        : BoolOption(command, "bugfix", '\0', _("Consider only content contained in bugfix advisories."), false) {}
};


class EnhancementOption : public libdnf::cli::session::BoolOption {
public:
    explicit EnhancementOption(libdnf::cli::session::Command & command)
        : BoolOption(
              command, "enhancement", '\0', _("Consider only content contained in enhancement advisories."), false) {}
};


class NewpackageOption : public libdnf::cli::session::BoolOption {
public:
    explicit NewpackageOption(libdnf::cli::session::Command & command)
        : BoolOption(
              command, "newpackage", '\0', _("Consider only content contained in newpackage advisories."), false) {}
};


class AdvisorySeverityOption : public libdnf::cli::session::StringListOption {
public:
    explicit AdvisorySeverityOption(libdnf::cli::session::Command & command)
        : StringListOption(
              command,
              "advisory-severities",
              '\0',
              _("Consider only content contained in advisories with specified severity. List option. Can be "
                "\"critical\", \"important\", \"moderate\", \"low\", \"none\"."),
              _("ADVISORY_SEVERITY,..."),
              "critical|important|moderate|low|none",
              true) {}


    std::vector<std::string> get_value() const {
        auto vals = StringListOption::get_value();
        std::transform(vals.begin(), vals.end(), vals.begin(), [](std::string val) -> std::string {
            val = libdnf::utils::string::tolower(val);
            val[0] = static_cast<char>(std::toupper(val[0]));
            return val;
        });

        return vals;
    }
};

class BzOption : public libdnf::cli::session::StringListOption {
public:
    explicit BzOption(libdnf::cli::session::Command & command)
        : StringListOption(
              command,
              "bzs",
              '\0',
              _("Consider only content contained in advisories that fix a Bugzilla ID, Eg. 123123. List option."),
              _("BUGZILLA_ID,...")) {}
};

class CveOption : public libdnf::cli::session::StringListOption {
public:
    explicit CveOption(libdnf::cli::session::Command & command)
        : StringListOption(
              command,
              "cves",
              '\0',
              _("Consider only content contained in advisories that fix a CVE (Common Vulnerabilities and Exposures) "
                "ID, Eg. CVE-2201-0123. List option."),
              _("CVE_ID,...")) {}
};


}  // namespace dnf5


#endif  // DNF5_COMMANDS_ARGUMENTS_HPP
