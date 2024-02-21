/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/


#include "search_processor.hpp"

#include <libdnf5/utils/patterns.hpp>

#include <functional>
#include <map>
#include <set>

namespace dnf5 {

using namespace libdnf5::cli::output;

/// Definition of all package metadata fields keys used for searching.
static const char * SEARCH_KEY_NAME = "name";
static const char * SEARCH_KEY_SUMMARY = "summary";
static const char * SEARCH_KEY_DESCRIPTION = "description";
static const char * SEARCH_KEY_URL = "url";

static const char * const SEARCH_KEYS[] = {SEARCH_KEY_NAME, SEARCH_KEY_SUMMARY, SEARCH_KEY_DESCRIPTION, SEARCH_KEY_URL};

/// Definition of priority values for each search key.
/// The values are defined so that original keys could be later decomposed from the value.
/// Also there is always a "bitwise gap" between the values used for determining the exact key matches priority.
static const std::map<const char *, int> SEARCH_KEYS_PRIORITIES = {
    {SEARCH_KEY_URL, 1 << 0},
    {SEARCH_KEY_DESCRIPTION, 1 << 2},
    {SEARCH_KEY_SUMMARY, 1 << 4},
    {SEARCH_KEY_NAME, 1 << 6},
};

static int get_search_key_priority(const char * key) {
    return SEARCH_KEYS_PRIORITIES.at(key);
}

/// Calculate a list of matched keys based on the priority value.
static auto get_matched_keys(int priority) {
    std::vector<matched_key_pair> matched_keys;
    for (auto key : SEARCH_KEYS) {
        auto key_priority = get_search_key_priority(key);
        if ((key_priority << 1) & priority) {
            matched_keys.push_back(std::make_pair(key, SearchKeyMatch::EXACT));
        } else if (key_priority & priority) {
            matched_keys.push_back(std::make_pair(key, SearchKeyMatch::PARTIAL));
        }
    }
    return matched_keys;
}

/// Get comparing method based on the pattern input representation.
static auto get_cmp_from_pattern(const std::string & pattern) {
    if (libdnf5::utils::is_glob_pattern(pattern.c_str())) {
        return libdnf5::sack::QueryCmp::IGLOB;
    } else {
        return libdnf5::sack::QueryCmp::ICONTAINS;
    }
}

SearchProcessor::SearchProcessor(
    libdnf5::Base & base, std::vector<std::string> patterns, bool search_all, bool show_duplicates)
    : base(base),
      patterns(patterns),
      search_all(search_all),
      full_package_query(base, libdnf5::sack::ExcludeFlags::IGNORE_VERSIONLOCK),
      showdupes(show_duplicates) {
    if (!showdupes) {
        full_package_query.filter_latest_evr();
    }
}

libdnf5::rpm::PackageSet SearchProcessor::get_matches(
    const std::string & pattern, int priority, filter_method_type filter) {
    auto cmp = get_cmp_from_pattern(pattern);

    libdnf5::rpm::PackageQuery query_contains(full_package_query);
    (query_contains.*filter)({pattern}, cmp);
    update_priorities(query_contains, priority);

    if (patterns.size() == 1 && cmp != libdnf5::sack::QueryCmp::IGLOB) {
        libdnf5::rpm::PackageQuery query_exact(query_contains);
        (query_exact.*filter)({pattern}, libdnf5::sack::QueryCmp::EXACT);
        update_priorities(query_exact, (priority << 1));
    }

    return query_contains;
}

libdnf5::rpm::PackageSet SearchProcessor::get_name_matches(const std::string & pattern) {
    return get_matches(pattern, get_search_key_priority(SEARCH_KEY_NAME), &libdnf5::rpm::PackageQuery::filter_name);
}

libdnf5::rpm::PackageSet SearchProcessor::get_summary_matches(const std::string & pattern) {
    return get_matches(
        pattern, get_search_key_priority(SEARCH_KEY_SUMMARY), &libdnf5::rpm::PackageQuery::filter_summary);
}

libdnf5::rpm::PackageSet SearchProcessor::get_description_matches(const std::string & pattern) {
    return get_matches(
        pattern, get_search_key_priority(SEARCH_KEY_DESCRIPTION), &libdnf5::rpm::PackageQuery::filter_description);
}

libdnf5::rpm::PackageSet SearchProcessor::get_url_matches(const std::string & pattern) {
    return get_matches(pattern, get_search_key_priority(SEARCH_KEY_URL), &libdnf5::rpm::PackageQuery::filter_url);
}

void SearchProcessor::update_priorities(const libdnf5::rpm::PackageSet & packages, int priority) {
    for (auto const & package : packages) {
        packages_priorities[package.get_full_nevra()] |= priority;
    }
}

SearchResults SearchProcessor::get_results() {
    libdnf5::rpm::PackageSet all_matches(base);

    for (auto it = patterns.begin(); it != patterns.end(); ++it) {
        libdnf5::rpm::PackageSet pattern_matches(base);
        pattern_matches |= get_name_matches(*it);
        pattern_matches |= get_summary_matches(*it);

        if (search_all) {
            pattern_matches |= get_description_matches(*it);
            pattern_matches |= get_url_matches(*it);
        }

        // For the first pattern we are always adding to the empty list.
        // If we are using the "--all" option, we want any matches in the result.
        // Otherwise we use an intersection (AND) with all previous results.
        if (it == patterns.begin() || search_all) {
            all_matches.update(std::move(pattern_matches));
        } else {
            all_matches.intersection(std::move(pattern_matches));
        }
    }

    // Aggregate packages into the groups based on their priorities.
    // This results in having packages grouped by the matched keys.
    // Results are sorted with the highest priorities first.
    std::map<int, SearchPackages, std::greater<int>> priority_matches;
    for (auto const & package : all_matches) {
        auto priority = packages_priorities[package.get_full_nevra()];
        priority_matches[priority].packages.insert(std::move(package));
    }

    // In the end just push everything into the final result structure.
    SearchResults results;
    for (auto const & [priority, packages] : priority_matches) {
        results.group_results.push_back(
            {.matched_keys = get_matched_keys(priority), .matched_packages = std::move(packages)});
    }
    results.options = {.search_all = search_all, .show_duplicates = showdupes};
    results.patterns = patterns;

    return results;
}

}  // namespace dnf5
