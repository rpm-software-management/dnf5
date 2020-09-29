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


#include "repo_fixture.hpp"

#include <filesystem>


void RepoFixture::add_repo(const std::string & name) {
    // Creates new repository in the repo_sack
    auto repo = repo_sack->new_repo(name);

    // Tunes repository configuration (baseurl is mandatory)
    std::filesystem::path repo_path = PROJECT_SOURCE_DIR "/test/libdnf/rpm/repos-data/";
    repo_path /= name;

    auto baseurl = "file://" + repo_path.native();
    auto repo_cfg = repo->get_config();
    repo_cfg->baseurl().set(libdnf::Option::Priority::RUNTIME, baseurl);

    // Loads repository into rpm::Repo.
    repo->load();

    // Loads rpm::Repo into rpm::SolvSack
    sack->load_repo(*repo.get(),
        libdnf::rpm::SolvSack::LoadRepoFlags::USE_FILELISTS |
        libdnf::rpm::SolvSack::LoadRepoFlags::USE_OTHER |
        libdnf::rpm::SolvSack::LoadRepoFlags::USE_PRESTO |
        libdnf::rpm::SolvSack::LoadRepoFlags::USE_UPDATEINFO
    );
}

void RepoFixture::setUp() {
    temp = std::make_unique<libdnf::utils::TempDir>(
        "libdnf_unittest_",
        std::vector<std::string>{"installroot", "cache"}
    );
    base = std::make_unique<libdnf::Base>();

    // set installroot to a temp directory
    base->get_config().installroot().set(libdnf::Option::Priority::RUNTIME, temp->get_path() / "installroot");

    // set cachedir to a temp directory
    base->get_config().cachedir().set(libdnf::Option::Priority::RUNTIME, temp->get_path() / "cache");

    repo_sack = &(base->get_rpm_repo_sack());
    sack = &(base->get_rpm_solv_sack());
}

void RepoFixture::dump_debugdata() {
    sack->dump_debugdata("debugdata");
}
