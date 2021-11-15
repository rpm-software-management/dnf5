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
#include <solv/repo_rpmmd.h>
#include <solv/repo_solv.h>
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

}  // end of anonymous namespace


void PackageSack::Impl::internalize_libsolv_repos() {
    auto rq = repo::RepoQuery(base);
    for (auto & repo : rq.get_data()) {
        repo->p_impl->solv_repo.internalize();
    }

    if (system_repo) {
        system_repo->p_impl->solv_repo.internalize();
    }

    if (cmdline_repo) {
        cmdline_repo->p_impl->solv_repo.internalize();
    }
}

void PackageSack::Impl::make_provides_ready() {
    if (provides_ready) {
        return;
    }

    internalize_libsolv_repos();

    auto & pool = get_pool(base);
    libdnf::solv::IdQueue addedfileprovides;
    libdnf::solv::IdQueue addedfileprovides_inst;
    pool_addfileprovides_queue(*pool, &addedfileprovides.get_queue(), &addedfileprovides_inst.get_queue());

    if (system_repo && !addedfileprovides_inst.empty()) {
        system_repo->p_impl->solv_repo.rewrite_repo(addedfileprovides_inst);
    }

    if (!addedfileprovides.empty()) {
        auto rq = repo::RepoQuery(base);
        for (auto & repo : rq.get_data()) {
            repo->p_impl->solv_repo.rewrite_repo(addedfileprovides);
        }
    }

    pool_createwhatprovides(*pool);
    provides_ready = true;
}

void PackageSack::Impl::load_available_repo(repo::Repo & repo, LoadRepoFlags flags) {
    auto & logger = *base->get_logger();
    auto & repo_impl = repo.p_impl;

    auto primary_fn = repo.get_metadata_path(repo::Repo::Impl::MD_FILENAME_PRIMARY);
    if (primary_fn.empty()) {
        throw Exception(_("Failed to load repository: \"primary\" data not present or in unsupported format"));
    }

    repo_impl->solv_repo.load_repo_main(repo_impl->downloader.get_repomd_filename(), primary_fn);

    if (any(flags & LoadRepoFlags::FILELISTS)) {
        auto md_filename = repo.get_metadata_path(repo::Repo::Impl::MD_FILENAME_FILELISTS);

        if (!md_filename.empty()) {
            repo.p_impl->solv_repo.load_repo_ext(md_filename, repo::RepodataType::FILENAMES);
        } else {
            logger.debug(fmt::format("no filelists metadata available for {}", repo.get_id()));
        }
    }
    if (any(flags & LoadRepoFlags::OTHER)) {
        auto md_filename = repo.get_metadata_path(repo::Repo::Impl::MD_FILENAME_OTHER);

        if (!md_filename.empty()) {
            repo.p_impl->solv_repo.load_repo_ext(md_filename, repo::RepodataType::OTHER);
        } else {
            logger.debug(fmt::format("no other metadata available for {}", repo.get_id()));
        }
    }
    if (any(flags & LoadRepoFlags::PRESTO)) {
        auto md_filename = repo.get_metadata_path(repo::Repo::Impl::MD_FILENAME_PRESTODELTA);

        if (!md_filename.empty()) {
            repo.p_impl->solv_repo.load_repo_ext(md_filename, repo::RepodataType::PRESTO);
        } else {
            logger.debug(fmt::format("no presto metadata available for {}", repo.get_id()));
        }
    }

    // updateinfo must come *after* all other extensions, as it is not a real
    //   extension, but contains a new set of packages
    if (any(flags & LoadRepoFlags::UPDATEINFO)) {
        auto md_filename = repo.get_metadata_path(repo::Repo::Impl::MD_FILENAME_UPDATEINFO);

        if (!md_filename.empty()) {
            repo.p_impl->solv_repo.load_repo_ext(md_filename, repo::RepodataType::UPDATEINFO);
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
            base->get_comps()->load_from_file(repo.get_weak_ptr(), path);
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
