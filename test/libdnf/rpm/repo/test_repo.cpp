/*
Copyright (C) 2020-2021 Red Hat, Inc.

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
#include "libdnf/rpm/solv_sack.hpp"

#include <filesystem>
#include <fstream>

CPPUNIT_TEST_SUITE_REGISTRATION(RepoTest);


void RepoTest::setUp() {
    TestCaseFixture::setUp();
    temp = new libdnf::utils::TempDir("libdnf_unittest_", {"installroot", "cache"});
}


void RepoTest::tearDown() {
    delete temp;
    TestCaseFixture::tearDown();
}


using LoadFlags = libdnf::rpm::SolvSack::LoadRepoFlags;


void RepoTest::test_repo_basics() {
    libdnf::Base base;

    // Sets logging file.
    auto & log_router = *base.get_logger();
    log_router.add_logger(std::make_unique<libdnf::StreamLogger>(std::make_unique<std::ofstream>("repo.log")));

    // set installroot to a temp directory
    base.get_config().installroot().set(libdnf::Option::Priority::RUNTIME, temp->get_path() / "installroot");

    // set cachedir to a temp directory
    base.get_config().cachedir().set(libdnf::Option::Priority::RUNTIME, temp->get_path() / "cache");

    libdnf::rpm::RepoSack repo_sack(base);
    libdnf::rpm::SolvSack sack(base);

    // Creates system repository and loads it into rpm::SolvSack.
    // TODO(dmach): commented the next line because it loads the system repo from host; fix it to load the repo from the installroot
    // sack.create_system_repo(false);

    // Creates new repositories in the repo_sack
    auto repo = repo_sack.new_repo("repomd-repo1");

    // Tunes repository configuration (baseurl is mandatory)
    std::filesystem::path repo_path = PROJECT_BINARY_DIR "/test/libdnf/rpm/repos-repomd/repomd-repo1/";
    repo->get_config().baseurl().set(libdnf::Option::Priority::RUNTIME, "file://" + repo_path.native());

    // Loads repository into rpm::Repo.
    try {
        repo->load();
    } catch (const std::exception & ex) {
        log_router.error(ex.what());
    }

    // Loads rpm::Repo into rpm::SolvSack
    try {
        sack.load_repo(
            *repo.get(),
            LoadFlags::USE_FILELISTS | LoadFlags::USE_PRESTO | LoadFlags::USE_UPDATEINFO | LoadFlags::USE_OTHER);
    } catch (const std::exception & ex) {
        log_router.error(ex.what());
    }
}
