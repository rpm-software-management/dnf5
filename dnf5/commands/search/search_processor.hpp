// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.


#ifndef DNF5_COMMANDS_SEARCH_PROCESSOR_HPP
#define DNF5_COMMANDS_SEARCH_PROCESSOR_HPP

#include <libdnf5-cli/output/search.hpp>
#include <libdnf5/base/base.hpp>
#include <libdnf5/rpm/package_query.hpp>
#include <libdnf5/rpm/package_set.hpp>

#include <unordered_map>
#include <vector>

namespace dnf5 {

/// Class for handling the whole searching logic.
class SearchProcessor {
public:
    /// Prepare the processor based on the parameters given by the user.
    explicit SearchProcessor(
        libdnf5::Base & base, std::vector<std::string> patterns, bool search_all, bool show_duplicates);

    /// Results computation method.
    libdnf5::cli::output::SearchResults get_results();

private:
    /// Define a type for pointing to the filtering method from PackageQuery object.
    using filter_method_type =
        void (libdnf5::rpm::PackageQuery::*)(const std::vector<std::string> &, libdnf5::sack::QueryCmp);

    /// @brief Search for matching packages based on the given parameters.
    ///
    /// When constructing the processor, the package query with all
    /// packages from repositories based on the configuration from base
    /// is stored. Then given filter method is used on the query to get
    /// matching results.
    ///
    /// @param pattern Pattern to be matched against the package metadata fields.
    /// @param priority Value indicating a level of importance of these matched packages.
    /// @param filter Type of the method to be used for filtering the package query.
    /// @return Set of matching packages.
    libdnf5::rpm::PackageSet get_matches(const std::string & pattern, int priority, filter_method_type filter);

    /// @brief Search for matching packages against the name metadata field.
    /// @param pattern Pattern to be matched against the name field.
    /// @return Set of matching packages.
    libdnf5::rpm::PackageSet get_name_matches(const std::string & pattern);

    /// @brief Search for matching packages against the summary metadata field.
    /// @param pattern Pattern to be matched against the summary field.
    /// @return Set of matching packages.
    libdnf5::rpm::PackageSet get_summary_matches(const std::string & pattern);

    /// @brief Search for matching packages against the description metadata field.
    /// @param pattern Pattern to be matched against the description field.
    /// @return Set of matching packages.
    libdnf5::rpm::PackageSet get_description_matches(const std::string & pattern);

    /// @brief Search for matching packages against the URL metadata field.
    /// @param pattern Pattern to be matched against the URL field.
    /// @return Set of matching packages.
    libdnf5::rpm::PackageSet get_url_matches(const std::string & pattern);

    /// @brief Update stored priority value for given packages.
    ///
    /// Each of the keys (= package metadata fields) has defined
    /// its priority. It is a value determining how this
    /// specific field is important for the user in the resulting
    /// output. The higher the priority is, the higher matching
    /// packages appear in the output.
    ///
    /// By applying various matching filters mentioned above,
    /// each package is accumulating its priority value. When
    /// the search pattern is matching more package fields or
    /// it exactly matches the metadata field, the related
    /// package gets higher priority value.
    ///
    /// @param packages Packages to be updated.
    /// @param priority Priority value to be added to the existing one.
    void update_priorities(const libdnf5::rpm::PackageSet & packages, int priority);

    libdnf5::Base & base;
    std::vector<std::string> patterns;
    bool search_all;
    libdnf5::rpm::PackageQuery full_package_query;
    std::unordered_map<std::string, int> packages_priorities;
    bool showdupes;
};

}  // namespace dnf5


#endif  // DNF5_COMMANDS_SEARCH_PROCESSOR_HPP
