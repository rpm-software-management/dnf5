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


#ifndef DNF5_COMMANDS_ARGUMENTS_HPP
#define DNF5_COMMANDS_ARGUMENTS_HPP


#include "utils/string.hpp"

#include <libdnf5-cli/session.hpp>
#include <libdnf5/advisory/advisory_query.hpp>
#include <libdnf5/base/base_weak.hpp>
#include <libdnf5/utils/bgettext/bgettext-lib.h>
#include <libdnf5/utils/bgettext/bgettext-mark-domain.h>

#include <iostream>
#include <memory>
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
    const std::vector<std::string> & advisory_cves,
    const bool strict) {
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
        advisories.clear();
        std::unique_ptr<BgettextMessage> specific_error{nullptr};
        // Filter by advisory name
        if (!advisory_names.empty()) {
            auto advisories_names = libdnf5::advisory::AdvisoryQuery(base);
            advisories_names.filter_name(advisory_names);
            if (advisories_names.empty()) {
                specific_error = std::make_unique<BgettextMessage>(
                    BgettextMessage(M_("No advisory found matching the requested name: \"{}\"")));
                if (strict) {
                    throw libdnf5::cli::CommandExitError(
                        1, *specific_error, libdnf5::utils::string::join(advisory_names, ", "));
                } else {
                    std::cerr << libdnf5::utils::sformat(
                                     TM_(*specific_error, 1), libdnf5::utils::string::join(advisory_names, ", "))
                              << std::endl;
                }
            }
            advisories |= advisories_names;
        }

        // Filter by advisory type
        if (!advisory_types.empty()) {
            auto advisories_types = libdnf5::advisory::AdvisoryQuery(base);
            advisories_types.filter_type(advisory_types);
            advisories |= advisories_types;
        }

        // Filter by advisory severity
        if (!advisory_severities.empty()) {
            auto advisories_severities = libdnf5::advisory::AdvisoryQuery(base);
            advisories_severities.filter_severity(advisory_severities);
            advisories |= advisories_severities;
        }

        // Filter by advisory bz
        if (!advisory_bzs.empty()) {
            auto advisories_bzs = libdnf5::advisory::AdvisoryQuery(base);
            advisories_bzs.filter_reference(advisory_bzs, {"bugzilla"});
            if (advisories_bzs.empty()) {
                specific_error = std::make_unique<BgettextMessage>(
                    BgettextMessage(M_("No advisory found fixing the Bugzilla ID: \"{}\"")));
                if (strict) {
                    throw libdnf5::cli::CommandExitError(
                        1, *specific_error, libdnf5::utils::string::join(advisory_bzs, ", "));
                } else {
                    std::cerr << libdnf5::utils::sformat(
                                     TM_(*specific_error, 1), libdnf5::utils::string::join(advisory_bzs, ", "))
                              << std::endl;
                }
            }
            advisories |= advisories_bzs;
        }

        // Filter by advisory cve
        if (!advisory_cves.empty()) {
            auto advisories_cves = libdnf5::advisory::AdvisoryQuery(base);
            advisories_cves.filter_reference(advisory_cves, {"cve"});
            if (advisories_cves.empty()) {
                specific_error = std::make_unique<BgettextMessage>(
                    BgettextMessage(M_("No advisory found fixing the CVE ID: \"{}\"")));
                if (strict) {
                    throw libdnf5::cli::CommandExitError(
                        1, *specific_error, libdnf5::utils::string::join(advisory_cves, ", "));
                } else {
                    std::cerr << libdnf5::utils::sformat(
                                     TM_(*specific_error, 1), libdnf5::utils::string::join(advisory_cves, ", "))
                              << std::endl;
                }
            }
            advisories |= advisories_cves;
        }


        if (!specific_error && advisories.empty()) {
            std::cerr << _("No advisory found matching the specified filters.") << std::endl;
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
              _("Include content contained in advisories with specified name. List option."),
              _("ADVISORY_NAME,...")) {}
};


class SecurityOption : public libdnf5::cli::session::BoolOption {
public:
    explicit SecurityOption(libdnf5::cli::session::Command & command)
        : BoolOption(command, "security", '\0', _("Include content contained in security advisories."), false) {}
};


class BugfixOption : public libdnf5::cli::session::BoolOption {
public:
    explicit BugfixOption(libdnf5::cli::session::Command & command)
        : BoolOption(command, "bugfix", '\0', _("Include content contained in bugfix advisories."), false) {}
};


class EnhancementOption : public libdnf5::cli::session::BoolOption {
public:
    explicit EnhancementOption(libdnf5::cli::session::Command & command)
        : BoolOption(command, "enhancement", '\0', _("Include content contained in enhancement advisories."), false) {}
};


class NewpackageOption : public libdnf5::cli::session::BoolOption {
public:
    explicit NewpackageOption(libdnf5::cli::session::Command & command)
        : BoolOption(command, "newpackage", '\0', _("Include content contained in newpackage advisories."), false) {}
};


class AdvisorySeverityOption : public libdnf5::cli::session::AppendStringListOption {
public:
    explicit AdvisorySeverityOption(libdnf5::cli::session::Command & command)
        : AppendStringListOption(
              command,
              "advisory-severities",
              '\0',
              _("Include content contained in advisories with specified severity. List option. Can be "
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
              _("Include content contained in advisories that fix a Bugzilla ID, Eg. 123123. List option."),
              _("BUGZILLA_ID,...")) {}
};

class CveOption : public libdnf5::cli::session::AppendStringListOption {
public:
    explicit CveOption(libdnf5::cli::session::Command & command)
        : AppendStringListOption(
              command,
              "cves",
              '\0',
              _("Include content contained in advisories that fix a CVE (Common Vulnerabilities and Exposures) "
                "ID, Eg. CVE-2201-0123. List option."),
              _("CVE_ID,...")) {}
};


}  // namespace dnf5


#endif  // DNF5_COMMANDS_ARGUMENTS_HPP
