/*
 * Copyright (C) 2019 Red Hat, Inc.
 *
 * Licensed under the GNU Lesser General Public License Version 2.1
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef _LIBDNF_REPO_PRIVATE_HPP
#define _LIBDNF_REPO_PRIVATE_HPP

#include "Repo.hpp"
#include "../dnf-utils.h"
#include "../hy-iutil.h"
#include "../hy-util-private.hpp"
#include "../hy-types.h"

#include <utils.hpp>

#include <librepo/librepo.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <gpgme.h>

#include <solv/chksum.h>
#include <solv/repo.h>
#include <solv/util.h>

#include <cctype>
#include <map>
#include <mutex>
#include <set>

#include <string.h>
#include <time.h>

#define MD_TYPE_PRIMARY "primary"
#define MD_TYPE_FILELISTS "filelists"
#define MD_TYPE_PRESTODELTA "prestodelta"
#define MD_TYPE_GROUP_GZ "group_gz"
#define MD_TYPE_GROUP "group"
#define MD_TYPE_UPDATEINFO "updateinfo"
#define MD_TYPE_MODULES "modules"
/* "other" in this context is not a generic "any other metadata", but real metadata type named "other"
 * containing changelogs for packages */
#define MD_TYPE_OTHER "other"

enum _hy_repo_state {
    _HY_NEW,
    _HY_LOADED_FETCH,
    _HY_LOADED_CACHE,
    _HY_WRITTEN
};

namespace std {

template<>
struct default_delete<LrHandle> {
    void operator()(LrHandle * ptr) noexcept { lr_handle_free(ptr); }
};

} // namespace std

namespace libdnf {

typedef ::Repo LibsolvRepo;

class Key {
public:
    Key(gpgme_key_t key, gpgme_subkey_t subkey)
    {
        id = subkey->keyid;
        fingerprint = subkey->fpr;
        timestamp = subkey->timestamp;
        userid = key->uids->uid;
    }

    std::string getId() const { return id; }
    std::string getUserId() const { return userid; }
    std::string getFingerprint() const { return fingerprint; }
    long int getTimestamp() const { return timestamp; }

    std::vector<char> rawKey;
    std::string url;

private:
    std::string id;
    std::string fingerprint;
    std::string userid;
    long int timestamp;
};

class Repo::Impl {
public:
    Impl(Repo & owner, const std::string & id, Type type, std::unique_ptr<ConfigRepo> && conf);
    ~Impl();

    bool load();
    bool loadCache(bool throwExcept);
    void downloadMetadata(const std::string & destdir);
    bool isInSync();
    void fetch(const std::string & destdir, std::unique_ptr<LrHandle> && h);
    std::string getCachedir() const;
    std::string getPersistdir() const;
    int getAge() const;
    void expire();
    bool isExpired() const;
    int getExpiresIn() const;
    void downloadUrl(const char * url, int fd);
    void addCountmeFlag(LrHandle *handle);
    void setHttpHeaders(const char * headers[]);
    const char * const * getHttpHeaders() const;
    const std::string & getMetadataPath(const std::string &metadataType) const;

    std::unique_ptr<LrHandle> lrHandleInitBase();
    std::unique_ptr<LrHandle> lrHandleInitLocal();
    std::unique_ptr<LrHandle> lrHandleInitRemote(const char *destdir, bool mirrorSetup = true);

    void attachLibsolvRepo(LibsolvRepo * libsolvRepo);
    void detachLibsolvRepo();

    std::string id;
    Type type;
    std::unique_ptr<ConfigRepo> conf;

    char ** mirrors{nullptr};
    int maxMirrorTries{0}; // try them all
    // 0 forces expiration on the next call to load(), -1 means undefined value
    int timestamp;
    int maxTimestamp{0};
    bool preserveRemoteTime{false};
    std::string repomdFn;
    std::set<std::string> additionalMetadata;
    std::string revision;
    std::vector<std::string> content_tags;
    std::vector<std::pair<std::string, std::string>> distro_tags;
    std::vector<std::pair<std::string, std::string>> metadata_locations;
    unsigned char checksum[CHKSUM_BYTES];
    bool useIncludes{false};
    bool loadMetadataOther;
    std::map<std::string, std::string> substitutions;

    std::unique_ptr<RepoCB> callbacks;
    std::string repoFilePath;
    LrHandle * getCachedHandle();

    SyncStrategy syncStrategy;
    std::map<std::string, std::string> metadataPaths;

    LibsolvRepo * libsolvRepo{nullptr};
    bool needs_internalizing{false};
    int nrefs{1};

    enum _hy_repo_state state_main{_HY_NEW};
    enum _hy_repo_state state_filelists{_HY_NEW};
    enum _hy_repo_state state_presto{_HY_NEW};
    enum _hy_repo_state state_updateinfo{_HY_NEW};
    enum _hy_repo_state state_other{_HY_NEW};
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
    std::mutex attachLibsolvMutex;

private:
    Repo * owner;
    std::unique_ptr<LrResult> lrHandlePerform(LrHandle * handle, const std::string & destDirectory,
        bool setGPGHomeDir);
    bool isMetalinkInSync();
    bool isRepomdInSync();
    void resetMetadataExpired();
    std::vector<Key> retrieve(const std::string & url);
    void importRepoKeys();

    static int progressCB(void * data, double totalToDownload, double downloaded);
    static void fastestMirrorCB(void * data, LrFastestMirrorStages stage, void *ptr);
    static int mirrorFailureCB(void * data, const char * msg, const char * url, const char * metadata);

    bool expired;
    std::unique_ptr<LrHandle> handle;
    std::unique_ptr<char*[], std::function<void(char **)>> httpHeaders{nullptr, [](char ** ptr)
    {
        for (auto item = ptr; *item; ++item)
            delete[] *item;
        delete[] ptr;
    }};
    bool endsWith(std::string const &str, std::string const &ending) const;
    std::string getHash() const;
};

}

#endif
