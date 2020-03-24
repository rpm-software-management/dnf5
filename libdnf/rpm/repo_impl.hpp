/*
Copyright (C) 2018-2020 Red Hat, Inc.

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

#ifndef LIBDNF_RPM_REPO_REPO_PRIVATE_HPP
#define LIBDNF_RPM_REPO_REPO_PRIVATE_HPP

#include "libdnf/base/base.hpp"
#include "libdnf/rpm/repo.hpp"

#include <gpgme.h>
#include <librepo/librepo.h>
#include <solv/chksum.h>
#include <solv/repo.h>
#include <solv/util.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <cctype>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <map>
#include <mutex>
#include <set>

#define MD_TYPE_PRIMARY     "primary"
#define MD_TYPE_FILELISTS   "filelists"
#define MD_TYPE_PRESTODELTA "prestodelta"
#define MD_TYPE_GROUP_GZ    "group_gz"
#define MD_TYPE_GROUP       "group"
#define MD_TYPE_UPDATEINFO  "updateinfo"
#define MD_TYPE_MODULES     "modules"
/* "other" in this context is not a generic "any other metadata", but real metadata type named "other"
 * containing changelogs for packages */
#define MD_TYPE_OTHER "other"

#define CHKSUM_BYTES 32

enum _hy_repo_state { _HY_NEW, _HY_LOADED_FETCH, _HY_LOADED_CACHE, _HY_WRITTEN };

namespace std {

template <>
struct default_delete<LrHandle> {
    void operator()(LrHandle * ptr) noexcept { lr_handle_free(ptr); }
};

}  // namespace std

namespace libdnf::rpm {

using LibsolvRepo = ::Repo;

class Key {
public:
    Key(gpgme_key_t key, gpgme_subkey_t subkey) {
        id = subkey->keyid;
        fingerprint = subkey->fpr;
        timestamp = subkey->timestamp;
        userid = key->uids->uid;
    }

    std::string get_id() const { return id; }
    std::string get_user_id() const { return userid; }
    std::string get_fingerprint() const { return fingerprint; }
    long int get_timestamp() const { return timestamp; }

    std::vector<char> raw_key;
    std::string url;

private:
    std::string id;
    std::string fingerprint;
    std::string userid;
    long int timestamp;
};

class Repo::Impl {
public:
    Impl(Repo & owner, std::string id, Type type, std::unique_ptr<ConfigRepo> && conf, Base & base);
    ~Impl();

    bool load();
    void load_cache();
    bool try_load_cache();
    void download_metadata(const std::string & destdir);
    bool is_in_sync();
    void fetch(const std::string & destdir, std::unique_ptr<LrHandle> && h);
    std::string get_cachedir() const;
    std::string get_persistdir() const;
    int64_t get_age() const;
    void expire();
    bool is_expired() const;
    int get_expires_in() const;
    void download_url(const char * url, int fd);
    void add_countme_flag(LrHandle * handle);
    void set_http_headers(const char * headers[]);
    const char * const * get_http_headers() const;
    const std::string & get_metadata_path(const std::string & metadata_type) const;

    std::unique_ptr<LrHandle> lr_handle_init_base();
    std::unique_ptr<LrHandle> lr_handle_init_local();
    std::unique_ptr<LrHandle> lr_handle_init_remote(const char * destdir, bool mirror_setup = true);

    void attach_libsolv_repo(LibsolvRepo * libsolv_repo);
    void detach_libsolv_repo();

    LrHandle * get_cached_handle();

private:
    friend class Repo;
    std::string id;
    Type type;
    std::unique_ptr<ConfigRepo> conf;

    char ** mirrors{nullptr};
    int max_mirror_tries{0};  // try them all
    // 0 forces expiration on the next call to load(), -1 means undefined value
    int64_t timestamp;
    int max_timestamp{0};
    bool preserve_remote_time{false};
    std::string repomd_fn;
    std::set<std::string> additional_metadata;
    std::string revision;
    std::vector<std::string> content_tags;
    std::vector<std::pair<std::string, std::string>> distro_tags;
    std::vector<std::pair<std::string, std::string>> metadata_locations;
    unsigned char checksum[CHKSUM_BYTES];
    bool use_includes{false};
    bool load_metadata_other;
    std::map<std::string, std::string> substitutions;

    std::unique_ptr<RepoCB> callbacks;
    std::string repo_file_path;

    SyncStrategy sync_strategy;
    std::map<std::string, std::string> metadata_paths;

    LibsolvRepo * libsolv_repo{nullptr};
    // TODO(jrohel): Later with rpm Sack work.
    //bool needs_internalizing{false};
    int nrefs{1};

    enum _hy_repo_state state_main { _HY_NEW };
    enum _hy_repo_state state_filelists { _HY_NEW };
    enum _hy_repo_state state_presto { _HY_NEW };
    enum _hy_repo_state state_updateinfo { _HY_NEW };
    enum _hy_repo_state state_other { _HY_NEW };
    Id filenames_repodata{0};
    Id presto_repodata{0};
    Id updateinfo_repodata{0};
    Id other_repodata{0};
    int load_flags{0};
    /* the following three elements are needed for repo rewriting */
    int main_nsolvables{0};
    int main_nrepodata{0};
    int main_end{0};

    // Lock attachLibsolvRepo(), detachLibsolvRepo() and hy_repo_free() to ensure atomic behavior
    // in threaded environment such as PackageKit.
    std::mutex attach_libsolv_mutex;

    Repo * owner;
    Base * base;
    std::unique_ptr<LrResult> lr_handle_perform(
        LrHandle * handle, const std::string & dest_directory, bool set_gpg__home_dir);
    bool is_metalink_in_sync();
    bool is_repomd_in_sync();
    void reset_metadata_expired();
    std::vector<Key> retrieve(const std::string & url);
    void import_repo_keys();

    static int progress_cb(void * data, double total_to_download, double downloaded);
    static void fastest_mirror_cb(void * data, LrFastestMirrorStages stage, void * ptr);
    static int mirror_failure_cb(void * data, const char * msg, const char * url, const char * metadata);

    bool expired;
    std::unique_ptr<LrHandle> handle;
    std::unique_ptr<char * [], std::function<void(char **)>> http_headers {
        nullptr, [](char ** ptr) {
            for (auto item = ptr; *item; ++item)
                delete[] * item;
            delete[] ptr;
        }
    };
    static bool ends_with(std::string const & str, std::string const & ending);
    std::string get_hash() const;
};

}  // namespace libdnf::rpm

#endif
