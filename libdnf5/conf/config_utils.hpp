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

#ifndef LIBDNF5_CONF_CONFIG_PRIVATE_HPP
#define LIBDNF5_CONF_CONFIG_PRIVATE_HPP

#include "libdnf5/conf/config_parser.hpp"
#include "libdnf5/conf/option.hpp"
#include "libdnf5/conf/option_bool.hpp"

#include <string>


namespace libdnf5 {


/// @brief Replaces globs (like /etc/foo.d/\\*.foo) by content of matching files.
///
/// Ignores comment lines (start with '#') and blank lines in files.
/// Result:
/// Words delimited by spaces. Characters ',' and '\n' are replaced by spaces.
/// Extra spaces are removed.
/// @param strWithGlobs Input string with globs
/// @return Words delimited by space
std::string resolve_path_globs(const std::string & str_with_globs, const std::filesystem::path & installroot);


/// Expands the gpgcheck option into sub-options (pkg_gpgcheck, repo_gpgcheck, localpkg_gpgcheck)
/// according to the gpgcheck_policy. Templated because ConfigMain uses OptionBool while
/// ConfigRepo uses OptionChild<OptionBool> for these options.
template <typename PkgGpgcheckOpt, typename RepoGpgcheckOpt>
void apply_gpgcheck_policy(
    const ConfigParser & parser,
    const std::string & section,
    Option::Priority priority,
    Option::Priority gpgcheck_priority,
    bool gpgcheck_val,
    const std::string & policy,
    PkgGpgcheckOpt & pkg_gpgcheck,
    RepoGpgcheckOpt & repo_gpgcheck,
    OptionBool * localpkg_gpgcheck = nullptr) {
    if (gpgcheck_priority < priority) {
        return;
    }

    bool pkg_gpgcheck_explicit = false;
    bool repo_gpgcheck_explicit = false;
    bool localpkg_gpgcheck_explicit = false;
    auto section_iter = parser.get_data().find(section);
    if (section_iter != parser.get_data().end()) {
        pkg_gpgcheck_explicit = section_iter->second.find("pkg_gpgcheck") != section_iter->second.end();
        repo_gpgcheck_explicit = section_iter->second.find("repo_gpgcheck") != section_iter->second.end();
        if (localpkg_gpgcheck) {
            localpkg_gpgcheck_explicit = section_iter->second.find("localpkg_gpgcheck") != section_iter->second.end();
        }
    }

    if (!pkg_gpgcheck_explicit) {
        pkg_gpgcheck.set(priority, gpgcheck_val);
    }

    if (policy == "full" || policy == "all") {
        if (!repo_gpgcheck_explicit) {
            repo_gpgcheck.set(priority, gpgcheck_val);
        }
    }

    if (policy == "all" && localpkg_gpgcheck) {
        if (!localpkg_gpgcheck_explicit) {
            localpkg_gpgcheck->set(priority, gpgcheck_val);
        }
    }
}


}  // namespace libdnf5

#endif
