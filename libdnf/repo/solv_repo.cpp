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
#include "utils/temp.hpp"

#include "libdnf/base/base.hpp"

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


// Deleter for std::unique_ptr<FILE>
void close_file(std::FILE * fp) {
    std::fclose(fp);
}


constexpr auto CHKSUM_TYPE = REPOKEY_TYPE_SHA256;
constexpr const char * CHKSUM_IDENT = "H000";

// Computes checksum of data in opened file.
// Calls rewind(fp) before returning.
void checksum_fp(unsigned char * out, FILE * fp) {
    // based on calc_checksum_fp in libsolv's solv.c
    char buf[4096];
    auto h = solv_chksum_create(CHKSUM_TYPE);
    int l;

    rewind(fp);
    solv_chksum_add(h, CHKSUM_IDENT, strlen(CHKSUM_IDENT));
    while ((l = static_cast<int>(fread(buf, 1, sizeof(buf), fp))) > 0) {
        solv_chksum_add(h, buf, l);
    }
    rewind(fp);
    solv_chksum_free(h, out);
}

// Appends checksum to the end of file.
// Moves fp to the end of file.
int checksum_write(const unsigned char * cs, FILE * fp) {
    if ((fseek(fp, 0, SEEK_END) != 0) || fwrite(cs, CHKSUM_BYTES, 1, fp) != 1) {
        return 1;
    }
    return 0;
}

// Reads checksum of data in opened file.
// Calls rewind(fp) before returning.
bool checksum_read(unsigned char * csout, FILE * fp) {
    if ((fseek(fp, -CHKSUM_BYTES, SEEK_END) != 0) || fread(csout, CHKSUM_BYTES, 1, fp) != 1) {
        return false;
    }
    rewind(fp);
    return true;
}

bool can_use_repomd_cache(FILE * fp_solv, unsigned char cs_repomd[CHKSUM_BYTES]) {
    unsigned char cs_cache[CHKSUM_BYTES];
    return (fp_solv != nullptr) && checksum_read(cs_cache, fp_solv) && memcmp(cs_cache, cs_repomd, CHKSUM_BYTES) == 0;
}


namespace libdnf::repo {

static const char * repodata_type_to_suffix(RepodataType type) {
    switch (type) {
        case RepodataType::FILELISTS:
            return "-filelists";
        case RepodataType::PRESTO:
            return "-presto";
        case RepodataType::UPDATEINFO:
            return "-updateinfo";
        case RepodataType::COMPS:
            return "-comps";
        case RepodataType::OTHER:
            return "-other";
    }

    libdnf_throw_assertion("Unknown RepodataType: {}", type);
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

    libdnf_throw_assertion("Unknown RepodataType: {}", type);
}

SolvRepo::SolvRepo(const libdnf::BaseWeakPtr & base, const ConfigRepo & config)
    : base(base),
      config(config),
      repo(repo_create(*get_pool(base), config.get_id().c_str())) {}


void SolvRepo::write_main(bool load_after_write) {
    auto & logger = *base->get_logger();

    const char * chksum = pool_bin2hex(*get_pool(base), checksum, solv_chksum_len(CHKSUM_TYPE));

    auto fn = solv_file_name();

    auto tmp_file = libdnf::utils::TempFile(config.basecachedir().get_value(), fn);

    auto fp = tmp_file.fdopen("w+");

    logger.debug(fmt::format("caching libsolv_repo: {} (0x{})", repo->name, chksum));

    int ret = repo_write(repo, fp);
    ret |= checksum_write(checksum, fp);
    if (ret) {
        // TODO(lukash) improve error message
        throw SolvError(M_("Failed writing main solv cache data"));
    }

    tmp_file.close();

    if (load_after_write && is_one_piece()) {
        /* switch over to written solv file activate paging */
        std::unique_ptr<std::FILE, decltype(&close_file)> fp(fopen(tmp_file.get_path().c_str(), "r"), &close_file);
        if (fp) {
            repo_empty(repo, 1);
            int ret = repo_add_solv(repo, fp.get(), 0);
            if (ret) {
                // TODO(lukash) improve error message
                throw SolvError(M_("Failed to re-load main solv cache data file"));
            }
        }
    }

    std::filesystem::rename(tmp_file.get_path(), std::filesystem::path(config.basecachedir().get_value()) / fn);
    tmp_file.release();
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
    auto fn = solv_file_name(repodata_type_to_suffix(type));

    auto tmp_file = libdnf::utils::TempFile(base->get_config().cachedir().get_value(), fn);

    auto fp = tmp_file.fdopen("w+");

    logger.debug(fmt::format("{}: storing {} to: {}", __func__, repo->name, tmp_file.get_path().native()));
    int ret;
    if (type != RepodataType::UPDATEINFO) {
        ret = repodata_write(data, fp);
    } else {
        // block replaces: ret = write_ext_updateinfo(hrepo, data, fp);
        auto oldstart = repo->start;
        repo->start = main_end;
        repo->nsolvables -= main_nsolvables;
        ret = repo_write_filtered(repo, fp, write_ext_updateinfo_filter, data, 0);
        repo->start = oldstart;
        repo->nsolvables += main_nsolvables;
    }
    ret |= checksum_write(checksum, fp);
    if (ret) {
        // TODO(lukash) improve error message
        throw SolvError(M_("Failed writing extended solv cache data \"{}\""), fn);
    }

    tmp_file.close();

    if (is_one_piece() && type != RepodataType::UPDATEINFO) {
        // switch over to written solv file activate paging
        std::unique_ptr<std::FILE, decltype(&close_file)> fp(fopen(tmp_file.get_path().c_str(), "r"), &close_file);
        if (fp) {
            repodata_extend_block(data, repo->start, repo->end - repo->start);
            data->state = REPODATA_LOADING;
            repo_add_solv(repo, fp.get(), repodata_type_to_flags(type) | REPO_USE_LOADING);
            data->state = REPODATA_AVAILABLE;
        }
    }

    std::filesystem::rename(tmp_file.get_path(), std::filesystem::path(base->get_config().cachedir().get_value()) / fn);
    tmp_file.release();
}

void SolvRepo::load_repo_main(const std::string & repomd_fn, const std::string & primary_fn) {
    auto & logger = *base->get_logger();

    std::unique_ptr<std::FILE, decltype(&close_file)> fp_repomd(fopen(repomd_fn.c_str(), "rb"), &close_file);
    if (!fp_repomd) {
        throw SystemError(errno, repomd_fn);
    }
    checksum_fp(checksum, fp_repomd.get());
    std::unique_ptr<std::FILE, decltype(&close_file)> fp_cache(fopen(solv_file_path().c_str(), "rb"), &close_file);
    if (can_use_repomd_cache(fp_cache.get(), checksum)) {
        //const char *chksum = pool_checksum_str(pool, repoImpl->checksum);
        //logger.debug("using cached %s (0x%s)", name, chksum);
        if (repo_add_solv(repo, fp_cache.get(), 0)) {
            // TODO(lukash) improve error message
            throw SolvError(M_("repo_add_solv() has failed."));
        }
    } else {
        std::unique_ptr<std::FILE, decltype(&close_file)> fp_primary(solv_xfopen(primary_fn.c_str(), "r"), &close_file);

        if (!fp_primary) {
            // TODO(lukash) proper exception
            throw RuntimeError(fmt::format("Failed to open repo primary \"{}\": {}", primary_fn, errno));
        }

        logger.debug(std::string("fetching ") + config.get_id());
        if (repo_add_repomdxml(repo, fp_repomd.get(), 0) || repo_add_rpmmd(repo, fp_primary.get(), 0, 0)) {
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

void SolvRepo::load_repo_ext(const std::string & filename, RepodataType type) {
    auto & logger = *base->get_logger();

    auto fn_cache = solv_file_path(repodata_type_to_suffix(type));

    std::unique_ptr<std::FILE, decltype(&close_file)> fp(fopen(fn_cache.c_str(), "rb"), &close_file);
    if (can_use_repomd_cache(fp.get(), checksum)) {
        logger.debug(fmt::format("{}: using cache file: {}", __func__, fn_cache.c_str()));
        if (repo_add_solv(repo, fp.get(), repodata_type_to_flags(type)) != 0) {
            // TODO(lukash) improve error message
            throw SolvError(M_("repo_add_solv() has failed."));
        }

        return;
    }

    fp.reset(solv_xfopen(filename.c_str(), "r"));
    if (!fp) {
        // TODO(lukash) improve error message
        throw SolvError(M_("failed to open: {}"), filename);
    }
    logger.debug(fmt::format("{}: loading: {}", __func__, filename));

    int res = 0;
    switch (type) {
        case RepodataType::FILELISTS:
            res = repo_add_rpmmd(repo, fp.get(), "FL", REPO_EXTEND_SOLVABLES);
            break;
        case RepodataType::PRESTO:
            res = repo_add_deltainfoxml(repo, fp.get(), 0);
            break;
        case RepodataType::UPDATEINFO:
            res = repo_add_updateinfoxml(repo, fp.get(), 0);
            break;
        case RepodataType::COMPS:
            res = repo_add_comps(repo, fp.get(), 0);
            break;
        case RepodataType::OTHER:
            res = repo_add_rpmmd(repo, fp.get(), 0, REPO_EXTEND_SOLVABLES);
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

    logger.debug("Loading system repo rpmdb, root: \"" + rootdir + "\"");
    if (rootdir.empty()) {
        base->get_config().installroot().lock("installroot locked by loading system repo");
        pool_set_rootdir(*pool, base->get_config().installroot().get_value().c_str());
    } else {
        pool_set_rootdir(*pool, rootdir.c_str());
    }

    int flagsrpm = REPO_REUSE_REPODATA | RPM_ADD_WITH_HDRID | REPO_USE_ROOTDIR;
    int rc = repo_add_rpmdb(repo, nullptr, flagsrpm);
    if (rc != 0) {
        logger.warning(fmt::format("load_system_repo(): failed loading RPMDB: {}", pool_errstr(*pool)));
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

    logger.debug(fmt::format("rewriting repo: {}", config.get_id()));
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

std::string SolvRepo::solv_file_name(const char * type) {
    if (type != nullptr) {
        return config.get_id() + type + ".solvx";
    } else {
        return config.get_id() + ".solv";
    }
}

std::string SolvRepo::solv_file_path(const char * type) {
    return std::filesystem::path(config.basecachedir().get_value()) / solv_file_name(type);
}

}  //namespace libdnf::repo
