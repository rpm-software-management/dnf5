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


#include "libdnf/rpm/package.hpp"

#include "package_sack_impl.hpp"
#include "reldep_list_impl.hpp"
#include "solv/pool.hpp"
#include "utils/bgettext/bgettext-mark-domain.h"
#include "utils/string.hpp"

#include "libdnf/common/exception.hpp"
#include "libdnf/rpm/package_query.hpp"

#include <filesystem>


static inline void reldeps_for(Solvable * solvable, libdnf::solv::IdQueue & queue, Id type) {
    Id marker = -1;
    Id solv_type = type;

    if (type == SOLVABLE_REQUIRES) {
        marker = -1;
    }

    if (type == SOLVABLE_PREREQMARKER) {
        solv_type = SOLVABLE_REQUIRES;
        marker = 1;
    }
    solvable_lookup_deparray(solvable, solv_type, &queue.get_queue(), marker);
}


namespace libdnf::rpm {

std::string Package::get_name() const {
    return libdnf::utils::string::c_to_str(get_pool(base).get_name(id.id));
}

std::string Package::get_epoch() const {
    return libdnf::utils::string::c_to_str(get_pool(base).get_epoch(id.id));
}

std::string Package::get_version() const {
    return libdnf::utils::string::c_to_str(get_pool(base).get_version(id.id));
}

std::string Package::get_release() const {
    return libdnf::utils::string::c_to_str(get_pool(base).get_release(id.id));
}

std::string Package::get_arch() const {
    return libdnf::utils::string::c_to_str(get_pool(base).get_arch(id.id));
}

std::string Package::get_evr() const {
    return libdnf::utils::string::c_to_str(get_pool(base).get_evr(id.id));
}

std::string Package::get_nevra() const {
    return libdnf::utils::string::c_to_str(get_pool(base).get_nevra(id.id));
}

std::string Package::get_full_nevra() const {
    return get_pool(base).get_full_nevra(id.id);
}

std::string Package::get_na() const {
    std::string res = get_name();
    res.append(".");
    res.append(get_arch());
    return res;
}

std::string Package::get_group() const {
    return libdnf::utils::string::c_to_str(get_pool(base).lookup_str(id.id, SOLVABLE_GROUP));
}

unsigned long long Package::get_package_size() const {
    return get_pool(base).lookup_num(id.id, SOLVABLE_DOWNLOADSIZE);
}

unsigned long long Package::get_install_size() const {
    return get_pool(base).lookup_num(id.id, SOLVABLE_INSTALLSIZE);
}

std::string Package::get_license() const {
    return libdnf::utils::string::c_to_str(get_pool(base).lookup_str(id.id, SOLVABLE_LICENSE));
}

std::string Package::get_source_name() const {
    const char * source_name = get_pool(base).lookup_str(id.id, SOLVABLE_SOURCENAME);
    return source_name ? source_name : get_name();
}

std::string Package::get_sourcerpm() const {
    return libdnf::utils::string::c_to_str(get_pool(base).get_sourcerpm(id.id));
}

unsigned long long Package::get_build_time() const {
    return get_pool(base).lookup_num(id.id, SOLVABLE_BUILDTIME);
}

// TODO not supported by libsolv: https://github.com/openSUSE/libsolv/issues/400
//std::string Package::get_build_host() {
//    return libdnf::utils::string::c_to_str(lookup_cstring(get_pool(base).id2solvable(id.id), SOLVABLE_BUILDHOST));
//}

std::string Package::get_packager() const {
    return libdnf::utils::string::c_to_str(get_pool(base).lookup_str(id.id, SOLVABLE_PACKAGER));
}

std::string Package::get_vendor() const {
    return libdnf::utils::string::c_to_str(get_pool(base).lookup_str(id.id, SOLVABLE_VENDOR));
}

std::string Package::get_url() const {
    return libdnf::utils::string::c_to_str(get_pool(base).lookup_str(id.id, SOLVABLE_URL));
}

std::string Package::get_summary() const {
    return libdnf::utils::string::c_to_str(get_pool(base).lookup_str(id.id, SOLVABLE_SUMMARY));
}

std::string Package::get_description() const {
    return libdnf::utils::string::c_to_str(get_pool(base).lookup_str(id.id, SOLVABLE_DESCRIPTION));
}

std::vector<std::string> Package::get_files() const {
    auto & pool = get_pool(base);

    Solvable * solvable = pool.id2solvable(id.id);
    libdnf::solv::get_repo(solvable).internalize();

    std::vector<std::string> ret;

    Dataiterator di;
    dataiterator_init(
        &di, *pool, solvable->repo, id.id, SOLVABLE_FILELIST, nullptr, SEARCH_FILES | SEARCH_COMPLETE_FILELIST);
    while (dataiterator_step(&di) != 0) {
        ret.emplace_back(di.kv.str);
    }
    dataiterator_free(&di);

    return ret;
}

std::vector<libdnf::rpm::Changelog> Package::get_changelogs() const {
    std::vector<libdnf::rpm::Changelog> changelogs;
    auto & pool = get_pool(base);
    Solvable * solvable = pool.id2solvable(id.id);
    libdnf::solv::get_repo(solvable).internalize();

    Dataiterator di;
    dataiterator_init(&di, *pool, solvable->repo, id.id, SOLVABLE_CHANGELOG, nullptr, 0);
    while (dataiterator_step(&di)) {
        dataiterator_setpos(&di);
        std::string author = pool_lookup_str(*pool, SOLVID_POS, SOLVABLE_CHANGELOG_AUTHOR);
        std::string text = pool_lookup_str(*pool, SOLVID_POS, SOLVABLE_CHANGELOG_TEXT);
        time_t timestamp = static_cast<time_t>(pool_lookup_num(*pool, SOLVID_POS, SOLVABLE_CHANGELOG_TIME, 0));
        changelogs.emplace_back(timestamp, std::move(author), std::move(text));
    }
    dataiterator_free(&di);

    return changelogs;
}

ReldepList Package::get_provides() const {
    ReldepList list(base);
    reldeps_for(get_pool(base).id2solvable(id.id), list.p_impl->queue, SOLVABLE_PROVIDES);
    return list;
}

ReldepList Package::get_requires() const {
    Solvable * solvable = get_pool(base).id2solvable(id.id);
    ReldepList list(base);
    reldeps_for(solvable, list.p_impl->queue, SOLVABLE_REQUIRES);

    libdnf::solv::IdQueue tmp_queue;
    reldeps_for(solvable, tmp_queue, SOLVABLE_PREREQMARKER);
    list.p_impl->queue += tmp_queue;

    return list;
}

ReldepList Package::get_requires_pre() const {
    ReldepList list(base);
    reldeps_for(get_pool(base).id2solvable(id.id), list.p_impl->queue, SOLVABLE_PREREQMARKER);
    return list;
}

ReldepList Package::get_conflicts() const {
    ReldepList list(base);
    reldeps_for(get_pool(base).id2solvable(id.id), list.p_impl->queue, SOLVABLE_CONFLICTS);
    return list;
}

ReldepList Package::get_obsoletes() const {
    ReldepList list(base);
    reldeps_for(get_pool(base).id2solvable(id.id), list.p_impl->queue, SOLVABLE_OBSOLETES);
    return list;
}

ReldepList Package::get_recommends() const {
    ReldepList list(base);
    reldeps_for(get_pool(base).id2solvable(id.id), list.p_impl->queue, SOLVABLE_RECOMMENDS);
    return list;
}

ReldepList Package::get_suggests() const {
    ReldepList list(base);
    reldeps_for(get_pool(base).id2solvable(id.id), list.p_impl->queue, SOLVABLE_SUGGESTS);
    return list;
}

ReldepList Package::get_enhances() const {
    ReldepList list(base);
    reldeps_for(get_pool(base).id2solvable(id.id), list.p_impl->queue, SOLVABLE_ENHANCES);
    return list;
}

ReldepList Package::get_supplements() const {
    ReldepList list(base);
    reldeps_for(get_pool(base).id2solvable(id.id), list.p_impl->queue, SOLVABLE_SUPPLEMENTS);
    return list;
}

ReldepList Package::get_prereq_ignoreinst() const {
    ReldepList list(base);
    reldeps_for(get_pool(base).id2solvable(id.id), list.p_impl->queue, SOLVABLE_PREREQ_IGNOREINST);
    return list;
}

ReldepList Package::get_regular_requires() const {
    ReldepList list(base);
    reldeps_for(get_pool(base).id2solvable(id.id), list.p_impl->queue, SOLVABLE_REQUIRES);
    return list;
}

std::string Package::get_baseurl() const {
    return libdnf::utils::string::c_to_str(get_pool(base).lookup_str(id.id, SOLVABLE_MEDIABASE));
}

std::string Package::get_location() const {
    Solvable * solvable = get_pool(base).id2solvable(id.id);
    libdnf::solv::get_repo(solvable).internalize();
    return libdnf::utils::string::c_to_str(solvable_lookup_location(solvable, nullptr));
}

//TODO(jrohel): What about local repositories? The original code in DNF4 uses baseurl+get_location(pool, package_id).
std::string Package::get_package_path() const {
    Solvable * solvable = get_pool(base).id2solvable(id.id);
    if (auto repo = static_cast<repo::Repo *>(solvable->repo->appdata)) {
        if (repo->get_type() == repo::Repo::Type::COMMANDLINE) {
            // Command line packages are used from their original location.
            return get_location();
        }
        // Returns the path to the cached file.
        auto dir = std::filesystem::path(repo->get_cachedir()) / "packages";
        return dir / std::filesystem::path(get_location()).filename();
    } else {
        return "";
    }
}

bool Package::is_installed() const {
    return get_pool(base).is_installed(id.id);
}

bool Package::is_excluded() const {
    base->get_rpm_package_sack()->p_impl->recompute_considered_in_pool();
    return get_pool(base).is_solvable_excluded(id.id);
}

unsigned long long Package::get_hdr_end() const {
    return get_pool(base).lookup_num(id.id, SOLVABLE_HEADEREND);
}

unsigned long long Package::get_install_time() const {
    return get_pool(base).lookup_num(id.id, SOLVABLE_INSTALLTIME);
}

unsigned long long Package::get_media_number() const {
    return get_pool(base).lookup_num(id.id, SOLVABLE_MEDIANR);
}

unsigned long long Package::get_rpmdbid() const {
    return get_pool(base).lookup_num(id.id, RPM_RPMDBID);
}

libdnf::repo::RepoWeakPtr Package::get_repo() const {
    return get_pool(base).get_repo(id.id).get_weak_ptr();
}

std::string Package::get_repo_id() const {
    return get_pool(base).get_repo(id.id).get_id();
}

libdnf::transaction::TransactionItemReason Package::get_reason() const {
    // TODO(lukash) this query is a temporary solution.
    // The logic should be moved to the system::State, where a cache of
    // installed NAs needs to be created (perhaps still as a query), as it is
    // needed for transaction reason resolution anyway
    rpm::PackageQuery installed_query(base, rpm::PackageQuery::ExcludeFlags::IGNORE_EXCLUDES);
    installed_query.filter_installed();
    installed_query.filter_name({get_name()});
    installed_query.filter_arch({get_arch()});
    if (!installed_query.empty()) {
        auto reason = base->get_system_state().get_package_reason(get_na());

        if (reason == libdnf::transaction::TransactionItemReason::NONE) {
            return libdnf::transaction::TransactionItemReason::EXTERNAL_USER;
        }

        return reason;
    }

    return libdnf::transaction::TransactionItemReason::NONE;
}

Checksum Package::get_checksum() const {
    Solvable * solvable = get_pool(base).id2solvable(id.id);
    int type;
    libdnf::solv::get_repo(solvable).internalize();
    const char * chksum = solvable_lookup_checksum(solvable, SOLVABLE_CHECKSUM, &type);
    Checksum checksum(chksum, type);

    return checksum;
}

Checksum Package::get_hdr_checksum() const {
    Solvable * solvable = get_pool(base).id2solvable(id.id);
    int type;
    libdnf::solv::get_repo(solvable).internalize();
    const char * chksum = solvable_lookup_checksum(solvable, SOLVABLE_HDRID, &type);
    Checksum checksum(chksum, type);

    return checksum;
}

Package::Package(const BaseWeakPtr & base, unsigned long long rpmdbid) : base(base) {
    Pool * pool = *get_pool(base);
    auto * installed_repo = pool->installed;

    libdnf_assert(installed_repo, "Installed repo not loaded");

    for (auto candidate_id = installed_repo->start; candidate_id < installed_repo->end; ++candidate_id) {
        if (rpmdbid == repo_lookup_num(installed_repo, candidate_id, RPM_RPMDBID, 0)) {
            id.id = candidate_id;
            return;
        }
    }

    throw RuntimeError(M_("Package with rpmdbid was not found"));
}

}  // namespace libdnf::rpm
