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

#include "test_repo.hpp"

#include "../shared/private_accessor.hpp"
#include "utils/string.hpp"

#include <libdnf5/base/base.hpp>
#include <libdnf5/repo/repo_errors.hpp>


CPPUNIT_TEST_SUITE_REGISTRATION(RepoTest);

namespace {

// Accessor of private Base::p_impl, see private_accessor.hpp
create_private_getter_template;
create_getter(load, &libdnf5::repo::Repo::load);

class DownloadCallbacks : public libdnf5::repo::DownloadCallbacks {
public:
    void * add_new_download(
        [[maybe_unused]] void * user_data,
        const char * description,
        [[maybe_unused]] double total_to_download) override {
        ++start_cnt;
        last_user_data = user_data;
        start_what = description;
        return nullptr;
    }

    int end([[maybe_unused]] void * user_cb_data, [[maybe_unused]] TransferStatus status, const char * error_message)
        override {
        ++end_cnt;
        end_error_message = libdnf5::utils::string::c_to_str(error_message);
        return 0;
    }

    int progress(
        [[maybe_unused]] void * user_cb_data,
        [[maybe_unused]] double total_to_download,
        [[maybe_unused]] double downloaded) override {
        ++progress_cnt;
        return 0;
    }

    void fastest_mirror(
        [[maybe_unused]] void * user_cb_data,
        [[maybe_unused]] FastestMirrorStage stage,
        [[maybe_unused]] const char * ptr) override {
        ++fastest_mirror_cnt;
    }

    int mirror_failure(
        [[maybe_unused]] void * user_cb_data,
        [[maybe_unused]] const char * msg,
        [[maybe_unused]] const char * url,
        [[maybe_unused]] const char * metadata) override {
        ++handle_mirror_failure_cnt;
        return 0;
    }

    void * last_user_data = nullptr;

    int start_cnt = 0;
    std::string start_what;

    int end_cnt = 0;
    std::string end_error_message;

    int progress_cnt = 0;
    int fastest_mirror_cnt = 0;
    int handle_mirror_failure_cnt = 0;
};

class RepoCallbacks : public libdnf5::repo::RepoCallbacks {
public:
    bool repokey_import([[maybe_unused]] const libdnf5::rpm::KeyInfo & key_info) override {
        ++repokey_import_cnt;
        return true;
    }

    int repokey_import_cnt = 0;
};

}  // namespace

void RepoTest::test_load_system_repo() {
    // TODO(lukash) there's no rpmdb in the installroot, create data for the test
    (*(repo_sack->get_system_repo()).*get(load{}))();
}

void RepoTest::test_load_repo() {
    std::string repoid("repomd-repo1");
    auto repo = add_repo_repomd(repoid, false);

    std::string user_data = "User data";
    repo->set_user_data(&user_data);

    auto dl_callbacks = std::make_unique<DownloadCallbacks>();
    auto dl_callbacks_ptr = dl_callbacks.get();
    base.set_download_callbacks(std::move(dl_callbacks));

    auto callbacks = std::make_unique<RepoCallbacks>();
    auto cbs = callbacks.get();
    repo->set_callbacks(std::move(callbacks));

    repo_sack->load_repos(libdnf5::repo::Repo::Type::AVAILABLE);

    CPPUNIT_ASSERT_EQUAL(&user_data, static_cast<std::string *>(dl_callbacks_ptr->last_user_data));

    CPPUNIT_ASSERT_EQUAL(1, dl_callbacks_ptr->start_cnt);
    CPPUNIT_ASSERT_EQUAL(repoid, dl_callbacks_ptr->start_what);

    CPPUNIT_ASSERT_EQUAL(1, dl_callbacks_ptr->end_cnt);
    CPPUNIT_ASSERT_EQUAL(std::string(""), dl_callbacks_ptr->end_error_message);

    CPPUNIT_ASSERT_GREATEREQUAL(1, dl_callbacks_ptr->progress_cnt);
    CPPUNIT_ASSERT_EQUAL(0, dl_callbacks_ptr->fastest_mirror_cnt);
    CPPUNIT_ASSERT_EQUAL(0, dl_callbacks_ptr->handle_mirror_failure_cnt);
    CPPUNIT_ASSERT_EQUAL(0, cbs->repokey_import_cnt);
}

void RepoTest::test_load_repo_nonexistent() {
    std::string repoid("nonexistent");
    auto repo = add_repo(repoid, "/path/thats/not/here", false);
    repo->get_config().get_skip_if_unavailable_option().set(false);

    CPPUNIT_ASSERT_THROW(repo_sack->load_repos(libdnf5::repo::Repo::Type::AVAILABLE), libdnf5::repo::RepoDownloadError);
}

void RepoTest::test_load_repos_twice_fails() {
    // Call this once...
    repo_sack->load_repos(libdnf5::repo::Repo::Type::AVAILABLE);

    // calling this again should fail
    CPPUNIT_ASSERT_THROW(repo_sack->load_repos(libdnf5::repo::Repo::Type::AVAILABLE), libdnf5::UserAssertionError);
}

void RepoTest::test_load_repos_invalid_type() {
    CPPUNIT_ASSERT_THROW(repo_sack->load_repos(libdnf5::repo::Repo::Type::COMMANDLINE), libdnf5::UserAssertionError);
}

void RepoTest::test_load_repos_load_available() {
    std::string repoid("repomd-repo1");
    auto repo = add_repo_repomd(repoid, false);

    auto dl_callbacks = std::make_unique<DownloadCallbacks>();
    auto dl_callbacks_ptr = dl_callbacks.get();
    base.set_download_callbacks(std::move(dl_callbacks));

    auto callbacks = std::make_unique<RepoCallbacks>();
    auto cbs = callbacks.get();
    repo->set_callbacks(std::move(callbacks));

    repo_sack->load_repos(libdnf5::repo::Repo::Type::AVAILABLE);

    CPPUNIT_ASSERT_EQUAL(1, dl_callbacks_ptr->start_cnt);
    CPPUNIT_ASSERT_EQUAL(repoid, dl_callbacks_ptr->start_what);

    CPPUNIT_ASSERT_EQUAL(1, dl_callbacks_ptr->end_cnt);
    CPPUNIT_ASSERT_EQUAL(std::string(""), dl_callbacks_ptr->end_error_message);

    CPPUNIT_ASSERT_GREATEREQUAL(1, dl_callbacks_ptr->progress_cnt);
    CPPUNIT_ASSERT_EQUAL(0, dl_callbacks_ptr->fastest_mirror_cnt);
    CPPUNIT_ASSERT_EQUAL(0, dl_callbacks_ptr->handle_mirror_failure_cnt);
    CPPUNIT_ASSERT_EQUAL(0, cbs->repokey_import_cnt);
}

void RepoTest::test_load_repos_load_available_system() {
    std::string repoid("repomd-repo1");
    auto repo = add_repo_repomd(repoid, false);

    auto dl_callbacks = std::make_unique<DownloadCallbacks>();
    auto dl_callbacks_ptr = dl_callbacks.get();
    base.set_download_callbacks(std::move(dl_callbacks));

    auto callbacks = std::make_unique<RepoCallbacks>();

    repo_sack->load_repos();

    CPPUNIT_ASSERT_EQUAL(1, dl_callbacks_ptr->start_cnt);
    CPPUNIT_ASSERT_EQUAL(repoid, dl_callbacks_ptr->start_what);

    CPPUNIT_ASSERT_EQUAL(1, dl_callbacks_ptr->end_cnt);
    CPPUNIT_ASSERT_EQUAL(std::string(""), dl_callbacks_ptr->end_error_message);

    CPPUNIT_ASSERT_GREATEREQUAL(2, dl_callbacks_ptr->progress_cnt);
    CPPUNIT_ASSERT_EQUAL(0, dl_callbacks_ptr->fastest_mirror_cnt);
    CPPUNIT_ASSERT_EQUAL(0, dl_callbacks_ptr->handle_mirror_failure_cnt);
}
