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


libdnf::repo::RepoWeakPtr PackageSack::get_system_repo() const {
    return p_impl->get_system_repo();
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
    return Package(p_impl->base, PackageId(new_id));
}

repo::RepoWeakPtr PackageSack::Impl::get_system_repo() {
    if (!system_repo) {
        system_repo = std::make_unique<repo::Repo>(base, SYSTEM_REPO_NAME, repo::Repo::Type::SYSTEM);
        pool_set_installed(*get_pool(base), system_repo->p_impl->solv_repo.repo);
    }

    return system_repo->get_weak_ptr();
}

Package PackageSack::add_system_package(const std::string & fn, bool add_with_hdrid) {
    auto new_id = p_impl->get_system_repo()->p_impl->add_rpm_package(fn, add_with_hdrid);

    p_impl->provides_ready = false;
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
