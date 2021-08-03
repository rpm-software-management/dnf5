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
#include "libdnf/rpm/solv/solv_private.hpp"
#include "libdnf/solv/pool.hpp"

#include <filesystem>


static inline std::string cstring2string(const char * input) {
    return input ? std::string(input) : std::string();
}

static inline unsigned long long lookup_num(Solvable * solvable, Id type) {
    libdnf::rpm::solv::SolvPrivate::internalize_libsolv_repo(solvable->repo);
    return solvable_lookup_num(solvable, type, 0);
}

static inline const char * lookup_cstring(Solvable * solvable, Id type) {
    libdnf::rpm::solv::SolvPrivate::internalize_libsolv_repo(solvable->repo);
    return solvable_lookup_str(solvable, type);
}

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
    libdnf::solv::Pool pool(sack->p_impl->pool);
    return cstring2string(pool.get_name(id.id));
}

std::string Package::get_epoch() const {
    libdnf::solv::Pool pool(sack->p_impl->pool);
    return cstring2string(pool.get_epoch(id.id));
}

std::string Package::get_version() const {
    libdnf::solv::Pool pool(sack->p_impl->pool);
    return cstring2string(pool.get_version(id.id));
}

std::string Package::get_release() const {
    libdnf::solv::Pool pool(sack->p_impl->pool);
    return cstring2string(pool.get_release(id.id));
}

std::string Package::get_arch() const {
    libdnf::solv::Pool pool(sack->p_impl->pool);
    return cstring2string(pool.get_arch(id.id));
}

std::string Package::get_evr() const {
    libdnf::solv::Pool pool(sack->p_impl->pool);
    return cstring2string(pool.get_evr(id.id));
}

std::string Package::get_nevra() const {
    libdnf::solv::Pool pool(sack->p_impl->pool);
    return cstring2string(pool.get_nevra(id.id));
}

std::string Package::get_full_nevra() const {
    libdnf::solv::Pool pool(sack->p_impl->pool);
    return pool.get_full_nevra(id.id);
}

std::string Package::get_group() const {
    libdnf::solv::Pool pool(sack->p_impl->pool);
    return cstring2string(lookup_cstring(pool.id2solvable(id.id), SOLVABLE_GROUP));
}

unsigned long long Package::get_package_size() const {
    libdnf::solv::Pool pool(sack->p_impl->pool);
    return lookup_num(pool.id2solvable(id.id), SOLVABLE_DOWNLOADSIZE);
}

unsigned long long Package::get_install_size() const {
    libdnf::solv::Pool pool(sack->p_impl->pool);
    return lookup_num(pool.id2solvable(id.id), SOLVABLE_INSTALLSIZE);
}

std::string Package::get_license() const {
    libdnf::solv::Pool pool(sack->p_impl->pool);
    return cstring2string(lookup_cstring(pool.id2solvable(id.id), SOLVABLE_LICENSE));
}

std::string Package::get_sourcerpm() const {
    libdnf::solv::Pool pool(sack->p_impl->pool);
    return cstring2string(pool.get_sourcerpm(id.id));
}

unsigned long long Package::get_build_time() const {
    libdnf::solv::Pool pool(sack->p_impl->pool);
    return lookup_num(pool.id2solvable(id.id), SOLVABLE_BUILDTIME);
}

// TODO not supported by libsolv: https://github.com/openSUSE/libsolv/issues/400
//std::string Package::get_build_host() {
//    libdnf::solv::Pool pool(sack->p_impl->pool);
//    return cstring2string(lookup_cstring(pool.id2solvable(id.id), SOLVABLE_BUILDHOST));
//}

std::string Package::get_packager() const {
    libdnf::solv::Pool pool(sack->p_impl->pool);
    return cstring2string(lookup_cstring(pool.id2solvable(id.id), SOLVABLE_PACKAGER));
}

std::string Package::get_vendor() const {
    libdnf::solv::Pool pool(sack->p_impl->pool);
    return cstring2string(lookup_cstring(pool.id2solvable(id.id), SOLVABLE_VENDOR));
}

std::string Package::get_url() const {
    libdnf::solv::Pool pool(sack->p_impl->pool);
    return cstring2string(lookup_cstring(pool.id2solvable(id.id), SOLVABLE_URL));
}

std::string Package::get_summary() const {
    libdnf::solv::Pool pool(sack->p_impl->pool);
    return cstring2string(lookup_cstring(pool.id2solvable(id.id), SOLVABLE_SUMMARY));
}

std::string Package::get_description() const {
    libdnf::solv::Pool pool(sack->p_impl->pool);
    return cstring2string(lookup_cstring(pool.id2solvable(id.id), SOLVABLE_DESCRIPTION));
}

std::vector<std::string> Package::get_files() const {
    libdnf::solv::Pool pool(sack->p_impl->pool);

    Solvable * solvable = pool.id2solvable(id.id);
    libdnf::rpm::solv::SolvPrivate::internalize_libsolv_repo(solvable->repo);

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

ReldepList Package::get_provides() const {
    libdnf::solv::Pool pool(sack->p_impl->pool);
    ReldepList list(sack);
    reldeps_for(pool.id2solvable(id.id), list.p_impl->queue, SOLVABLE_PROVIDES);
    return list;
}

ReldepList Package::get_requires() const {
    libdnf::solv::Pool pool(sack->p_impl->pool);
    Solvable * solvable = pool.id2solvable(id.id);
    ReldepList list(sack);
    reldeps_for(solvable, list.p_impl->queue, SOLVABLE_REQUIRES);

    libdnf::solv::IdQueue tmp_queue;
    reldeps_for(solvable, tmp_queue, SOLVABLE_PREREQMARKER);
    list.p_impl->queue += tmp_queue;

    return list;
}

ReldepList Package::get_requires_pre() const {
    libdnf::solv::Pool pool(sack->p_impl->pool);
    ReldepList list(sack);
    reldeps_for(pool.id2solvable(id.id), list.p_impl->queue, SOLVABLE_PREREQMARKER);
    return list;
}

ReldepList Package::get_conflicts() const {
    libdnf::solv::Pool pool(sack->p_impl->pool);
    ReldepList list(sack);
    reldeps_for(pool.id2solvable(id.id), list.p_impl->queue, SOLVABLE_CONFLICTS);
    return list;
}

ReldepList Package::get_obsoletes() const {
    libdnf::solv::Pool pool(sack->p_impl->pool);
    ReldepList list(sack);
    reldeps_for(pool.id2solvable(id.id), list.p_impl->queue, SOLVABLE_OBSOLETES);
    return list;
}

ReldepList Package::get_recommends() const {
    libdnf::solv::Pool pool(sack->p_impl->pool);
    ReldepList list(sack);
    reldeps_for(pool.id2solvable(id.id), list.p_impl->queue, SOLVABLE_RECOMMENDS);
    return list;
}

ReldepList Package::get_suggests() const {
    libdnf::solv::Pool pool(sack->p_impl->pool);
    ReldepList list(sack);
    reldeps_for(pool.id2solvable(id.id), list.p_impl->queue, SOLVABLE_SUGGESTS);
    return list;
}

ReldepList Package::get_enhances() const {
    libdnf::solv::Pool pool(sack->p_impl->pool);
    ReldepList list(sack);
    reldeps_for(pool.id2solvable(id.id), list.p_impl->queue, SOLVABLE_ENHANCES);
    return list;
}

ReldepList Package::get_supplements() const {
    libdnf::solv::Pool pool(sack->p_impl->pool);
    ReldepList list(sack);
    reldeps_for(pool.id2solvable(id.id), list.p_impl->queue, SOLVABLE_SUPPLEMENTS);
    return list;
}

ReldepList Package::get_prereq_ignoreinst() const {
    libdnf::solv::Pool pool(sack->p_impl->pool);
    ReldepList list(sack);
    reldeps_for(pool.id2solvable(id.id), list.p_impl->queue, SOLVABLE_PREREQ_IGNOREINST);
    return list;
}

ReldepList Package::get_regular_requires() const {
    libdnf::solv::Pool pool(sack->p_impl->pool);
    ReldepList list(sack);
    reldeps_for(pool.id2solvable(id.id), list.p_impl->queue, SOLVABLE_REQUIRES);
    return list;
}

std::string Package::get_baseurl() const {
    libdnf::solv::Pool pool(sack->p_impl->pool);
    return cstring2string(lookup_cstring(pool.id2solvable(id.id), SOLVABLE_MEDIABASE));
}

std::string Package::get_location() const {
    libdnf::solv::Pool pool(sack->p_impl->pool);
    Solvable * solvable = pool.id2solvable(id.id);
    libdnf::rpm::solv::SolvPrivate::internalize_libsolv_repo(solvable->repo);
    return cstring2string(solvable_lookup_location(solvable, nullptr));
}

//TODO(jrohel): What about local repositories? The original code in DNF4 uses baseurl+get_location(pool, package_id).
std::string Package::get_package_path() const {
    libdnf::solv::Pool pool(sack->p_impl->pool);
    Solvable * solvable = pool.id2solvable(id.id);
    if (auto repo = static_cast<repo::Repo *>(solvable->repo->appdata)) {
        auto dir = std::filesystem::path(repo->get_cachedir()) / "packages";
        return dir / std::filesystem::path(get_location()).filename();
    } else {
        return "";
    }
}

bool Package::is_installed() const {
    libdnf::solv::Pool pool(sack->p_impl->pool);
    return pool.is_installed(id.id);
}

unsigned long long Package::get_hdr_end() const {
    libdnf::solv::Pool pool(sack->p_impl->pool);
    return lookup_num(pool.id2solvable(id.id), SOLVABLE_HEADEREND);
}

unsigned long long Package::get_install_time() const {
    libdnf::solv::Pool pool(sack->p_impl->pool);
    return lookup_num(pool.id2solvable(id.id), SOLVABLE_INSTALLTIME);
}

unsigned long long Package::get_media_number() const {
    libdnf::solv::Pool pool(sack->p_impl->pool);
    return lookup_num(pool.id2solvable(id.id), SOLVABLE_MEDIANR);
}

unsigned long long Package::get_rpmdbid() const {
    libdnf::solv::Pool pool(sack->p_impl->pool);
    return lookup_num(pool.id2solvable(id.id), RPM_RPMDBID);
}

libdnf::repo::RepoWeakPtr Package::get_repo() const {
    libdnf::solv::Pool pool(sack->p_impl->pool);
    // TODO(lukash) handle nullptr - is it a possibility?
    return pool.get_repo(id.id)->get_weak_ptr();
}

std::string Package::get_repo_id() const {
    libdnf::solv::Pool pool(sack->p_impl->pool);
    // TODO(lukash) handle nullptr - is it a possibility?
    return pool.get_repo(id.id)->get_id();
}

Checksum Package::get_checksum() const {
    libdnf::solv::Pool pool(sack->p_impl->pool);

    Solvable * solvable = pool.id2solvable(id.id);
    int type;
    solv::SolvPrivate::internalize_libsolv_repo(solvable->repo);
    const char * chksum = solvable_lookup_checksum(solvable, SOLVABLE_CHECKSUM, &type);
    Checksum checksum(chksum, type);

    return checksum;
}

Checksum Package::get_hdr_checksum() const {
    libdnf::solv::Pool pool(sack->p_impl->pool);

    Solvable * solvable = pool.id2solvable(id.id);
    int type;
    solv::SolvPrivate::internalize_libsolv_repo(solvable->repo);
    const char * chksum = solvable_lookup_checksum(solvable, SOLVABLE_HDRID, &type);
    Checksum checksum(chksum, type);

    return checksum;
}

}  // namespace libdnf::rpm
