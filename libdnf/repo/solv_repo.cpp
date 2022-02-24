/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2.1 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "solv_repo.hpp"

#include "repo_downloader.hpp"
#include "solv/pool.hpp"
#include "utils/bgettext/bgettext-lib.h"
#include "utils/fs/file.hpp"
#include "utils/fs/temp.hpp"

#include "libdnf/base/base.hpp"
#include "libdnf/utils/to_underlying.hpp"

extern "C" {
#include <solv/chksum.h>
#include <solv/repo_comps.h>
#include <solv/repo_deltainfoxml.h>
#include <solv/repo_repomdxml.h>
#include <solv/repo_rpmdb.h>
#include <solv/repo_rpmmd.h>
#include <solv/repo_solv.h>
#include <solv/repo_updateinfoxml.h>
#include <solv/repo_write.h>
#include <solv/solv_xfopen.h>
}


namespace libdnf::repo {

namespace fs = libdnf::utils::fs;


constexpr auto CHKSUM_TYPE = REPOKEY_TYPE_SHA256;
constexpr const char * CHKSUM_IDENT = "H000";

// Computes checksum of data in opened file.
// Calls rewind(fp) before returning.
void checksum_calc(unsigned char * out, fs::File & file) {
    // based on calc_checksum_fp in libsolv's solv.c
    char buf[4096];
    auto h = solv_chksum_create(CHKSUM_TYPE);
    int l;

    file.rewind();
    solv_chksum_add(h, CHKSUM_IDENT, strlen(CHKSUM_IDENT));
    while ((l = static_cast<int>(file.read(buf, sizeof(buf)))) > 0) {
        solv_chksum_add(h, buf, l);
    }
    file.rewind();
    solv_chksum_free(h, out);
}

// Appends checksum to the end of file.
// Moves fp to the end of file.
void checksum_write(const unsigned char * cs, fs::File & file) {
    file.seek(0, SEEK_END);
    file.write(cs, CHKSUM_BYTES);
}

// Checks checksum of data in an opened file.
// Rewinds the file before returning.
bool can_use_repomd_cache(fs::File & file, unsigned char cs_repomd[CHKSUM_BYTES]) {
    if (!file) {
        return false;
    }

    unsigned char cs_cache[CHKSUM_BYTES] = {};

    file.seek(-CHKSUM_BYTES, SEEK_END);
    file.read(cs_cache, CHKSUM_BYTES);  // short read not checked, it'd just cause checksum mismatch
    file.rewind();

    return memcmp(cs_cache, cs_repomd, CHKSUM_BYTES) == 0;
}


static const char * repodata_type_to_name(RepodataType type) {
    switch (type) {
        case RepodataType::FILELISTS:
            return RepoDownloader::MD_FILENAME_FILELISTS;
        case RepodataType::PRESTO:
            return RepoDownloader::MD_FILENAME_PRESTODELTA;
        case RepodataType::UPDATEINFO:
            return RepoDownloader::MD_FILENAME_UPDATEINFO;
        case RepodataType::COMPS:
            return RepoDownloader::MD_FILENAME_GROUP;
        case RepodataType::OTHER:
            return RepoDownloader::MD_FILENAME_OTHER;
    }

    libdnf_throw_assertion("Unknown RepodataType: {}", utils::to_underlying(type));
}


static int repodata_type_to_flags(RepodataType type) {
    switch (type) {
        case RepodataType::FILELISTS:
            return REPO_EXTEND_SOLVABLES | REPO_LOCALPOOL;
        case RepodataType::PRESTO:
            return REPO_EXTEND_SOLVABLES;
        case RepodataType::UPDATEINFO:
            return 0;
        case RepodataType::COMPS:
            return 0;
        case RepodataType::OTHER:
            return REPO_EXTEND_SOLVABLES | REPO_LOCALPOOL;
    }

    libdnf_throw_assertion("Unknown RepodataType: {}", utils::to_underlying(type));
}


SolvRepo::SolvRepo(const libdnf::BaseWeakPtr & base, const ConfigRepo & config)
    : base(base),
      config(config),
      repo(repo_create(*get_pool(base), config.get_id().c_str())) {}


void SolvRepo::load_repo_main(const std::string & repomd_fn, const std::string & primary_fn) {
    auto & logger = *base->get_logger();

    fs::File repomd_file(repomd_fn, "r");

    checksum_calc(checksum, repomd_file);

    if (!load_solv_cache(nullptr, 0)) {
        fs::File primary_file(primary_fn, "r", true);

        logger.debug("Loading repomd and primary for repo \"{}\"", config.get_id());
        if (repo_add_repomdxml(repo, repomd_file.get(), 0) || repo_add_rpmmd(repo, primary_file.get(), 0, 0)) {
            // TODO(lukash) improve error message
            throw SolvError(M_("repo_add_repomdxml/rpmmd() has failed."));
        }

        if (config.build_cache().get_value()) {
            write_main(true);
        }
    }

    main_nsolvables = repo->nsolvables;
    main_nrepodata = repo->nrepodata;
    main_end = repo->end;
}


void SolvRepo::load_repo_ext(const std::string & ext_fn, RepodataType type) {
    auto & logger = *base->get_logger();

    auto type_name = repodata_type_to_name(type);
    if (load_solv_cache(type_name, repodata_type_to_flags(type))) {
        return;
    }

    fs::File ext_file(ext_fn, "r", true);
    logger.debug("Loading {} extension for repo \"{}\" from \"{}\"", type_name, config.get_id(), ext_fn);

    int res = 0;
    switch (type) {
        case RepodataType::FILELISTS:
            res = repo_add_rpmmd(repo, ext_file.get(), "FL", REPO_EXTEND_SOLVABLES);
            break;
        case RepodataType::PRESTO:
            res = repo_add_deltainfoxml(repo, ext_file.get(), 0);
            break;
        case RepodataType::UPDATEINFO:
            res = repo_add_updateinfoxml(repo, ext_file.get(), 0);
            break;
        case RepodataType::COMPS:
            res = repo_add_comps(repo, ext_file.get(), 0);
            break;
        case RepodataType::OTHER:
            res = repo_add_rpmmd(repo, ext_file.get(), 0, REPO_EXTEND_SOLVABLES);
            break;
    }

    if (res != 0) {
        // TODO(lukash) verify pool_errstr() is the correct way to get the message here
        throw SolvError(M_("Failed loading extended metadata type \"{}\": {}"), pool_errstr(*get_pool(base)));
    }

    if (config.build_cache().get_value()) {
        write_ext(repo->nrepodata - 1, type);
    }
}


bool SolvRepo::load_system_repo(const std::string & rootdir) {
    auto & logger = *base->get_logger();
    auto & pool = get_pool(base);

    logger.debug("Loading system repo rpmdb from root \"{}\"", rootdir.empty() ? "/" : rootdir);
    if (rootdir.empty()) {
        base->get_config().installroot().lock("installroot locked by loading system repo");
        pool_set_rootdir(*pool, base->get_config().installroot().get_value().c_str());
    } else {
        pool_set_rootdir(*pool, rootdir.c_str());
    }

    int flagsrpm = REPO_REUSE_REPODATA | RPM_ADD_WITH_HDRID | REPO_USE_ROOTDIR;
    int rc = repo_add_rpmdb(repo, nullptr, flagsrpm);
    if (rc != 0) {
        logger.warning("Failed to load system repo rpmdb: {}", pool_errstr(*pool));
        return false;
    }

    if (!rootdir.empty()) {
        // if loading an extra repo, reset rootdir back to installroot
        pool_set_rootdir(*pool, base->get_config().installroot().get_value().c_str());
    }

    pool_set_installed(*pool, repo);

    // not used for system repo; set for consistency
    main_nsolvables = repo->nsolvables;
    main_nrepodata = repo->nrepodata;
    main_end = repo->end;

    return true;
}


// return true if q1 is a superset of q2
// only works if there are no duplicates both in q1 and q2
// the map parameter must point to an empty map that can hold all ids
// (it is also returned empty)
static bool is_superset(
    const libdnf::solv::IdQueue & q1, const libdnf::solv::IdQueue & q2, libdnf::solv::SolvMap & map) {
    int cnt = 0;
    for (int i = 0; i < q2.size(); i++) {
        map.add_unsafe(q2[i]);
    }
    for (int i = 0; i < q1.size(); i++) {
        if (map.contains_unsafe(q1[i])) {
            cnt++;
        }
    }
    for (int i = 0; i < q2.size(); i++) {
        map.remove_unsafe(q2[i]);
    }
    return cnt == q2.size();
}


void SolvRepo::rewrite_repo(libdnf::solv::IdQueue & fileprovides) {
    auto & logger = *base->get_logger();
    auto & pool = get_pool(base);

    logger.debug("Rewriting repo \"{}\" with added file provides", config.get_id());

    if (!config.build_cache().get_value() || main_nrepodata < 2 || fileprovides.size() == 0) {
        return;
    }

    libdnf::solv::IdQueue fileprovidesq;
    libdnf::solv::SolvMap providedids(pool->ss.nstrings);
    Repodata * data = repo_id2repodata(repo, 1);
    if (repodata_lookup_idarray(data, SOLVID_META, REPOSITORY_ADDEDFILEPROVIDES, &fileprovidesq.get_queue())) {
        if (is_superset(fileprovidesq, fileprovides, providedids)) {
            return;
        }
    }

    repodata_set_idarray(data, SOLVID_META, REPOSITORY_ADDEDFILEPROVIDES, &fileprovides.get_queue());
    repodata_internalize(data);

    // re-write main data only
    int oldnrepodata = repo->nrepodata;
    int oldnsolvables = repo->nsolvables;
    int oldend = repo->end;
    repo->nrepodata = main_nrepodata;
    repo->nsolvables = main_nsolvables;
    repo->end = main_end;

    write_main(false);

    repo->nrepodata = oldnrepodata;
    repo->nsolvables = oldnsolvables;
    repo->end = oldend;
}


bool SolvRepo::is_one_piece() const {
    for (auto i = repo->start; i < repo->end; ++i)
        if (repo->pool->solvables[i].repo != repo)
            return false;
    return true;
}


void SolvRepo::internalize() {
    if (!needs_internalizing) {
        return;
    }
    repo_internalize(repo);
    needs_internalizing = false;
}


bool SolvRepo::load_solv_cache(const char * type, int flags) {
    auto & logger = *base->get_logger();

    auto path = solv_file_path(type);

    try {
        fs::File cache_file(path, "r");

        if (can_use_repomd_cache(cache_file, checksum)) {
            logger.debug("Loading solv cache file: \"{}\"", path);
            if (repo_add_solv(repo, cache_file.get(), flags)) {
                // TODO(lukash) improve error message
                throw SolvError(M_("repo_add_solv() has failed."));
            }
            return true;
        } else {
            logger.trace("Cache file \"{}\" checksum mismatch, not loading", path);
        }
    } catch (const std::filesystem::filesystem_error & e) {
        if (e.code().default_error_condition() == std::errc::no_such_file_or_directory) {
            logger.trace("Cache file \"{}\" not found", path);
        } else {
            logger.warning("Error opening cache file, ignoring: {}", e.what());
        }
    }

    return false;
}


void SolvRepo::write_main(bool load_after_write) {
    auto & logger = *base->get_logger();

    const char * chksum = pool_bin2hex(*get_pool(base), checksum, solv_chksum_len(CHKSUM_TYPE));

    auto fn = solv_file_name();

    auto cache_tmp_file = fs::TempFile(config.basecachedir().get_value(), fn);

    auto & cache_file = cache_tmp_file.open_as_file("w+");

    logger.trace(
        "Writing primary cache for repo \"{}\" to \"{}\" (checksum: 0x{})",
        config.get_id(),
        cache_tmp_file.get_path().native(),
        chksum);

    int ret = repo_write(repo, cache_file.get());
    if (ret) {
        // TODO(lukash) improve error message
        throw SolvError(M_("Failed writing main solv cache data"));
    }

    checksum_write(checksum, cache_file);

    cache_tmp_file.close();

    if (load_after_write && is_one_piece()) {
        // switch over to written solv file activate paging
        fs::File file(cache_tmp_file.get_path(), "r");

        repo_empty(repo, 1);
        int ret = repo_add_solv(repo, file.get(), 0);
        if (ret) {
            // TODO(lukash) improve error message
            throw SolvError(M_("Failed to re-load main solv cache data file"));
        }
    }

    std::filesystem::rename(cache_tmp_file.get_path(), std::filesystem::path(config.basecachedir().get_value()) / fn);
    cache_tmp_file.release();
}


// this filter makes sure only the updateinfo repodata is written
static int write_ext_updateinfo_filter(::Repo * repo, Repokey * key, void * kfdata) {
    auto data = static_cast<Repodata *>(kfdata);
    if (key->name == 1 && static_cast<Id>(key->size) != data->repodataid) {
        return -1;
    }
    return repo_write_stdkeyfilter(repo, key, nullptr);
}


void SolvRepo::write_ext(Id repodata_id, RepodataType type) {
    auto & logger = *base->get_logger();
    libdnf_assert(repodata_id != 0, "0 is not a valid repodata id");

    Repodata * data = repo_id2repodata(repo, repodata_id);

    auto type_name = repodata_type_to_name(type);
    auto fn = solv_file_name(type_name);

    auto cache_tmp_file = fs::TempFile(base->get_config().cachedir().get_value(), fn);
    auto & cache_file = cache_tmp_file.open_as_file("w+");

    logger.trace(
        "Writing {} extension cache for repo \"{}\" to \"{}\"",
        type_name,
        config.get_id(),
        cache_tmp_file.get_path().native());
    int ret;
    if (type != RepodataType::UPDATEINFO) {
        ret = repodata_write(data, cache_file.get());
    } else {
        // block replaces: ret = write_ext_updateinfo(repo, data, cache_file.get());
        auto oldstart = repo->start;
        repo->start = main_end;
        repo->nsolvables -= main_nsolvables;
        ret = repo_write_filtered(repo, cache_file.get(), write_ext_updateinfo_filter, data, 0);
        repo->start = oldstart;
        repo->nsolvables += main_nsolvables;
    }
    if (ret) {
        // TODO(lukash) improve error message
        throw SolvError(M_("Failed writing extended solv cache data \"{}\""), fn);
    }

    checksum_write(checksum, cache_file);

    cache_tmp_file.close();

    if (is_one_piece() && type != RepodataType::UPDATEINFO) {
        // switch over to written solv file activate paging
        fs::File file(cache_tmp_file.get_path(), "r");

        repodata_extend_block(data, repo->start, repo->end - repo->start);
        data->state = REPODATA_LOADING;
        repo_add_solv(repo, file.get(), repodata_type_to_flags(type) | REPO_USE_LOADING);
        data->state = REPODATA_AVAILABLE;
    }

    std::filesystem::rename(
        cache_tmp_file.get_path(), std::filesystem::path(base->get_config().cachedir().get_value()) / fn);
    cache_tmp_file.release();
}


std::string SolvRepo::solv_file_name(const char * type) {
    if (type != nullptr) {
        return utils::sformat("{}-{}.solvx", config.get_id(), type);
    } else {
        return config.get_id() + ".solv";
    }
}


std::string SolvRepo::solv_file_path(const char * type) {
    return std::filesystem::path(config.basecachedir().get_value()) / solv_file_name(type);
}

}  //namespace libdnf::repo
