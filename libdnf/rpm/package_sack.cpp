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


#include "libdnf/common/exception.hpp"
#include "../libdnf/utils/bgettext/bgettext-lib.h"
#include "package_sack_impl.hpp"
#include "../repo/repo_impl.hpp"
#include "libdnf/solv/id_queue.hpp"
#include "libdnf/solv/solv_map.hpp"
#include "libdnf/utils/temp.hpp"

extern "C" {
#include <solv/chksum.h>
#include <solv/repo.h>
#include <solv/repo_comps.h>
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
bool is_superset(const libdnf::solv::IdQueue & q1, const libdnf::solv::IdQueue * q2, libdnf::solv::SolvMap & map) {
    int cnt = 0;
    for (int i = 0; i < q2->size(); i++) {
        map.add_unsafe((*q2)[i]);
    }
    for (int i = 0; i < q1.size(); i++) {
        if (map.contains_unsafe(q1[i])) {
            cnt++;
        }
    }
    for (int i = 0; i < q2->size(); i++) {
        map.remove_unsafe((*q2)[i]);
    }
    return cnt == q2->size();
}

}  // end of anonymous namespace

void PackageSack::Impl::rewrite_repos(libdnf::solv::IdQueue & addedfileprovides, libdnf::solv::IdQueue & addedfileprovides_inst) {
    int i;
    auto & logger = *base->get_logger();

    auto & pool = get_pool(base);
    libdnf::solv::SolvMap providedids(pool->ss.nstrings);

    libdnf::solv::IdQueue fileprovidesq;

    LibsolvRepo * libsolv_repo;
    FOR_REPOS(i, libsolv_repo) {
        auto repo = static_cast<repo::Repo *>(libsolv_repo->appdata);
        if (!repo) {
            continue;
        }
        if (!(repo->get_config().build_cache().get_value())) {
            continue;
        }
        auto & solv_repo = repo->p_impl.get()->solv_repo;
        if (solv_repo.main_nrepodata < 2) {
            continue;
        }
        // now check if the repo already contains all of our file provides
        libdnf::solv::IdQueue * addedq = libsolv_repo == pool->installed ? &addedfileprovides_inst : &addedfileprovides;
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
        libsolv_repo->nrepodata = solv_repo.main_nrepodata;
        libsolv_repo->nsolvables = solv_repo.main_nsolvables;
        libsolv_repo->end = solv_repo.main_end;
        logger.debug(fmt::format("rewriting repo: {}", libsolv_repo->name));
        repo->p_impl->solv_repo.write_main(false);
        libsolv_repo->nrepodata = oldnrepodata;
        libsolv_repo->nsolvables = oldnsolvables;
        libsolv_repo->end = oldend;
    }
}

repo::RepodataState PackageSack::Impl::load_repo_main(repo::Repo & repo) {
    repo::RepodataState data_state = repo::RepodataState::NEW;
    auto repo_impl = repo.p_impl.get();

    std::string repomd_fn = repo_impl->downloader.get_repomd_filename();
    if (repomd_fn.empty()) {
        throw Exception("repo md file name is empty");
    }

    auto & logger = *base->get_logger();
    auto id = repo.get_id();
    std::unique_ptr<LibsolvRepo, decltype(&libsolv_repo_free)> libsolv_repo(
        repo_create(*get_pool(base), id.c_str()), &libsolv_repo_free);

    auto fn_cache = repo_solv_cache_path(id, nullptr);

    std::unique_ptr<std::FILE, decltype(&close_file)> fp_repomd(fopen(repomd_fn.c_str(), "rb"), &close_file);
    if (!fp_repomd) {
        throw SystemError(errno, repomd_fn);
    }
    checksum_fp(repo_impl->solv_repo.checksum, fp_repomd.get());
    std::unique_ptr<std::FILE, decltype(&close_file)> fp_cache(fopen(fn_cache.c_str(), "rb"), &close_file);
    if (can_use_repomd_cache(fp_cache.get(), repo_impl->solv_repo.checksum)) {
        //const char *chksum = pool_checksum_str(pool, repoImpl->checksum);
        //logger.debug("using cached %s (0x%s)", name, chksum);
        if (repo_add_solv(libsolv_repo.get(), fp_cache.get(), 0)) {
            throw Exception(M_("repo_add_solv() has failed."));
            // g_set_error (error, DNF_ERROR, DNF_ERROR_INTERNAL_ERROR, _("repo_add_solv() has failed."));
        }
        data_state = repo::RepodataState::LOADED_CACHE;
    } else {
        auto primary = repo.get_metadata_path(repo::Repo::Impl::MD_FILENAME_PRIMARY);
        if (primary.empty()) {
            // It could happen when repomd file has no "primary" data or they are in unsupported
            // format like zchunk
            throw Exception(M_("loading of MD_FILENAME_PRIMARY has failed."));
            // g_set_error (error, DNF_ERROR, DNF_ERROR_INTERNAL_ERROR, _("loading of MD_FILENAME_PRIMARY has failed."));
        }
        std::unique_ptr<std::FILE, decltype(&close_file)> fp_primary(solv_xfopen(primary.c_str(), "r"), &close_file);

        if (!fp_primary) {
            // TODO(lukash) proper exception
            throw RuntimeError(fmt::format("Failed to open repo primary \"{}\": {}", primary, errno));
        }

        logger.debug(std::string("fetching ") + id);
        if (repo_add_repomdxml(libsolv_repo.get(), fp_repomd.get(), 0) ||
            repo_add_rpmmd(libsolv_repo.get(), fp_primary.get(), 0, 0)) {
            throw Exception(M_("repo_add_repomdxml/rpmmd() has failed."));
            // g_set_error (error, DNF_ERROR,  DNF_ERROR_INTERNAL_ERROR, _("repo_add_repomdxml/rpmmd() has failed."));
        }
        data_state = repo::RepodataState::LOADED_FETCH;
    }

    repo_impl->attach_libsolv_repo(libsolv_repo.release());
    provides_ready = false;
    return data_state;
}

void PackageSack::Impl::internalize_libsolv_repos() {
    int i;
    LibsolvRepo * libsolv_repo;
    ::Pool * pool = *get_pool(base);

    FOR_REPOS(i, libsolv_repo) { internalize_libsolv_repo(libsolv_repo); }
}

void PackageSack::Impl::internalize_libsolv_repo(LibsolvRepo * libsolv_repo) {
    if (auto repo = static_cast<repo::Repo *>(libsolv_repo->appdata)) {
        repo->p_impl->solv_repo.internalize();
    } else {
        // TODO(jrohel): Do we allow existence of libsolv repo without appdata set?
        repo_internalize(libsolv_repo);
    }
}

void PackageSack::Impl::make_provides_ready() {
    if (provides_ready) {
        return;
    }

    auto & pool = get_pool(base);

    internalize_libsolv_repos();
    libdnf::solv::IdQueue addedfileprovides;
    libdnf::solv::IdQueue addedfileprovides_inst;
    pool_addfileprovides_queue(*pool, &addedfileprovides.get_queue(), &addedfileprovides_inst.get_queue());
    if (!addedfileprovides.empty() || !addedfileprovides_inst.empty()) {
        rewrite_repos(addedfileprovides, addedfileprovides_inst);
    }
    pool_createwhatprovides(*pool);
    provides_ready = true;
}

bool PackageSack::Impl::load_system_repo() {
    auto & logger = *base->get_logger();
    auto repo_impl = system_repo->p_impl.get();
    auto & pool = get_pool(base);

    std::unique_ptr<LibsolvRepo, decltype(&libsolv_repo_free)> libsolv_repo(
        repo_create(*pool, system_repo->get_id().c_str()), &libsolv_repo_free);

    logger.debug("load_system_repo(): fetching rpmdb");
    base->get_config().installroot().lock("installroot locked by load_system_repo");
    pool_set_rootdir(*pool, base->get_config().installroot().get_value().c_str());
    int flagsrpm = REPO_REUSE_REPODATA | RPM_ADD_WITH_HDRID | REPO_USE_ROOTDIR;
    int rc = repo_add_rpmdb(libsolv_repo.get(), nullptr, flagsrpm);
    if (rc != 0) {
        // g_set_error(error, DNF_ERROR, DNF_ERROR_FILE_INVALID, _("failed loading RPMDB"));
        logger.warning(fmt::format(_("load_system_repo(): failed loading RPMDB: {}"), pool_errstr(*pool)));
        return false;
    }

    repo_impl->attach_libsolv_repo(libsolv_repo.release());

    auto solv_repo = repo_impl->solv_repo;

    pool_set_installed(*pool, solv_repo.repo);

    // TODO(jrohel): Probably not needed for system repository. For consistency with available repositories?
    solv_repo.main_nsolvables = solv_repo.repo->nsolvables;
    solv_repo.main_nrepodata = solv_repo.repo->nrepodata;
    solv_repo.main_end = solv_repo.repo->end;

    provides_ready = false;
    considered_uptodate = false;

    return true;
}

bool PackageSack::Impl::load_extra_system_repo(const std::string & rootdir) {
    auto & logger = *base->get_logger();
    auto & solv_repo = system_repo->p_impl.get()->solv_repo;
    LibsolvRepo * libsolv_repo = solv_repo.repo;
    auto & pool = get_pool(base);

    logger.debug("load_extra_system_repo(): fetching rpmdb");
    pool_set_rootdir(*pool, rootdir.c_str());
    int flagsrpm = REPO_REUSE_REPODATA | RPM_ADD_WITH_HDRID | REPO_USE_ROOTDIR;
    int rc = repo_add_rpmdb(libsolv_repo, nullptr, flagsrpm);
    //reset rootdir to main one
    pool_set_rootdir(*pool, base->get_config().installroot().get_value().c_str());
    if (rc != 0) {
        logger.warning(fmt::format(_("load_extra_system_repo(): failed loading RPMDB: {}"), pool_errstr(*pool)));
        return false;
    }

    solv_repo.main_nsolvables = solv_repo.repo->nsolvables;
    solv_repo.main_nrepodata = solv_repo.repo->nrepodata;
    solv_repo.main_end = solv_repo.repo->end;

    provides_ready = false;
    considered_uptodate = false;

    return true;
}

void PackageSack::Impl::load_available_repo(repo::Repo & repo, LoadRepoFlags flags) {
    auto & logger = *base->get_logger();
    auto repo_impl = repo.p_impl.get();

    bool build_cache = repo.get_config().build_cache().get_value();
    auto state = load_repo_main(repo);
    if (state == repo::RepodataState::LOADED_FETCH && build_cache) {
        repo_impl->solv_repo.write_main(true);
    }
    repo_impl->solv_repo.main_nsolvables = repo_impl->solv_repo.repo->nsolvables;
    repo_impl->solv_repo.main_nrepodata = repo_impl->solv_repo.repo->nrepodata;
    repo_impl->solv_repo.main_end = repo_impl->solv_repo.repo->end;
    if (any(flags & LoadRepoFlags::FILELISTS)) {
        auto md_filename = repo.get_metadata_path(repo::Repo::Impl::MD_FILENAME_FILELISTS);

        if (!md_filename.empty()) {
            // do not pollute the main pool with directory component ids flags |= REPO_LOCALPOOL
            auto repodata_info = repo.p_impl->solv_repo.load_repo_ext(
                SOLV_EXT_FILENAMES,
                md_filename,
                REPO_EXTEND_SOLVABLES | REPO_LOCALPOOL,
                [](LibsolvRepo * repo, FILE * fp) {
                    return repo_add_rpmmd(repo, fp, "FL", REPO_EXTEND_SOLVABLES) == 0;
                });
            provides_ready = false;
            if (repodata_info.state == repo::RepodataState::LOADED_FETCH && build_cache) {
                repo_impl->solv_repo.write_ext(repodata_info.id, repo::RepodataType::FILENAMES, SOLV_EXT_FILENAMES);
            }
        } else {
            logger.debug(fmt::format("no filelists metadata available for {}", repo.get_id()));
        }
    }
    if (any(flags & LoadRepoFlags::OTHER)) {
        auto md_filename = repo.get_metadata_path(repo::Repo::Impl::MD_FILENAME_OTHER);

        if (!md_filename.empty()) {
            // do not pollute the main pool with directory component ids flags |= REPO_LOCALPOOL
            auto repodata_info = repo.p_impl->solv_repo.load_repo_ext(
                SOLV_EXT_OTHER,
                md_filename,
                REPO_EXTEND_SOLVABLES | REPO_LOCALPOOL,
                [](LibsolvRepo * repo, FILE * fp) { return repo_add_rpmmd(repo, fp, 0, REPO_EXTEND_SOLVABLES) == 0; });
            provides_ready = false;
            if (repodata_info.state == repo::RepodataState::LOADED_FETCH && build_cache) {
                repo_impl->solv_repo.write_ext(repodata_info.id, repo::RepodataType::OTHER, SOLV_EXT_OTHER);
            }
        } else {
            logger.debug(fmt::format("no other metadata available for {}", repo.get_id()));
        }
    }
    if (any(flags & LoadRepoFlags::PRESTO)) {
        auto md_filename = repo.get_metadata_path(repo::Repo::Impl::MD_FILENAME_PRESTODELTA);

        if (!md_filename.empty()) {
            auto repodata_info = repo.p_impl->solv_repo.load_repo_ext(
                SOLV_EXT_PRESTO,
                md_filename,
                REPO_EXTEND_SOLVABLES,
                [](LibsolvRepo * repo, FILE * fp) { return repo_add_deltainfoxml(repo, fp, 0) == 0; });
            provides_ready = false;
            if (repodata_info.state == repo::RepodataState::LOADED_FETCH && build_cache) {
                repo_impl->solv_repo.write_ext(repodata_info.id, repo::RepodataType::PRESTO, SOLV_EXT_PRESTO);
            }
        } else {
            logger.debug(fmt::format("no presto metadata available for {}", repo.get_id()));
        }
    }

    // updateinfo must come *after* all other extensions, as it is not a real
    //   extension, but contains a new set of packages
    if (any(flags & LoadRepoFlags::UPDATEINFO)) {
        auto md_filename = repo.get_metadata_path(repo::Repo::Impl::MD_FILENAME_UPDATEINFO);

        if (!md_filename.empty()) {
            // the updateinfo is not a real extension flags = 0
            auto repodata_info = repo.p_impl->solv_repo.load_repo_ext(
                SOLV_EXT_UPDATEINFO,
                md_filename,
                0,
                [](LibsolvRepo * repo, FILE * fp) { return repo_add_updateinfoxml(repo, fp, 0) == 0; });
            provides_ready = false;
            if (repodata_info.state == repo::RepodataState::LOADED_FETCH && build_cache) {
                repo_impl->solv_repo.write_ext(repodata_info.id, repo::RepodataType::UPDATEINFO, SOLV_EXT_UPDATEINFO);
            }
        } else {
            logger.debug(fmt::format("no updateinfo available for {}", repo.get_id()));
        }
    }

    if (any(flags & LoadRepoFlags::COMPS)) {
        auto path = repo_impl->get_metadata_path(repo::Repo::Impl::MD_FILENAME_GROUP_GZ);
        if (path.empty()) {
            path = repo_impl->get_metadata_path(repo::Repo::Impl::MD_FILENAME_GROUP);
        }
        if (!path.empty()) {
            base->get_comps()->load_from_file(repo, path);
        }
    }


    considered_uptodate = false;
}


void PackageSack::load_repo(repo::Repo & repo, LoadRepoFlags flags) {
    auto repo_impl = repo.p_impl.get();
    libdnf_assert(repo_impl->type == repo::Repo::Type::AVAILABLE, "Only repositories of type AVAILABLE can be loaded");

    p_impl->load_available_repo(repo, flags);
}

void PackageSack::create_system_repo(bool build_cache) {
    libdnf_assert(!p_impl->system_repo, "System repo already exists");

    p_impl->system_repo = std::make_unique<repo::Repo>(p_impl->base, SYSTEM_REPO_NAME, repo::Repo::Type::SYSTEM);
    p_impl->system_repo->get_config().build_cache().set(libdnf::Option::Priority::RUNTIME, build_cache);
    p_impl->load_system_repo();
}


libdnf::repo::RepoWeakPtr PackageSack::get_system_repo() const {
    return p_impl->system_repo->get_weak_ptr();
}


void PackageSack::append_extra_system_repo(const std::string & rootdir) {
    libdnf_assert(p_impl->system_repo != nullptr, "System repo does not exist");

    p_impl->load_extra_system_repo(rootdir);
}

repo::Repo & PackageSack::Impl::get_cmdline_repo() {
    if (cmdline_repo) {
        return *cmdline_repo.get();
    }

    cmdline_repo = std::make_unique<repo::Repo>(base, CMDLINE_REPO_NAME, repo::Repo::Type::SYSTEM);
    cmdline_repo->get_config().build_cache().set(libdnf::Option::Priority::RUNTIME, false);

    std::unique_ptr<LibsolvRepo, decltype(&libsolv_repo_free)> libsolv_repo(
        repo_create(*get_pool(base), CMDLINE_REPO_NAME), &libsolv_repo_free);
    cmdline_repo->p_impl->attach_libsolv_repo(libsolv_repo.release());
    return *cmdline_repo.get();
}

Package PackageSack::add_cmdline_package(const std::string & fn, bool add_with_hdrid) {
    auto & repo = p_impl->get_cmdline_repo();
    auto new_id = repo.p_impl->add_rpm_package(fn, add_with_hdrid);

    p_impl->provides_ready = false;
    p_impl->considered_uptodate = false;
    return Package(p_impl->base, PackageId(new_id));
}

repo::Repo & PackageSack::Impl::get_system_repo(bool build_cache) {
    if (system_repo) {
        return *system_repo.get();
    }
    auto & pool = get_pool(base);

    system_repo = std::make_unique<repo::Repo>(base, SYSTEM_REPO_NAME, repo::Repo::Type::SYSTEM);
    system_repo->get_config().build_cache().set(libdnf::Option::Priority::RUNTIME, build_cache);

    std::unique_ptr<LibsolvRepo, decltype(&libsolv_repo_free)> libsolv_repo(
        repo_create(*pool, SYSTEM_REPO_NAME), &libsolv_repo_free);
    system_repo->p_impl->attach_libsolv_repo(libsolv_repo.release());
    pool_set_installed(*pool, system_repo->p_impl->solv_repo.repo);
    return *system_repo.get();
}

Package PackageSack::add_system_package(const std::string & fn, bool add_with_hdrid, bool build_cache) {
    auto & repo = p_impl->get_system_repo(build_cache);
    auto new_id = repo.p_impl->add_rpm_package(fn, add_with_hdrid);

    p_impl->provides_ready = false;
    p_impl->considered_uptodate = false;
    return Package(p_impl->base, PackageId(new_id));
}

void PackageSack::dump_debugdata(const std::string & dir) {
    Solver * solver = solver_create(*get_pool(p_impl->base));

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

PackageSackWeakPtr PackageSack::get_weak_ptr() {
    return PackageSackWeakPtr(this, &p_impl->sack_guard);
}

BaseWeakPtr PackageSack::get_base() const {
    return p_impl->base->get_weak_ptr();
}

int PackageSack::get_nsolvables() const noexcept {
    return p_impl->get_nsolvables();
};

// TODO(jrohel): we want to change directory for solv(x) cache (into repo metadata directory?)
std::string PackageSack::Impl::repo_solv_cache_fn(const std::string & repoid, const char * ext) {
    if (ext != nullptr) {
        return repoid + ext + ".solvx";
    } else {
        return repoid + ".solv";
    }
}

std::string PackageSack::Impl::repo_solv_cache_path(const std::string & repoid, const char * ext) {
    return std::filesystem::path(base->get_config().cachedir().get_value()) / repo_solv_cache_fn(repoid, ext);
}

PackageSack::PackageSack(const BaseWeakPtr & base)
  : p_impl{new Impl(base)},
    system_state(base->get_config().installroot().get_value())
{}

PackageSack::PackageSack(libdnf::Base & base) : PackageSack(base.get_weak_ptr()) {}

PackageSack::~PackageSack() = default;

}  // namespace libdnf::rpm
