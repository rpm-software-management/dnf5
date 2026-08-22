// Copyright Contributors to the DNF5 project.
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

#include "test_package_download_order.hpp"

#include <libdnf5-cli/utils/package_download_order.hpp>

#include <vector>

CPPUNIT_TEST_SUITE_REGISTRATION(PackageDownloadOrderTest);


void PackageDownloadOrderTest::setUp() {
    BaseTestCase::setUp();
    add_repo_repomd("repomd-repo-download-order");
}


void PackageDownloadOrderTest::test_sort_ascending() {
    // Fetched in NEVRA (name) order, which does not match size order:
    // alpha=900000, beta=100, gamma=50000
    std::vector<libdnf5::rpm::Package> packages{
        get_pkg("alpha-1-1.noarch"), get_pkg("beta-1-1.noarch"), get_pkg("gamma-1-1.noarch")};

    libdnf5::cli::utils::sort_packages_by_download_size(packages, false);

    CPPUNIT_ASSERT_EQUAL(std::string("beta"), packages[0].get_name());
    CPPUNIT_ASSERT_EQUAL(std::string("gamma"), packages[1].get_name());
    CPPUNIT_ASSERT_EQUAL(std::string("alpha"), packages[2].get_name());
}


void PackageDownloadOrderTest::test_sort_descending() {
    std::vector<libdnf5::rpm::Package> packages{
        get_pkg("alpha-1-1.noarch"), get_pkg("beta-1-1.noarch"), get_pkg("gamma-1-1.noarch")};

    libdnf5::cli::utils::sort_packages_by_download_size(packages, true);

    CPPUNIT_ASSERT_EQUAL(std::string("alpha"), packages[0].get_name());
    CPPUNIT_ASSERT_EQUAL(std::string("gamma"), packages[1].get_name());
    CPPUNIT_ASSERT_EQUAL(std::string("beta"), packages[2].get_name());
}


void PackageDownloadOrderTest::test_sort_empty() {
    std::vector<libdnf5::rpm::Package> packages;

    libdnf5::cli::utils::sort_packages_by_download_size(packages, false);

    CPPUNIT_ASSERT(packages.empty());
}
