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

void PackageSack::Impl::load_available_repo(repo::Repo & repo, LoadRepoFlags flags) {
    auto & logger = *base->get_logger();
    auto & repo_impl = repo.p_impl;

    bool build_cache = repo.get_config().build_cache().get_value();

    auto primary_fn = repo.get_metadata_path(repo::Repo::Impl::MD_FILENAME_PRIMARY);
    if (primary_fn.empty()) {
        throw Exception(_("Failed to load repository: \"primary\" data not present or in unsupported format"));
    }

    auto state = repo_impl->solv_repo.load_repo_main(repo_impl->downloader.get_repomd_filename(), primary_fn);

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

    provides_ready = false;
    considered_uptodate = false;
}


void PackageSack::load_repo(repo::Repo & repo, LoadRepoFlags flags) {
    auto repo_impl = repo.p_impl.get();
    libdnf_assert(repo_impl->type == repo::Repo::Type::AVAILABLE, "Only repositories of type AVAILABLE can be loaded");

    p_impl->load_available_repo(repo, flags);
}

void PackageSack::create_system_repo(bool build_cache) {
    libdnf_assert(!p_impl->system_repo, "System repo already exists");

    p_impl->base->get_config().installroot().lock("installroot locked by create_system_repo");

    p_impl->system_repo = std::make_unique<repo::Repo>(p_impl->base, SYSTEM_REPO_NAME, repo::Repo::Type::SYSTEM);
    p_impl->system_repo->get_config().build_cache().set(libdnf::Option::Priority::RUNTIME, build_cache);
    p_impl->system_repo->p_impl->solv_repo.load_system_repo();

    p_impl->provides_ready = false;
    p_impl->considered_uptodate = false;
}


libdnf::repo::RepoWeakPtr PackageSack::get_system_repo() const {
    libdnf_assert(p_impl->system_repo != nullptr, "System repo does not exist");
    return p_impl->system_repo->get_weak_ptr();
}


void PackageSack::append_extra_system_repo(const std::string & rootdir) {
    libdnf_assert(p_impl->system_repo != nullptr, "System repo does not exist");
    p_impl->system_repo->p_impl->solv_repo.load_system_repo(rootdir);
}

repo::Repo & PackageSack::Impl::get_cmdline_repo() {
    if (cmdline_repo) {
        return *cmdline_repo.get();
    }

    cmdline_repo = std::make_unique<repo::Repo>(base, CMDLINE_REPO_NAME, repo::Repo::Type::SYSTEM);
    cmdline_repo->get_config().build_cache().set(libdnf::Option::Priority::RUNTIME, false);

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

PackageSack::PackageSack(const BaseWeakPtr & base)
  : p_impl{new Impl(base)},
    system_state(base->get_config().installroot().get_value())
{}

PackageSack::PackageSack(libdnf::Base & base) : PackageSack(base.get_weak_ptr()) {}

PackageSack::~PackageSack() = default;

}  // namespace libdnf::rpm
