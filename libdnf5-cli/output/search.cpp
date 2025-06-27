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


#include "libdnf5-cli/output/search.hpp"

#include "libdnf5-cli/tty.hpp"

#include <libdnf5/utils/bgettext/bgettext-lib.h>
#include <libdnf5/utils/patterns.hpp>

#include <algorithm>
#include <iostream>
#include <numeric>
#include <regex>

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

}  // namespace


void print_search_results(const SearchResults & results) {
    if (results.group_results.empty()) {
        std::cout << _("No matches found.") << std::endl;
        return;
    }

    auto patterns_regex = construct_patterns_regex(results.patterns);
    auto highlight_regex_format = construct_highlight_regex_format();
    auto highlight = [&](auto const & text) {
        return std::regex_replace(text, patterns_regex, highlight_regex_format);
    };

    for (auto const & result : results.group_results) {
        std::cout << bold << _("Matched fields: ") << reset << construct_keys_string(result.matched_keys) << std::endl;

        for (auto const & package : result.matched_packages.packages) {
            std::cout << " ";
            if (results.options.show_duplicates) {
                std::cout << highlight(package.get_full_nevra());
            } else {
                std::cout << highlight(package.get_name()) << "." << package.get_arch();
            }
            std::cout << ": " << highlight(package.get_summary());
            if (package.is_installed()) {
                std::cout << " [installed]";
            }
            std::cout << std::endl;
        }
    }
}

}  // namespace libdnf5::cli::output
