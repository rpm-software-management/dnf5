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


#include "test_versionlock_config.hpp"

#include <libdnf5/common/exception.hpp>
#include <libdnf5/conf/const.hpp>
#include <libdnf5/rpm/versionlock_config.hpp>
#include <libdnf5/utils/fs/file.hpp>
#include <unistd.h>

#include <cstddef>


CPPUNIT_TEST_SUITE_REGISTRATION(RpmVersionlockConfigTest);


void RpmVersionlockConfigTest::setUp() {
    BaseTestCase::setUp();

    // The package sack builds the versionlock path by prepending the installroot
    // (set to a temporary directory by BaseTestCase) to VERSIONLOCK_CONF_FILENAME.
    const std::filesystem::path installroot = base.get_config().get_installroot_option().get_value();
    versionlock_path = installroot / std::filesystem::path(libdnf5::VERSIONLOCK_CONF_FILENAME).relative_path();
}


void RpmVersionlockConfigTest::write_versionlock_config(const std::string & content) {
    std::filesystem::create_directories(versionlock_path.parent_path());
    libdnf5::utils::fs::File(versionlock_path, "w").write(content);
}


void RpmVersionlockConfigTest::test_load_missing_file() {
    // A missing versionlock file is not an error, it just means nothing is locked.
    auto config = sack->get_versionlock_config();
    CPPUNIT_ASSERT(config.get_packages().empty());
}


void RpmVersionlockConfigTest::test_load_valid_file() {
    write_versionlock_config(
        "version = \"1.0\"\n"
        "[[packages]]\n"
        "name = \"wget\"\n"
        "[[packages.conditions]]\n"
        "key = \"evr\"\n"
        "comparator = \"=\"\n"
        "value = \"0:1.19.5-5.fc29\"\n");

    auto config = sack->get_versionlock_config();
    auto & packages = config.get_packages();
    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(1), packages.size());
    CPPUNIT_ASSERT_EQUAL(std::string("wget"), packages[0].get_name());
}


void RpmVersionlockConfigTest::test_load_malformed_file() {
    // Malformed TOML must result in a clean libdnf5 error instead of an uncaught
    // toml11 exception that aborts the process.
    write_versionlock_config("version = \"1.0\"\n[unterminated\n");

    CPPUNIT_ASSERT_THROW(sack->get_versionlock_config(), libdnf5::Error);
}


void RpmVersionlockConfigTest::test_load_unreadable_file() {
    // The root user bypasses file permission checks, so an unreadable file can
    // only be simulated when running as an unprivileged user.
    if (geteuid() == 0) {
        return;
    }

    write_versionlock_config("version = \"1.0\"\n");
    std::filesystem::permissions(versionlock_path, std::filesystem::perms::none);

    CPPUNIT_ASSERT_THROW(sack->get_versionlock_config(), libdnf5::FileSystemError);
}
