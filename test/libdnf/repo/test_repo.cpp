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

#include "test_repo.hpp"

#include "utils/string.hpp"

#include "libdnf/base/base.hpp"

#include <filesystem>


CPPUNIT_TEST_SUITE_REGISTRATION(RepoTest);


void RepoTest::test_load_system_repo() {
    // TODO(lukash) there's no rpmdb in the installroot, create data for the test
    repo_sack->get_system_repo()->load();
}


class RepoCallbacks : public libdnf::repo::RepoCallbacks {
public:
    void start(const char * what) override {
        ++start_cnt;
        start_what = what;
    }

    void end(const char * error_message) override {
        ++end_cnt;
        end_error_message = libdnf::utils::string::c_to_str(error_message);
    }

    int progress([[maybe_unused]] double total_to_download, [[maybe_unused]] double downloaded) override {
        ++progress_cnt;
        return 0;
    }

    void fastest_mirror([[maybe_unused]] FastestMirrorStage stage, [[maybe_unused]] const char * ptr) override {
        ++fastest_mirror_cnt;
    }

    int handle_mirror_failure(
        [[maybe_unused]] const char * msg,
        [[maybe_unused]] const char * url,
        [[maybe_unused]] const char * metadata) override {
        ++handle_mirror_failure_cnt;
        return 0;
    }

    bool repokey_import(
        [[maybe_unused]] const std::string & id,
        [[maybe_unused]] const std::vector<std::string> & user_ids,
        [[maybe_unused]] const std::string & fingerprint,
        [[maybe_unused]] const std::string & url,
        [[maybe_unused]] long int timestamp) override {
        ++repokey_import_cnt;
        return true;
    }

    int start_cnt = 0;
    std::string start_what;

    int end_cnt = 0;
    std::string end_error_message;

    int progress_cnt = 0;
    int fastest_mirror_cnt = 0;
    int handle_mirror_failure_cnt = 0;
    int repokey_import_cnt = 0;
};

void RepoTest::test_load_repo() {
    std::string repoid("repomd-repo1");
    auto repo = add_repo_repomd(repoid, false);

    auto callbacks = std::make_unique<RepoCallbacks>();
    auto cbs = callbacks.get();
    repo->set_callbacks(std::move(callbacks));

    libdnf::repo::RepoQuery repos(base);
    repos.filter_id(repoid);
    repo_sack->update_and_load_repos(repos);

    CPPUNIT_ASSERT_EQUAL(1, cbs->start_cnt);
    CPPUNIT_ASSERT_EQUAL(repoid, cbs->start_what);

    CPPUNIT_ASSERT_EQUAL(1, cbs->end_cnt);
    CPPUNIT_ASSERT_EQUAL(std::string(""), cbs->end_error_message);

    CPPUNIT_ASSERT_GREATEREQUAL(1, cbs->progress_cnt);
    CPPUNIT_ASSERT_EQUAL(0, cbs->fastest_mirror_cnt);
    CPPUNIT_ASSERT_EQUAL(0, cbs->handle_mirror_failure_cnt);
    CPPUNIT_ASSERT_EQUAL(0, cbs->repokey_import_cnt);
}

void RepoTest::test_load_repo_nonexistent() {
    std::string repoid("nonexistent");
    auto repo = add_repo(repoid, "/path/thats/not/here", false);
    repo->get_config().get_skip_if_unavailable_option().set(false);

    libdnf::repo::RepoQuery repos(base);
    repos.filter_id(repoid);
    CPPUNIT_ASSERT_THROW(repo_sack->update_and_load_repos(repos), libdnf::repo::RepoDownloadError);
}
