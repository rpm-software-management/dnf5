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


SolvRepo::SolvRepo(const libdnf::BaseWeakPtr & base, const ConfigRepo & config, void * appdata)
    : base(base),
      config(config),
      repo(repo_create(*get_pool(base), config.get_id().c_str())) {
    repo->appdata = appdata;
}


SolvRepo::~SolvRepo() {
    repo->appdata = nullptr;
}


void SolvRepo::load_repo_main(const std::string & repomd_fn, const std::string & primary_fn) {
    auto & logger = *base->get_logger();
    auto & pool = get_pool(base);

    fs::File repomd_file(repomd_fn, "r");

    checksum_calc(checksum, repomd_file);

    int solvables_start = pool->nsolvables;

    if (load_solv_cache(nullptr, 0)) {
        main_solvables_start = solvables_start;
        main_solvables_end = pool->nsolvables;

        return;
    }

    fs::File primary_file(primary_fn, "r", true);

    logger.debug("Loading repomd and primary for repo \"{}\"", config.get_id());
    if (repo_add_repomdxml(repo, repomd_file.get(), 0) != 0) {
        throw SolvError(
            M_("Failed to load repomd for repo \"{}\" from \"{}\": {}."),
            config.get_id(),
            repomd_fn,
            pool_errstr(*pool));
    }

    if (repo_add_rpmmd(repo, primary_file.get(), 0, 0) != 0) {
        throw SolvError(
            M_("Failed to load primary for repo \"{}\" from \"{}\": {}."),
            config.get_id(),
            primary_fn,
            pool_errstr(*pool));
    }

    main_solvables_start = solvables_start;
    main_solvables_end = pool->nsolvables;

    if (config.build_cache().get_value()) {
        write_main();
    }
}


void SolvRepo::load_repo_ext(RepodataType type, const RepoDownloader & downloader) {
    auto & logger = *base->get_logger();
    auto & pool = get_pool(base);

    auto type_name = repodata_type_to_name(type);

    std::string_view ext_fn;

    if (type == RepodataType::COMPS) {
        ext_fn = downloader.get_metadata_path(RepoDownloader::MD_FILENAME_GROUP_GZ);
        if (ext_fn.empty()) {
            ext_fn = downloader.get_metadata_path(type_name);
        }
    } else {
        ext_fn = downloader.get_metadata_path(type_name);
    }

    if (ext_fn.empty()) {
        logger.debug("No {} metadata available for repo \"{}\"", type_name, config.get_id());
        return;
    }

    int solvables_start = pool->nsolvables;

    if (load_solv_cache(type_name, repodata_type_to_flags(type))) {
        if (type == RepodataType::UPDATEINFO) {
            updateinfo_solvables_start = solvables_start;
            updateinfo_solvables_end = pool->nsolvables;
        } else if (type == RepodataType::COMPS) {
            comps_solvables_start = solvables_start;
            comps_solvables_end = pool->nsolvables;
        }

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
            if ((res = repo_add_updateinfoxml(repo, ext_file.get(), 0)) == 0) {
                updateinfo_solvables_start = solvables_start;
                updateinfo_solvables_end = pool->nsolvables;
            }
            break;
        case RepodataType::COMPS:
            if ((res = repo_add_comps(repo, ext_file.get(), 0)) == 0) {
                comps_solvables_start = solvables_start;
                comps_solvables_end = pool->nsolvables;
            }
            break;
        case RepodataType::OTHER:
            res = repo_add_rpmmd(repo, ext_file.get(), 0, REPO_EXTEND_SOLVABLES);
            break;
    }

    if (res != 0) {
        throw SolvError(
            M_("Failed to load {} extension for repo \"{}\" from \"{}\": {}"),
            type_name,
            config.get_id(),
            ext_fn,
            pool_errstr(*get_pool(base)));
    }

    if (config.build_cache().get_value()) {
        write_ext(repo->nrepodata - 1, type);
    }
}


void SolvRepo::load_system_repo(const std::string & rootdir) {
    auto & logger = *base->get_logger();
    auto & pool = get_pool(base);

    logger.debug("Loading system repo rpmdb from root \"{}\"", rootdir.empty() ? "/" : rootdir);
    if (rootdir.empty()) {
        base->get_config().installroot().lock("installroot locked by loading system repo");
        pool_set_rootdir(*pool, base->get_config().installroot().get_value().c_str());
    } else {
        pool_set_rootdir(*pool, rootdir.c_str());
    }

    int solvables_start = pool->nsolvables;

    int flagsrpm = REPO_REUSE_REPODATA | RPM_ADD_WITH_HDRID | REPO_USE_ROOTDIR;
    if (repo_add_rpmdb(repo, nullptr, flagsrpm) != 0) {
        throw SolvError(
            M_("Failed to load system repo from root \"{}\": {}"),
            rootdir.empty() ? "/" : rootdir,
            pool_errstr(*get_pool(base)));
    }

    if (!rootdir.empty()) {
        // if loading an extra repo, reset rootdir back to installroot
        pool_set_rootdir(*pool, base->get_config().installroot().get_value().c_str());
    }

    pool_set_installed(*pool, repo);

    main_solvables_start = solvables_start;
    main_solvables_end = pool->nsolvables;
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

    if (!config.build_cache().get_value() || main_solvables_start == 0 || fileprovides.size() == 0) {
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

    write_main();
}


void SolvRepo::internalize() {
    if (!needs_internalizing) {
        return;
    }
    repo_internalize(repo);
    needs_internalizing = false;
}


void SolvRepo::set_priority(int priority) {
    repo->priority = priority;
}


void SolvRepo::set_subpriority(int subpriority) {
    repo->subpriority = subpriority;
}


bool SolvRepo::load_solv_cache(const char * type, int flags) {
    auto & logger = *base->get_logger();

    auto path = solv_file_path(type);

    try {
        fs::File cache_file(path, "r");

        if (can_use_repomd_cache(cache_file, checksum)) {
            logger.debug("Loading solv cache file: \"{}\"", path.native());
            if (repo_add_solv(repo, cache_file.get(), flags) != 0) {
                throw SolvError(
                    M_("Failed to load {} cache for repo \"{}\" from \"{}\": {}"),
                    type ? type : "primary",
                    config.get_id(),
                    path.native(),
                    pool_errstr(*get_pool(base)));
            }
            return true;
        } else {
            logger.trace("Cache file \"{}\" checksum mismatch, not loading", path.native());
        }
    } catch (const std::filesystem::filesystem_error & e) {
        if (e.code().default_error_condition() == std::errc::no_such_file_or_directory) {
            logger.trace("Cache file \"{}\" not found", path.native());
        } else {
            logger.warning("Error opening cache file, ignoring: {}", e.what());
        }
    }

    return false;
}


void SolvRepo::write_main() {
    auto & logger = *base->get_logger();

    const char * chksum = pool_bin2hex(*get_pool(base), checksum, solv_chksum_len(CHKSUM_TYPE));

    const auto solvfile_path = solv_file_path();

    auto cache_tmp_file = fs::TempFile(solvfile_path.parent_path(), solvfile_path.filename());

    auto & cache_file = cache_tmp_file.open_as_file("w+");

    logger.trace(
        "Writing primary cache for repo \"{}\" to \"{}\" (checksum: 0x{})",
        config.get_id(),
        cache_tmp_file.get_path().native(),
        chksum);

    Repowriter * writer = repowriter_create(repo);
    repowriter_set_solvablerange(writer, main_solvables_start, main_solvables_end);
    int res = repowriter_write(writer, cache_file.get());
    repowriter_free(writer);

    if (res != 0) {
        throw SolvError(
            M_("Failed to write primary cache for repo \"{}\" to \"{}\": {}"),
            config.get_id(),
            cache_tmp_file.get_path().native(),
            pool_errstr(*get_pool(base)));
    }

    checksum_write(checksum, cache_file);

    cache_tmp_file.close();

    std::filesystem::rename(cache_tmp_file.get_path(), solvfile_path);
    cache_tmp_file.release();
}


void SolvRepo::write_ext(Id repodata_id, RepodataType type) {
    auto & logger = *base->get_logger();
    libdnf_assert(repodata_id != 0, "0 is not a valid repodata id");

    const auto type_name = repodata_type_to_name(type);
    const auto solvfile_path = solv_file_path(type_name);

    auto cache_tmp_file = fs::TempFile(solvfile_path.parent_path(), solvfile_path.filename());
    auto & cache_file = cache_tmp_file.open_as_file("w+");

    logger.trace(
        "Writing {} extension cache for repo \"{}\" to \"{}\"",
        type_name,
        config.get_id(),
        cache_tmp_file.get_path().native());

    Repowriter * writer = repowriter_create(repo);
    repowriter_set_repodatarange(writer, repodata_id, repodata_id + 1);

    if (type == RepodataType::UPDATEINFO) {
        repowriter_set_solvablerange(writer, updateinfo_solvables_start, updateinfo_solvables_end);
    } else if (type == RepodataType::COMPS) {
        repowriter_set_solvablerange(writer, comps_solvables_start, comps_solvables_end);
    } else {
        repowriter_set_flags(writer, REPOWRITER_NO_STORAGE_SOLVABLE);
    }

    int res = repowriter_write(writer, cache_file.get());
    repowriter_free(writer);

    if (res != 0) {
        throw SolvError(
            M_("Failed to write {} cache for repo \"{}\" to \"{}\": {}"),
            type_name,
            config.get_id(),
            cache_tmp_file.get_path().native(),
            pool_errstr(*get_pool(base)));
    }

    checksum_write(checksum, cache_file);

    cache_tmp_file.close();

    std::filesystem::rename(cache_tmp_file.get_path(), solvfile_path);
    cache_tmp_file.release();
}


std::string SolvRepo::solv_file_name(const char * type) {
    if (type != nullptr) {
        return utils::sformat("{}-{}.solvx", config.get_id(), type);
    } else {
        return config.get_id() + ".solv";
    }
}


std::filesystem::path SolvRepo::solv_file_path(const char * type) {
    return std::filesystem::path(config.get_cachedir()) / solv_file_name(type);
}

}  //namespace libdnf::repo
