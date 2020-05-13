/*
Copyright (C) 2020 Red Hat, Inc.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "test_repo.hpp"

#include "libdnf/base/base.hpp"
#include "libdnf/logger/stream_logger.hpp"
#include "libdnf/rpm/repo_sack.hpp"
#include "libdnf/rpm/sack.hpp"

#include <filesystem>
#include <fstream>

CPPUNIT_TEST_SUITE_REGISTRATION(RepoTest);


void RepoTest::setUp() {}


void RepoTest::tearDown() {}

using LoadFlags = libdnf::rpm::Sack::LoadRepoFlags;

void RepoTest::test_repo_basics() {
    libdnf::Base base;

    // Sets logging file.
    auto & log_router = base.get_logger();
    log_router.add_logger(std::make_unique<libdnf::StreamLogger>(std::make_unique<std::ofstream>("repo.log")));

    // Sets path to cache directory.
    auto cwd = std::filesystem::current_path();
    base.get_config().cachedir().set(libdnf::Option::Priority::RUNTIME, cwd.native());

    libdnf::rpm::RepoSack repo_sack(base);
    libdnf::rpm::Sack sack(base);

    // Creates new repositories in the repo_sack
    auto repo = repo_sack.new_repo("dnf-ci-fedora");

    // Tunes repositotory configuration (baseurl is mandatory)
    auto repo_path = cwd / "../../../test/libdnf/rpm/repos-data/dnf-ci-fedora/";
    auto baseurl = "file://" + repo_path.native();
    auto repo_cfg = repo->get_config();
    repo_cfg->baseurl().set(libdnf::Option::Priority::RUNTIME, baseurl);

    // Loads repository into rpm::Repo.
    try {
        repo->load();
    } catch (const std::exception & ex) {
        log_router.error(ex.what());
    }

    // Loads rpm::Repo into rpm::Sack
    try {
        sack.load_repo(
            *repo.get(),
            true,
            LoadFlags::USE_FILELISTS | LoadFlags::USE_PRESTO | LoadFlags::USE_UPDATEINFO | LoadFlags::USE_OTHER);
    } catch (const std::exception & ex) {
        log_router.error(ex.what());
    }
}
