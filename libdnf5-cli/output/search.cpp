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


#include "libdnf5-cli/output/search.hpp"

#include "libdnf5-cli/tty.hpp"

#include <libdnf5/rpm/package_query.hpp>
#include <libdnf5/utils/bgettext/bgettext-lib.h>
#include <libdnf5/utils/patterns.hpp>

#include <algorithm>
#include <iostream>
#include <numeric>
#include <optional>
#include <regex>
#include <set>

namespace libdnf5::cli::output {

namespace {

using namespace libdnf5::cli::tty;

/// Get a string representation of a key that was matched together with a type of the match.
std::string key_to_string(const matched_key_pair & key_pair) {
    auto & [key, key_match] = key_pair;
    if (key_match == SearchKeyMatch::PARTIAL) {
        return key;
    } else {
        return key + _(" (exact)");
    }
}


/// Auxiliary method for aggregating a list of matched keys into a string.
std::string concat_keys(const std::string & acc, const matched_key_pair & pair) {
    return acc.empty() ? key_to_string(pair) : acc + ", " + key_to_string(pair);
}


/// Construct a string representation of a list of matched keys.
std::string construct_keys_string(const std::vector<matched_key_pair> & key_pairs) {
    return std::accumulate(key_pairs.begin(), key_pairs.end(), std::string{}, concat_keys);
}


/// Auxiliary method for aggregating a pattern matching expression into a string.
std::string concat_patterns(const std::string & acc, const std::string & pattern) {
    return acc.empty() ? pattern : acc + "|" + pattern;
}


/// Construct a regular expression for matching all occurrences of any given pattern in the text.
std::regex construct_patterns_regex(std::vector<std::string> patterns) {
    // Glob patterns are skipped from highlighting for now.
    std::erase_if(patterns, [](const auto & pattern) { return libdnf5::utils::is_glob_pattern(pattern.c_str()); });
    auto inner_patterns_regex_str = std::accumulate(patterns.begin(), patterns.end(), std::string{}, concat_patterns);
    return std::regex("(" + inner_patterns_regex_str + ")", std::regex_constants::icase);
}


/// Construct a highlighting regex replacement format string.
/// In the end this is just wrapping all matched patterns with green highlighting markup.
std::string construct_highlight_regex_format() {
    std::stringstream replace_format;
    replace_format << green << "$1" << reset;
    return replace_format.str();
}


/// Get a string identifying the package in the search results.
/// Without duplicates only the latest version of each name.arch is shown, which may not be the installed
/// one, so packages are identified by name.arch. With duplicates all versions are shown, so by full NEVRA.
std::string get_package_id(const libdnf5::rpm::Package & package, bool show_duplicates) {
    if (show_duplicates) {
        return package.get_full_nevra();
    }
    return package.get_name() + "." + package.get_arch();
}


/// Get the identifiers of all installed packages, see get_package_id().
std::set<std::string> get_installed_package_ids(const libdnf5::BaseWeakPtr & base, bool show_duplicates) {
    std::set<std::string> installed_ids;
    // Excludes do not affect whether a package is installed.
    libdnf5::rpm::PackageQuery installed(base, libdnf5::sack::ExcludeFlags::IGNORE_EXCLUDES);
    installed.filter_installed();
    for (auto const & package : installed) {
        installed_ids.insert(get_package_id(package, show_duplicates));
    }
    return installed_ids;
}

}  // namespace


void print_search_results(const SearchResults & results) {
    if (results.group_results.empty()) {
        return;
    }

    auto patterns_regex = construct_patterns_regex(results.patterns);
    auto highlight_regex_format = construct_highlight_regex_format();
    auto highlight = [&](auto const & text) {
        return std::regex_replace(text, patterns_regex, highlight_regex_format);
    };
    std::optional<std::set<std::string>> installed_ids;

    for (auto const & result : results.group_results) {
        std::cout << bold << _("Matched fields: ") << reset << construct_keys_string(result.matched_keys) << std::endl;

        for (auto const & package : result.matched_packages.packages) {
            if (!installed_ids) {
                installed_ids = get_installed_package_ids(package.get_base(), results.options.show_duplicates);
            }
            std::cout << " ";
            if (results.options.show_duplicates) {
                std::cout << highlight(package.get_full_nevra());
            } else {
                std::cout << highlight(package.get_name()) << "." << package.get_arch();
            }
            std::cout << "\t" << highlight(package.get_summary());
            if (installed_ids->contains(get_package_id(package, results.options.show_duplicates))) {
                std::cout << " " << _("[installed]");
            }
            std::cout << std::endl;
        }
    }
}

}  // namespace libdnf5::cli::output
