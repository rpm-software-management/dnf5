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


#include "../libdnf/utils/bgettext/bgettext-lib.h"
#include "repo_impl.hpp"
#include "solv/id_queue.hpp"
#include "solv_sack_impl.hpp"

#include "libdnf/rpm/repo.hpp"

extern "C" {
#include <solv/chksum.h>
#include <solv/repo.h>
#include <solv/repo_deltainfoxml.h>
#include <solv/repo_repomdxml.h>
#include <solv/repo_rpmdb.h>
#include <solv/repo_rpmmd.h>
#include <solv/repo_solv.h>
#include <solv/repo_updateinfoxml.h>
#include <solv/repo_write.h>
#include <solv/solv_xfopen.h>
#include <solv/solver.h>
#include <solv/testcase.h>
}

#include <fmt/format.h>

#include <filesystem>


using LibsolvRepo = Repo;

namespace libdnf::rpm {

namespace {
// Names of special repositories
constexpr const char * SYSTEM_REPO_NAME = "@System";
constexpr const char * CMDLINE_REPO_NAME = "@commandline";
// TODO lukash: unused, remove?
//constexpr const char * MODULE_FAIL_SAFE_REPO_NAME = "@modulefailsafe";

// Extensions of solv file names
constexpr const char * SOLV_EXT_FILENAMES = "-filenames";
constexpr const char * SOLV_EXT_UPDATEINFO = "-updateinfo";
constexpr const char * SOLV_EXT_PRESTO = "-presto";
constexpr const char * SOLV_EXT_OTHER = "-other";

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

// Deleter for std::unique_ptr<FILE>
void close_file(std::FILE * fp) {
    std::fclose(fp);
}

// Deleter for std::unique_ptr<LibsolvRepo>
static void libsolv_repo_free(LibsolvRepo * libsolv_repo) {
    repo_free(libsolv_repo, 1);
}

// return true if q1 is a superset of q2
// only works if there are no duplicates both in q1 and q2
// the map parameter must point to an empty map that can hold all ids
// (it is also returned empty)
bool is_superset(const solv::IdQueue & q1, const solv::IdQueue * q2, solv::SolvMap & map) {
    int cnt = 0;
    for (int i = 0; i < q2->size(); i++) {
        map.add_unsafe(PackageId((*q2)[i]));
    }
    for (int i = 0; i < q1.size(); i++) {
        if (map.contains_unsafe(PackageId(q1[i]))) {
            cnt++;
        }
    }
    for (int i = 0; i < q2->size(); i++) {
        map.remove_unsafe(PackageId((*q2)[i]));
    }
    return cnt == q2->size();
}

}  // end of anonymous namespace

void SolvSack::Impl::write_main(LibsolvRepoExt & libsolv_repo_ext, bool switchtosolv) {
    auto & logger = base->get_logger();
    LibsolvRepo * libsolv_repo = libsolv_repo_ext.repo;
    const char * name = libsolv_repo->name;
    const char * chksum = pool_bin2hex(pool, libsolv_repo_ext.checksum, solv_chksum_len(CHKSUM_TYPE));
    auto fn = give_repo_solv_cache_fn(name, NULL);
    auto tmp_fn_templ = fn + ".XXXXXX";
    int tmp_fd = mkstemp(tmp_fn_templ.data());

    logger.debug(fmt::format("caching libsolv_repo: {} (0x{})", name, chksum));

    if (tmp_fd == -1) {
        throw SystemError(errno, fmt::format(_("cannot create temporary file: {}"), tmp_fn_templ));
        // g_set_error (error, DNF_ERROR, DNF_ERROR_FILE_INVALID, _("cannot create temporary file: %s"), tmp_fn_templ);
    }

    auto fp = fdopen(tmp_fd, "w+");
    if (!fp) {
        auto tmp_err = errno;
        close(tmp_fd);
        unlink(tmp_fn_templ.c_str());
        throw SystemError(tmp_err, _("failed opening tmp file"));
        // g_set_error (error, DNF_ERROR, DNF_ERROR_FILE_INVALID, _("failed opening tmp file: %s"), strerror(errno));
    }
    int ret = repo_write(libsolv_repo, fp);
    ret |= checksum_write(libsolv_repo_ext.checksum, fp);
    ret |= fclose(fp);
    if (ret) {
        unlink(tmp_fn_templ.c_str());
        throw Exception(_("write_main() failed writing data"));
        // g_set_error (error, DNF_ERROR, DNF_ERROR_FILE_INVALID, _("write_main() failed writing data: %i"), rc);
    }
    if (switchtosolv && libsolv_repo_ext.is_one_piece()) {
        /* switch over to written solv file activate paging */
        std::unique_ptr<std::FILE, decltype(&close_file)> fp(fopen(tmp_fn_templ.c_str(), "r"), &close_file);
        if (fp) {
            repo_empty(libsolv_repo, 1);
            int ret = repo_add_solv(libsolv_repo, fp.get(), 0);
            if (ret) {
                /* this is pretty fatal */
                throw Exception(_("write_main() failed to re-load written solv file"));
                // g_set_error_literal (error, DNF_ERROR, DNF_ERROR_FILE_INVALID, _("write_main() failed to re-load " "written solv file"));
            }
        }
    }

    try {
        std::filesystem::rename(tmp_fn_templ, fn);
    } catch (...) {
        unlink(tmp_fn_templ.c_str());
        throw;
    }
}

// this filter makes sure only the updateinfo repodata is written
static int write_ext_updateinfo_filter(LibsolvRepo * repo, Repokey * key, void * kfdata) {
    auto data = static_cast<Repodata *>(kfdata);
    if (key->name == 1 && static_cast<Id>(key->size) != data->repodataid) {
        return -1;
    }
    return repo_write_stdkeyfilter(repo, key, nullptr);
}

void SolvSack::Impl::write_ext(
    LibsolvRepoExt & libsolv_repo_ext, Id repodata_id, RepodataType which_repodata, const char * suffix) {
    auto & logger = base->get_logger();
    auto libsolv_repo = libsolv_repo_ext.repo;
    const char * repo_id = libsolv_repo->name;

    assert(repodata_id);
    Repodata * data = repo_id2repodata(libsolv_repo, repodata_id);
    auto fn = give_repo_solv_cache_fn(repo_id, suffix);
    auto tmp_fn_templ = fn + ".XXXXXX";
    int tmp_fd = mkstemp(tmp_fn_templ.data());

    if (tmp_fd == -1) {
        throw SystemError(errno, fmt::format(_("cannot create temporary file: {}"), tmp_fn_templ));
        // g_set_error (error, DNF_ERROR, DNF_ERROR_FILE_INVALID, _("can not create temporary file %s"), tmp_fn_templ);
    }

    auto fp = fdopen(tmp_fd, "w+");

    logger.debug(fmt::format("{}: storing {} to: {}", __func__, repo_id, tmp_fn_templ));
    int ret;
    if (which_repodata != RepodataType::UPDATEINFO)
        ret = repodata_write(data, fp);
    else {
        // block replaces: ret = write_ext_updateinfo(hrepo, data, fp);
        auto oldstart = libsolv_repo->start;
        libsolv_repo->start = libsolv_repo_ext.main_end;
        libsolv_repo->nsolvables -= libsolv_repo_ext.main_nsolvables;
        ret = repo_write_filtered(libsolv_repo, fp, write_ext_updateinfo_filter, data, 0);
        libsolv_repo->start = oldstart;
        libsolv_repo->nsolvables += libsolv_repo_ext.main_nsolvables;
    }
    ret |= checksum_write(libsolv_repo_ext.checksum, fp);
    ret |= fclose(fp);
    if (ret) {
        unlink(tmp_fn_templ.c_str());
        throw Exception(_("write_ext() has failed"));
        // g_set_error(error, DNF_ERROR, DNF_ERROR_FAILED, _("write_ext(%1$d) has failed: %2$d"), which_repodata, ret);
    }
    if (libsolv_repo_ext.is_one_piece() && which_repodata != RepodataType::UPDATEINFO) {
        // switch over to written solv file activate paging
        std::unique_ptr<std::FILE, decltype(&close_file)> fp(fopen(tmp_fn_templ.c_str(), "r"), &close_file);
        if (fp) {
            int flags = REPO_USE_LOADING | REPO_EXTEND_SOLVABLES;
            // do not pollute the main pool with directory component ids
            if (which_repodata == RepodataType::FILENAMES || which_repodata == RepodataType::OTHER)
                flags |= REPO_LOCALPOOL;
            repodata_extend_block(data, libsolv_repo->start, libsolv_repo->end - libsolv_repo->start);
            data->state = REPODATA_LOADING;
            repo_add_solv(libsolv_repo, fp.get(), flags);
            data->state = REPODATA_AVAILABLE;
        }
    }

    try {
        std::filesystem::rename(tmp_fn_templ, fn);
    } catch (...) {
        unlink(tmp_fn_templ.c_str());
        throw;
    }
}

void SolvSack::Impl::rewrite_repos(solv::IdQueue & addedfileprovides, solv::IdQueue & addedfileprovides_inst) {
    int i;
    auto & logger = base->get_logger();

    solv::SolvMap providedids(pool->ss.nstrings);

    solv::IdQueue fileprovidesq;

    LibsolvRepo * libsolv_repo;
    FOR_REPOS(i, libsolv_repo) {
        auto repo = static_cast<Repo *>(libsolv_repo->appdata);
        if (!repo) {
            continue;
        }
        if (!(repo->get_config().build_cache().get_value())) {
            continue;
        }
        auto & libsolv_repo_ext = repo->p_impl.get()->libsolv_repo_ext;
        if (libsolv_repo_ext.main_nrepodata < 2) {
            continue;
        }
        // now check if the repo already contains all of our file provides
        solv::IdQueue * addedq = libsolv_repo == pool->installed ? &addedfileprovides_inst : &addedfileprovides;
        if (addedq->size() == 0) {
            continue;
        }
        Repodata * data = repo_id2repodata(libsolv_repo, 1);
        fileprovidesq.clear();
        if (repodata_lookup_idarray(data, SOLVID_META, REPOSITORY_ADDEDFILEPROVIDES, &fileprovidesq.get_queue())) {
            if (is_superset(fileprovidesq, addedq, providedids)) {
                continue;
            }
        }
        repodata_set_idarray(data, SOLVID_META, REPOSITORY_ADDEDFILEPROVIDES, &addedq->get_queue());
        repodata_internalize(data);
        // re-write main data only
        int oldnrepodata = libsolv_repo->nrepodata;
        int oldnsolvables = libsolv_repo->nsolvables;
        int oldend = libsolv_repo->end;
        libsolv_repo->nrepodata = libsolv_repo_ext.main_nrepodata;
        libsolv_repo->nsolvables = libsolv_repo_ext.main_nsolvables;
        libsolv_repo->end = libsolv_repo_ext.main_end;
        logger.debug(fmt::format("rewriting repo: {}", libsolv_repo->name));
        write_main(libsolv_repo_ext, false);
        libsolv_repo->nrepodata = oldnrepodata;
        libsolv_repo->nsolvables = oldnsolvables;
        libsolv_repo->end = oldend;
    }
}

SolvSack::Impl::RepodataState SolvSack::Impl::load_repo_main(Repo & repo) {
    RepodataState data_state = RepodataState::NEW;
    auto repo_impl = repo.p_impl.get();
    if (repo_impl->repomd_fn.empty()) {
        throw Exception("repo md file name is empty");
    }

    auto & logger = base->get_logger();
    auto id = repo.get_id().c_str();
    std::unique_ptr<LibsolvRepo, decltype(&libsolv_repo_free)> libsolv_repo(repo_create(pool, id), &libsolv_repo_free);
    const char * fn_repomd = repo_impl->repomd_fn.c_str();
    auto fn_cache = give_repo_solv_cache_fn(id, nullptr);

    std::unique_ptr<std::FILE, decltype(&close_file)> fp_repomd(fopen(fn_repomd, "rb"), &close_file);
    if (!fp_repomd) {
        throw SystemError(errno, fn_repomd);
    }
    checksum_fp(repo_impl->libsolv_repo_ext.checksum, fp_repomd.get());
    std::unique_ptr<std::FILE, decltype(&close_file)> fp_cache(fopen(fn_cache.c_str(), "rb"), &close_file);
    if (can_use_repomd_cache(fp_cache.get(), repo_impl->libsolv_repo_ext.checksum)) {
        //const char *chksum = pool_checksum_str(pool, repoImpl->checksum);
        //logger.debug("using cached %s (0x%s)", name, chksum);
        if (repo_add_solv(libsolv_repo.get(), fp_cache.get(), 0)) {
            throw Exception(_("repo_add_solv() has failed."));
            // g_set_error (error, DNF_ERROR, DNF_ERROR_INTERNAL_ERROR, _("repo_add_solv() has failed."));
        }
        data_state = RepodataState::LOADED_CACHE;
    } else {
        auto primary = repo.get_metadata_path(Repo::Impl::MD_FILENAME_PRIMARY);
        if (primary.empty()) {
            // It could happen when repomd file has no "primary" data or they are in unsupported
            // format like zchunk
            throw Exception(_("loading of MD_FILENAME_PRIMARY has failed."));
            // g_set_error (error, DNF_ERROR, DNF_ERROR_INTERNAL_ERROR, _("loading of MD_FILENAME_PRIMARY has failed."));
        }
        std::unique_ptr<std::FILE, decltype(&close_file)> fp_primary(solv_xfopen(primary.c_str(), "r"), &close_file);
        assert(fp_primary);

        logger.debug(std::string("fetching ") + id);
        if (repo_add_repomdxml(libsolv_repo.get(), fp_repomd.get(), 0) ||
            repo_add_rpmmd(libsolv_repo.get(), fp_primary.get(), 0, 0)) {
            throw Exception(_("repo_add_repomdxml/rpmmd() has failed."));
            // g_set_error (error, DNF_ERROR,  DNF_ERROR_INTERNAL_ERROR, _("repo_add_repomdxml/rpmmd() has failed."));
        }
        data_state = RepodataState::LOADED_FETCH;
    }

    repo_impl->attach_libsolv_repo(libsolv_repo.release());
    provides_ready = false;
    return data_state;
}

SolvSack::Impl::RepodataInfo SolvSack::Impl::load_repo_ext(
    Repo & repo, const char * suffix, const char * which_filename, int flags, bool (*cb)(LibsolvRepo *, FILE *)) {
    RepodataInfo info;
    auto & logger = base->get_logger();
    auto repo_impl = repo.p_impl.get();
    auto libsolv_repo = repo_impl->libsolv_repo_ext.repo;
    const char * repo_id = libsolv_repo->name;

    // nothing set
    auto fn = repo.get_metadata_path(which_filename);
    if (fn.empty()) {
        // g_set_error (error, DNF_ERROR, DNF_ERROR_NO_CAPABILITY, _("no %1$s string for %2$s"), which_filename, repo_id);
        throw NoCapability(fmt::format(_("no {0} string for {1}"), which_filename, repo_id));
    }

    auto fn_cache = give_repo_solv_cache_fn(repo_id, suffix);
    std::unique_ptr<std::FILE, decltype(&close_file)> fp(fopen(fn_cache.c_str(), "rb"), &close_file);
    assert(repo_impl->libsolv_repo_ext.checksum);
    if (can_use_repomd_cache(fp.get(), repo_impl->libsolv_repo_ext.checksum)) {
        logger.debug(fmt::format("{}: using cache file: {}", __func__, fn_cache));
        if (repo_add_solv(libsolv_repo, fp.get(), flags) != 0) {
            // g_set_error_literal (error, DNF_ERROR, DNF_ERROR_INTERNAL_ERROR, _("failed to add solv"));
            throw Exception(_("repo_add_solv() has failed."));
        }
        info.state = RepodataState::LOADED_CACHE;
        info.id = libsolv_repo->nrepodata - 1;
        return info;
    }

    fp.reset(solv_xfopen(fn.c_str(), "r"));
    if (!fp) {
        // g_set_error (error, DNF_ERROR, DNF_ERROR_FILE_INVALID, _("failed to open: %s"), fn.c_str());
        throw Exception(fmt::format(_("failed to open: {}"), fn));
    }
    logger.debug(fmt::format("{}: loading: {}", __func__, fn.c_str()));

    int previous_last = libsolv_repo->nrepodata - 1;
    auto ok = cb(libsolv_repo, fp.get());
    if (ok) {
        info.state = RepodataState::LOADED_FETCH;
        assert(previous_last == libsolv_repo->nrepodata - 2);
        info.id = libsolv_repo->nrepodata - 1;
    }
    provides_ready = false;
    return info;
}

void SolvSack::Impl::internalize_libsolv_repos() {
    int i;
    LibsolvRepo * libsolv_repo;

    FOR_REPOS(i, libsolv_repo) { internalize_libsolv_repo(libsolv_repo); }
}

void SolvSack::Impl::internalize_libsolv_repo(LibsolvRepo * libsolv_repo) {
    if (auto repo = static_cast<Repo *>(libsolv_repo->appdata)) {
        repo->p_impl->libsolv_repo_ext.internalize();
    } else {
        // TODO(jrohel): Do we allow existence of libsolv repo without appdata set?
        repo_internalize(libsolv_repo);
    }
}

void SolvSack::Impl::make_provides_ready() {
    if (provides_ready) {
        return;
    }
    internalize_libsolv_repos();
    solv::IdQueue addedfileprovides;
    solv::IdQueue addedfileprovides_inst;
    pool_addfileprovides_queue(pool, &addedfileprovides.get_queue(), &addedfileprovides_inst.get_queue());
    if (!addedfileprovides.empty() || !addedfileprovides_inst.empty()) {
        rewrite_repos(addedfileprovides, addedfileprovides_inst);
    }
    pool_createwhatprovides(pool);
    provides_ready = true;
}

bool SolvSack::Impl::load_system_repo() {
    auto & logger = base->get_logger();
    auto repo_impl = system_repo->p_impl.get();
    auto id = system_repo->get_id().c_str();

    std::unique_ptr<LibsolvRepo, decltype(&libsolv_repo_free)> libsolv_repo(repo_create(pool, id), &libsolv_repo_free);

    logger.debug("load_system_repo(): fetching rpmdb");
    int flagsrpm = REPO_REUSE_REPODATA | RPM_ADD_WITH_HDRID | REPO_USE_ROOTDIR;
    int rc = repo_add_rpmdb(libsolv_repo.get(), nullptr, flagsrpm);
    if (rc != 0) {
        // g_set_error(error, DNF_ERROR, DNF_ERROR_FILE_INVALID, _("failed loading RPMDB"));
        logger.warning(fmt::format(_("load_system_repo(): failed loading RPMDB: {}"), pool_errstr(pool)));
        return false;
    }

    repo_impl->attach_libsolv_repo(libsolv_repo.release());

    auto libsolv_repo_ext = repo_impl->libsolv_repo_ext;

    pool_set_installed(pool, libsolv_repo_ext.repo);

    // TODO(jrohel): Probably not needed for system repository. For consistency with available repositories?
    libsolv_repo_ext.main_nsolvables = libsolv_repo_ext.repo->nsolvables;
    libsolv_repo_ext.main_nrepodata = libsolv_repo_ext.repo->nrepodata;
    libsolv_repo_ext.main_end = libsolv_repo_ext.repo->end;

    provides_ready = false;
    considered_uptodate = false;

    return true;
}

void SolvSack::Impl::load_available_repo(Repo & repo, LoadRepoFlags flags) {
    auto & logger = base->get_logger();
    auto repo_impl = repo.p_impl.get();

    bool build_cache = repo.get_config().build_cache().get_value();
    auto state = load_repo_main(repo);
    if (state == RepodataState::LOADED_FETCH && build_cache) {
        write_main(repo_impl->libsolv_repo_ext, true);
    }
    repo_impl->libsolv_repo_ext.main_nsolvables = repo_impl->libsolv_repo_ext.repo->nsolvables;
    repo_impl->libsolv_repo_ext.main_nrepodata = repo_impl->libsolv_repo_ext.repo->nrepodata;
    repo_impl->libsolv_repo_ext.main_end = repo_impl->libsolv_repo_ext.repo->end;
    if (any(flags & LoadRepoFlags::USE_FILELISTS)) {
        try {
            // do not pollute the main pool with directory component ids flags |= REPO_LOCALPOOL
            auto repodata_info = load_repo_ext(
                repo,
                SOLV_EXT_FILENAMES,
                Repo::Impl::MD_FILENAME_FILELISTS,
                REPO_EXTEND_SOLVABLES | REPO_LOCALPOOL,
                [](LibsolvRepo * repo, FILE * fp) {
                    return repo_add_rpmmd(repo, fp, "FL", REPO_EXTEND_SOLVABLES) == 0;
                });
            if (repodata_info.state == RepodataState::LOADED_FETCH && build_cache) {
                write_ext(repo_impl->libsolv_repo_ext, repodata_info.id, RepodataType::FILENAMES, SOLV_EXT_FILENAMES);
            }
        } catch (const NoCapability & ex) {
            logger.debug(fmt::format("no filelists metadata available for {}", repo_impl->id));
        }
    }
    if (any(flags & LoadRepoFlags::USE_OTHER)) {
        try {
            // do not pollute the main pool with directory component ids flags |= REPO_LOCALPOOL
            auto repodata_info = load_repo_ext(
                repo,
                SOLV_EXT_OTHER,
                Repo::Impl::MD_FILENAME_OTHER,
                REPO_EXTEND_SOLVABLES | REPO_LOCALPOOL,
                [](LibsolvRepo * repo, FILE * fp) { return repo_add_rpmmd(repo, fp, 0, REPO_EXTEND_SOLVABLES) == 0; });
            if (repodata_info.state == RepodataState::LOADED_FETCH && build_cache) {
                write_ext(repo_impl->libsolv_repo_ext, repodata_info.id, RepodataType::OTHER, SOLV_EXT_OTHER);
            }
        } catch (const NoCapability & ex) {
            logger.debug(fmt::format("no other metadata available for {}", repo_impl->id));
        }
    }
    if (any(flags & LoadRepoFlags::USE_PRESTO)) {
        try {
            auto repodata_info = load_repo_ext(
                repo,
                SOLV_EXT_PRESTO,
                Repo::Impl::MD_FILENAME_PRESTODELTA,
                REPO_EXTEND_SOLVABLES,
                [](LibsolvRepo * repo, FILE * fp) { return repo_add_deltainfoxml(repo, fp, 0) == 0; });
            if (repodata_info.state == RepodataState::LOADED_FETCH && build_cache) {
                write_ext(repo_impl->libsolv_repo_ext, repodata_info.id, RepodataType::PRESTO, SOLV_EXT_PRESTO);
            }
        } catch (const NoCapability & ex) {
            logger.debug(fmt::format("no presto metadata available for {}", repo_impl->id));
        }
    }
    // updateinfo must come *after* all other extensions, as it is not a real
    //   extension, but contains a new set of packages
    if (any(flags & LoadRepoFlags::USE_UPDATEINFO)) {
        try {
            // the updateinfo is not a real extension flags = 0
            auto repodata_info = load_repo_ext(
                repo, SOLV_EXT_UPDATEINFO, Repo::Impl::MD_FILENAME_UPDATEINFO, 0, [](LibsolvRepo * repo, FILE * fp) {
                    return repo_add_updateinfoxml(repo, fp, 0) == 0;
                });
            if (repodata_info.state == RepodataState::LOADED_FETCH && build_cache) {
                write_ext(repo_impl->libsolv_repo_ext, repodata_info.id, RepodataType::UPDATEINFO, SOLV_EXT_UPDATEINFO);
            }
        } catch (const NoCapability & ex) {
            logger.debug(fmt::format("no updateinfo available for {}", repo_impl->id));
        }
    }
    considered_uptodate = false;
}


void SolvSack::load_repo(Repo & repo, LoadRepoFlags flags) {
    auto repo_impl = repo.p_impl.get();
    if (repo_impl->type != Repo::Type::AVAILABLE) {
        throw LogicError("SolvSack::load_repo(): User can load only \"available\" repository");
    }
    p_impl->load_available_repo(repo, flags);
}

void SolvSack::create_system_repo(bool build_cache) {
    if (p_impl->system_repo) {
        throw LogicError("SolvSack::create_system_repo(): System repo already exists");
    }
    p_impl->system_repo = std::make_unique<Repo>(SYSTEM_REPO_NAME, *p_impl->base, Repo::Type::SYSTEM);
    p_impl->system_repo->get_config().build_cache().set(libdnf::Option::Priority::RUNTIME, build_cache);
    p_impl->load_system_repo();
}

Repo & SolvSack::Impl::get_cmdline_repo() {
    if (cmdline_repo) {
        return *cmdline_repo.get();
    }
    cmdline_repo = std::make_unique<Repo>(CMDLINE_REPO_NAME, *base, Repo::Type::SYSTEM);
    cmdline_repo->get_config().build_cache().set(libdnf::Option::Priority::RUNTIME, false);

    std::unique_ptr<LibsolvRepo, decltype(&libsolv_repo_free)> libsolv_repo(
        repo_create(pool, CMDLINE_REPO_NAME), &libsolv_repo_free);
    cmdline_repo->p_impl->attach_libsolv_repo(libsolv_repo.release());
    return *cmdline_repo.get();
}

Package SolvSack::add_cmdline_package(const std::string & fn, bool add_with_hdrid) {
    auto & repo = p_impl->get_cmdline_repo();
    auto new_id = repo.p_impl->add_rpm_package(fn, add_with_hdrid);

    p_impl->provides_ready = false;
    p_impl->considered_uptodate = false;
    return Package(this, PackageId(new_id));
}

Repo & SolvSack::Impl::get_system_repo(bool build_cache) {
    if (system_repo) {
        return *system_repo.get();
    }

    system_repo = std::make_unique<Repo>(SYSTEM_REPO_NAME, *base, Repo::Type::SYSTEM);
    system_repo->get_config().build_cache().set(libdnf::Option::Priority::RUNTIME, build_cache);

    std::unique_ptr<LibsolvRepo, decltype(&libsolv_repo_free)> libsolv_repo(
        repo_create(pool, SYSTEM_REPO_NAME), &libsolv_repo_free);
    system_repo->p_impl->attach_libsolv_repo(libsolv_repo.release());
    pool_set_installed(pool, system_repo->p_impl->libsolv_repo_ext.repo);
    return *system_repo.get();
}

Package SolvSack::add_system_package(const std::string & fn, bool add_with_hdrid, bool build_cache) {
    auto & repo = p_impl->get_system_repo(build_cache);
    auto new_id = repo.p_impl->add_rpm_package(fn, add_with_hdrid);

    p_impl->provides_ready = false;
    p_impl->considered_uptodate = false;
    return Package(this, PackageId(new_id));
}

void SolvSack::dump_debugdata(const std::string & dir) {
    Solver * solver = solver_create(p_impl->pool);

    try {
        std::filesystem::create_directory(dir);

        int ret = testcase_write(solver, dir.c_str(), 0, NULL, NULL);
        if (!ret) {
            throw SystemError(errno, fmt::format("Failed to write debug data to {}", dir));
        }
    } catch (...) {
        solver_free(solver);
        throw;
    }
    solver_free(solver);
}

SolvSackWeakPtr SolvSack::get_weak_ptr() {
    return SolvSackWeakPtr(this, &p_impl->data_guard);
}

int SolvSack::get_nsolvables() const noexcept {
    return p_impl->get_nsolvables();
};

// TODO(jrohel): we want to change directory for solv(x) cache (into repo metadata directory?)
std::string SolvSack::Impl::give_repo_solv_cache_fn(const std::string & repoid, const char * ext) {
    std::filesystem::path cachedir = base->get_config().cachedir().get_value();
    auto fn = cachedir / repoid;
    if (ext) {
        fn += ext;
        fn += ".solvx";
    } else {
        fn += ".solv";
    }
    return fn;
}

SolvSack::SolvSack(Base & base) : p_impl{new Impl(base)} {}

SolvSack::~SolvSack() = default;

}  // namespace libdnf::rpm
