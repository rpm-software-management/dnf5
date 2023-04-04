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


#ifndef LIBDNF_CLI_OUTPUT_SEARCH_HPP
#define LIBDNF_CLI_OUTPUT_SEARCH_HPP


#include "libdnf-cli/tty.hpp"

#include <libdnf-cli/tty.hpp>
#include <libdnf/rpm/nevra.hpp>
#include <libdnf/rpm/package.hpp>
#include <libdnf/utils/patterns.hpp>
#include <libsmartcols/libsmartcols.h>

#include <algorithm>
#include <iostream>
#include <numeric>
#include <regex>
#include <set>
#include <utility>
#include <vector>


namespace libdnf::cli::output {

using namespace libdnf::cli::tty;

/// Type of the match against the related key.
enum class SearchKeyMatch {
    PARTIAL,  ///< When the pattern is contained within the given key.
    EXACT     ///< When the pattern exactly matches the given key.
};

// search table columns
enum { COL_SEARCH_NAME, COL_SEARCH_DESC };

/// Defines a type for a key that was matched together with a type of the match.
using matched_key_pair = std::pair<std::string, SearchKeyMatch>;

/// Options used with the search command that affect the output.
struct SearchOptions {
    bool search_all;       ///< If we search also in description and URL fields in addition to name, summary.
    bool show_duplicates;  ///< If multiple versions of the same package are allowed in the output.
};

/// Auxilliary structure for holding the set of result packages together with their comparator.
struct SearchPackages {
    std::set<libdnf::rpm::Package, decltype(&libdnf::rpm::cmp_nevra<libdnf::rpm::Package>)> packages{
        &libdnf::rpm::cmp_nevra<libdnf::rpm::Package>};
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

///////////////////

/// Get a string representation of a key that was matched together with a type of the match.
static std::string key_to_string(const matched_key_pair & key_pair) {
    auto & [key, key_match] = key_pair;
    if (key_match == SearchKeyMatch::PARTIAL) {
        return key;
    } else {
        return key + " (exact)";
    }
}

/// Auxilliary method for aggregating a list of matched keys into a string.
static std::string concat_keys(const std::string & acc, const matched_key_pair & pair) {
    return acc.empty() ? key_to_string(pair) : acc + ", " + key_to_string(pair);
}
/// Construct a string representation of a list of matched keys.
static std::string construct_keys_string(const std::vector<matched_key_pair> & key_pairs) {
    return std::accumulate(key_pairs.begin(), key_pairs.end(), std::string{}, concat_keys);
}

/// Auxilliary method for aggregating a pattern matching expression into a string.
static std::string concat_patterns(const std::string & acc, const std::string & pattern) {
    return acc.empty() ? pattern : acc + "|" + pattern;
}

/// Construct a regular expression for matching all occurrences of any given pattern in the text.
static std::regex construct_patterns_regex(std::vector<std::string> patterns) {
    // Glob patterns are skipped from highlighting for now.
    std::erase_if(patterns, [](const auto & pattern) { return libdnf::utils::is_glob_pattern(pattern.c_str()); });
    auto inner_patterns_regex_str = std::accumulate(patterns.begin(), patterns.end(), std::string{}, concat_patterns);
    return std::regex("(" + inner_patterns_regex_str + ")", std::regex_constants::icase);
}

/// Construct a highlighting regex replacement format string.
/// In the end this is just wrapping all matched patterns with green highlighting markup.
static std::string construct_highlight_regex_format() {
    std::stringstream replace_format;
    replace_format << green << "$1" << reset;
    return replace_format.str();
}

static void add_line_into_search_table(struct libscols_table * table, const char * name, const char * desc) {
    struct libscols_line * ln = scols_table_new_line(table, NULL);
    scols_line_set_data(ln, COL_SEARCH_NAME, name);
    scols_line_set_data(ln, COL_SEARCH_DESC, desc);
}

template <class Results>
static struct libscols_table * create_search_table(Results results) {
    struct libscols_table * table = scols_new_table();
    if (libdnf::cli::tty::is_interactive()) {
        scols_table_enable_colors(table, 1);
        scols_table_enable_maxout(table, 1);
    }
    struct libscols_column * cl = scols_table_new_column(table, "Search results", 1, SCOLS_FL_TRUNC);
    scols_column_set_cmpfunc(cl, scols_cmpstr_cells, NULL);
    scols_table_new_column(table, "name", 0.5, SCOLS_FL_TRUNC);
    scols_table_new_column(table, "desc", 0.1, SCOLS_FL_RIGHT);

    auto patterns_regex = construct_patterns_regex(results.patterns);
    auto highlight_regex_format = construct_highlight_regex_format();
    auto highlight = [&](auto const & text) {
        return std::regex_replace(text, patterns_regex, highlight_regex_format);
    };

    for (auto const & result : results.group_results) {
        add_line_into_search_table(
            table,
            // bold + "Matched fields: " + reset + construct_keys_string(result.matched_keys),
            (std::string("Matched fields: ") + construct_keys_string(result.matched_keys)).c_str(),
            "");

        for (auto const & package : result.matched_packages.packages) {
            std::cout << " ";
            if (results.options.show_duplicates) {
                add_line_into_search_table(
                    table, highlight(package.get_full_nevra()).c_str(), highlight(package.get_summary()).c_str());
            } else {
                add_line_into_search_table(
                    table,
                    (highlight(package.get_name()) + "." + package.get_arch()).c_str(),
                    highlight(package.get_summary()).c_str());
            }
        }
    }
    return table;
}

template <class Results>
static void print_search_table(Results results) {
    if (results.group_results.empty()) {
        std::cout << "No matches found." << std::endl;
        return;
    }
    auto table = create_search_table(results);
    scols_print_table(table);
    scols_unref_table(table);
}

}  // namespace libdnf::cli::output

#endif  // LIBDNF_CLI_OUTPUT_SEARCH_HPP
