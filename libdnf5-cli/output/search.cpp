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
#include <libsmartcols/libsmartcols.h>

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

    std::unique_ptr<libscols_table, decltype(&scols_unref_table)> table(scols_new_table(), &scols_unref_table);

    if (!libdnf5::cli::tty::is_interactive()) {
        // Do not hard-code 80 as non-interactive screen width. Let libdnf5::cli::tty to decide
        auto screen_width = size_t(libdnf5::cli::tty::get_width());
        scols_table_set_termwidth(table.get(), screen_width);
        // The below is necessary to make the libsmartcols' truncation work with non-interactive terminal
        scols_table_set_termforce(table.get(), SCOLS_TERMFORCE_ALWAYS);
    }

    const std::vector<std::tuple<const char *, double, int>> columns = {
        {_("Package"), 0.15, 0}, {_("Description"), 0.7, SCOLS_FL_TRUNC}, {_("Matched fields"), 0.15, 0}};

    for (auto [name, whint, flags] : columns) {
        scols_table_new_column(table.get(), name, whint, flags);
    }

    for (auto const & result : results.group_results) {
        auto matched_fields = construct_keys_string(result.matched_keys);
        for (auto const & package : result.matched_packages.packages) {
            struct libscols_line * ln = scols_table_new_line(table.get(), NULL);
            std::string name;
            if (results.options.show_duplicates) {
                name = package.get_full_nevra();
            } else {
                name = package.get_name() + "." + package.get_arch();
            }
            scols_line_set_data(ln, 0, name.c_str());
            // Regretfully we can not use the highlight for the below because of truncation
            scols_line_set_data(ln, 1, package.get_summary().c_str());
            scols_line_set_data(ln, 2, matched_fields.c_str());
        }
    }

    char * output_buffer = NULL;
    size_t output_size = 0;
    FILE * output_stream;
    output_stream = open_memstream(&output_buffer, &output_size);
    scols_table_set_stream(table.get(), output_stream);
    scols_print_table(table.get());
    char * line = NULL;
    size_t size = 0;
    // Printing table header in bold text
    if (getline(&line, &size, output_stream) == -1) {
        std::cout << _("Unable to retrieve search results.") << std::endl;
        free(line);
        return;
    }
    std::cout << bold << std::string(line) << reset;
    // Printing the rest of table with highlighting (only for first 2 columns)
    auto pkg_n_desc_width = scols_column_get_width(scols_table_get_column(table.get(), 0));
    pkg_n_desc_width += scols_column_get_width(scols_table_get_column(table.get(), 1));
    pkg_n_desc_width += 2;  // extra space between columns
    while (getline(&line, &size, output_stream) != -1) {
        auto pkg_n_desc = std::string(line).substr(0, pkg_n_desc_width);
        auto rest = std::string(line).substr(pkg_n_desc_width, std::string::npos);
        // nitpick: The highlight does not work if the matching word is truncated
        std::cout << highlight(pkg_n_desc) << rest;
    }
    free(line);
    fclose(output_stream);
}

}  // namespace libdnf5::cli::output
