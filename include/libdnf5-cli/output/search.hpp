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


#ifndef LIBDNF5_CLI_OUTPUT_SEARCH_HPP
#define LIBDNF5_CLI_OUTPUT_SEARCH_HPP

#include "libdnf5-cli/defs.h"

#include <libdnf5/rpm/nevra.hpp>
#include <libdnf5/rpm/package.hpp>

#include <set>
#include <utility>
#include <vector>

namespace libdnf5::cli::output {

/// Type of the match against the related key.
enum class SearchKeyMatch {
    PARTIAL,  ///< When the pattern is contained within the given key.
    EXACT     ///< When the pattern exactly matches the given key.
};

/// Defines a type for a key that was matched together with a type of the match.
using matched_key_pair = std::pair<std::string, SearchKeyMatch>;

/// Options used with the search command that affect the output.
struct SearchOptions {
    bool search_all;       ///< If we search also in description and URL fields in addition to name, summary.
    bool show_duplicates;  ///< If multiple versions of the same package are allowed in the output.
    bool search_name;      ///< If we search only to the name field
    bool search_summary;   ///< If we search only to the summary field
};

/// Auxiliary structure for holding the set of result packages together with their comparator.
struct SearchPackages {
    std::set<libdnf5::rpm::Package, decltype(&libdnf5::rpm::cmp_nevra<libdnf5::rpm::Package>)> packages{
        &libdnf5::rpm::cmp_nevra<libdnf5::rpm::Package>};
};

/// Search results grouped by the keys that were matched.
struct SearchResult {
    std::vector<matched_key_pair> matched_keys;  ///< List of keys that were matched for this group.
    SearchPackages matched_packages;             ///< List of packages that were matched for this group.
};

/// Aggregating structure for all search results.
struct SearchResults {
    SearchOptions options;                    ///< Search options that were used.
    std::vector<std::string> patterns;        ///< List of patterns to be matched, given by the user.
    std::vector<SearchResult> group_results;  ///< List of results groups.
};

/// @brief Write the search results to the standard output.
/// @param results Structure with already computed search results.
LIBDNF_CLI_API void print_search_results(const SearchResults & results);

}  // namespace libdnf5::cli::output

#endif  // LIBDNF5_CLI_OUTPUT_SEARCH_HPP
