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


#include "libdnf5-cli/output/changelogs.hpp"

#include "libdnf5-cli/tty.hpp"

#include <libdnf5/rpm/package_set.hpp>

#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>


namespace libdnf5::cli::output {

void print_changelogs(
    libdnf5::rpm::PackageQuery & query,
    std::pair<ChangelogFilterType, std::variant<libdnf5::rpm::PackageQuery, int64_t, int32_t>> filter) {
    // by_srpm
    std::map<std::string, std::vector<libdnf5::rpm::Package>> by_srpm;
    for (auto pkg : query) {
        by_srpm[pkg.get_source_name() + '/' + pkg.get_evr()].push_back(pkg);
    }
    if (filter.first == ChangelogFilterType::UPGRADES) {
        std::get<libdnf5::rpm::PackageQuery>(filter.second).filter_installed();
    }

    for (auto & [name, packages] : by_srpm) {
        // Print header
        std::cout << "Changelogs for ";
        std::vector<std::string> nevras;
        nevras.reserve(packages.size());
        for (auto & pkg : packages) {
            nevras.push_back(pkg.get_nevra());
        }
        std::sort(nevras.begin(), nevras.end());
        std::cout << nevras[0];
        for (size_t idx = 1; idx < nevras.size(); ++idx) {
            std::cout << ", " << nevras[idx];
        }
        std::cout << std::endl;

        auto changelogs = packages[0].get_changelogs();
        std::sort(
            changelogs.begin(),
            changelogs.end(),
            [](const libdnf5::rpm::Changelog & a, const libdnf5::rpm::Changelog & b) {
                return a.get_timestamp() > b.get_timestamp();
            });

        // filter changelog
        if (filter.first == ChangelogFilterType::UPGRADES) {
            auto & installed = std::get<libdnf5::rpm::PackageQuery>(filter.second);
            // Find the newest changelog on the installed version of the
            // package, and filter out any changelogs newer than that

            libdnf5::rpm::PackageQuery query(installed);
            query.filter_name({packages[0].get_name()});
            time_t newest_timestamp = 0;
            for (auto pkg : query) {
                for (auto & chlog : pkg.get_changelogs()) {
                    if (chlog.get_timestamp() > newest_timestamp) {
                        newest_timestamp = chlog.get_timestamp();
                    }
                }
            }
            size_t idx;
            for (idx = 0; idx < changelogs.size() && changelogs[idx].get_timestamp() > newest_timestamp; ++idx) {
            }
            changelogs.erase(changelogs.begin() + static_cast<int>(idx), changelogs.end());
        } else if (filter.first == ChangelogFilterType::COUNT) {
            int32_t count = std::get<int32_t>(filter.second);
            if (count > 0) {
                if (static_cast<size_t>(count) < changelogs.size()) {
                    changelogs.erase(changelogs.begin() + count, changelogs.end());
                }
            } else {
                if (static_cast<size_t>(-count) < changelogs.size()) {
                    changelogs.erase(changelogs.end() + count, changelogs.end());
                } else {
                    changelogs.clear();
                }
            }
        } else if (filter.first == ChangelogFilterType::SINCE) {
            int64_t since = std::get<int64_t>(filter.second);
            size_t idx;
            for (idx = 0; idx < changelogs.size() && changelogs[idx].get_timestamp() >= since; ++idx) {
            }
            changelogs.erase(changelogs.begin() + static_cast<int>(idx), changelogs.end());
        }

        for (auto & chlog : changelogs) {
            std::cout << std::put_time(std::gmtime(&chlog.get_timestamp()), "* %a %b %d %X %Y ");
            std::cout << chlog.get_author() << "\n";
            std::cout << chlog.get_text() << "\n" << std::endl;
        }
    }
}

}  // namespace libdnf5::cli::output
