/*
Copyright (C) 2020 Red Hat, Inc.

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


#include "libdnf/rpm/package.hpp"

#include "reldep_list_impl.hpp"
#include "solv/package_private.hpp"
#include "solv_sack_impl.hpp"


inline static std::string cstring2string(const char * input) {
    return input ? std::string(input) : std::string();
}


namespace libdnf::rpm {

const char * Package::get_name_cstring() const {
    Pool * pool = sack->p_impl->pool;
    return solv::get_name(pool, id);
}

std::string Package::get_name() const {
    Pool * pool = sack->p_impl->pool;
    return cstring2string(solv::get_name(pool, id));
}

const char * Package::get_epoch_cstring() const {
    Pool * pool = sack->p_impl->pool;
    return solv::get_epoch_cstring(pool, id);
}

std::string Package::get_epoch() const {
    Pool * pool = sack->p_impl->pool;
    return solv::get_epoch_cstring(pool, id);
}

const char * Package::get_version_cstring() const {
    Pool * pool = sack->p_impl->pool;
    return solv::get_version(pool, id);
}

std::string Package::get_version() const {
    Pool * pool = sack->p_impl->pool;
    return cstring2string(solv::get_version(pool, id));
}

const char * Package::get_release_cstring() const {
    Pool * pool = sack->p_impl->pool;
    return solv::get_release(pool, id);
}

std::string Package::get_release() const {
    Pool * pool = sack->p_impl->pool;
    return cstring2string(solv::get_release(pool, id));
}

const char * Package::get_arch_cstring() const {
    Pool * pool = sack->p_impl->pool;
    return solv::get_arch(pool, id);
}

std::string Package::get_arch() const {
    Pool * pool = sack->p_impl->pool;
    return cstring2string(solv::get_arch(pool, id));
}

const char * Package::get_evr_cstring() const {
    Pool * pool = sack->p_impl->pool;
    return solv::get_evr(pool, id);
}

std::string Package::get_evr() const {
    Pool * pool = sack->p_impl->pool;
    return cstring2string(solv::get_evr(pool, id));
}

std::string Package::get_nevra() const {
    Pool * pool = sack->p_impl->pool;
    return cstring2string(solv::get_nevra(pool, id));
}

std::string Package::get_full_nevra() const {
    Pool * pool = sack->p_impl->pool;
    return cstring2string(solv::get_full_nevra(pool, id));
}

std::string Package::get_group() const {
    Pool * pool = sack->p_impl->pool;
    return cstring2string(solv::get_group(pool, id));
}

unsigned long long Package::get_size() const {
    Pool * pool = sack->p_impl->pool;
    return solv::get_size(pool, id);
}

unsigned long long Package::get_download_size() const {
    Pool * pool = sack->p_impl->pool;
    return solv::get_download_size(pool, id);
}

unsigned long long Package::get_install_size() const {
    Pool * pool = sack->p_impl->pool;
    return solv::get_install_size(pool, id);
}

std::string Package::get_license() const {
    Pool * pool = sack->p_impl->pool;
    return cstring2string(solv::get_license(pool, id));
}

std::string Package::get_sourcerpm() const {
    Pool * pool = sack->p_impl->pool;
    return cstring2string(solv::get_sourcerpm(pool, id));
}

unsigned long long Package::get_build_time() const {
    Pool * pool = sack->p_impl->pool;
    return solv::get_build_time(pool, id);
}

// TODO not supported by libsolv: https://github.com/openSUSE/libsolv/issues/400
//std::string Package::get_build_host() {
//    Pool * pool = sack->p_impl->pool;
//    return cstring2string(solv::get_build_host(pool, id));
//}

std::string Package::get_packager() const {
    Pool * pool = sack->p_impl->pool;
    return cstring2string(solv::get_packager(pool, id));
}

std::string Package::get_vendor() const {
    Pool * pool = sack->p_impl->pool;
    return cstring2string(solv::get_vendor(pool, id));
}

std::string Package::get_url() const {
    Pool * pool = sack->p_impl->pool;
    return cstring2string(solv::get_url(pool, id));
}

std::string Package::get_summary() const {
    Pool * pool = sack->p_impl->pool;
    return cstring2string(solv::get_summary(pool, id));
}

std::string Package::get_description() const {
    Pool * pool = sack->p_impl->pool;
    return cstring2string(solv::get_description(pool, id));
}

std::vector<std::string> Package::get_files() const {
    Pool * pool = sack->p_impl->pool;
    return solv::get_files(pool, id);
}

ReldepList Package::get_provides() const {
    Pool * pool = sack->p_impl->pool;
    ReldepList list(sack.get());
    solv::get_provides(pool, id, list.p_impl->queue);
    return list;
}

ReldepList Package::get_requires() const {
    Pool * pool = sack->p_impl->pool;
    ReldepList list(sack.get());
    solv::get_requires(pool, id, list.p_impl->queue);
    return list;
}

ReldepList Package::get_requires_pre() const {
    Pool * pool = sack->p_impl->pool;
    ReldepList list(sack.get());
    solv::get_requires_pre(pool, id, list.p_impl->queue);
    return list;
}

ReldepList Package::get_conflicts() const {
    Pool * pool = sack->p_impl->pool;
    ReldepList list(sack.get());
    solv::get_conflicts(pool, id, list.p_impl->queue);
    return list;
}

ReldepList Package::get_obsoletes() const {
    Pool * pool = sack->p_impl->pool;
    ReldepList list(sack.get());
    solv::get_obsoletes(pool, id, list.p_impl->queue);
    return list;
}

ReldepList Package::get_recommends() const {
    Pool * pool = sack->p_impl->pool;
    ReldepList list(sack.get());
    solv::get_recommends(pool, id, list.p_impl->queue);
    return list;
}

ReldepList Package::get_suggests() const {
    Pool * pool = sack->p_impl->pool;
    ReldepList list(sack.get());
    solv::get_suggests(pool, id, list.p_impl->queue);
    return list;
}

ReldepList Package::get_enhances() const {
    Pool * pool = sack->p_impl->pool;
    ReldepList list(sack.get());
    solv::get_enhances(pool, id, list.p_impl->queue);
    return list;
}

ReldepList Package::get_supplements() const {
    Pool * pool = sack->p_impl->pool;
    ReldepList list(sack.get());
    solv::get_supplements(pool, id, list.p_impl->queue);
    return list;
}

ReldepList Package::get_prereq_ignoreinst() const {
    Pool * pool = sack->p_impl->pool;
    ReldepList list(sack.get());
    solv::get_prereq_ignoreinst(pool, id, list.p_impl->queue);
    return list;
}

ReldepList Package::get_regular_requires() const {
    Pool * pool = sack->p_impl->pool;
    ReldepList list(sack.get());
    solv::get_regular_requires(pool, id, list.p_impl->queue);
    return list;
}

std::string Package::get_baseurl() const {
    Pool * pool = sack->p_impl->pool;
    return cstring2string(solv::get_baseurl(pool, id));
}

std::string Package::get_location() const {
    Pool * pool = sack->p_impl->pool;
    return cstring2string(solv::get_location(pool, id));
}

std::string Package::get_local_filepath() const {
    Pool * pool = sack->p_impl->pool;
    return solv::get_local_filepath(pool, id);
}

bool Package::is_installed() const {
    Pool * pool = sack->p_impl->pool;
    return solv::is_installed(pool, solv::get_solvable(pool, id));
}

unsigned long long Package::get_hdr_end() const {
    Pool * pool = sack->p_impl->pool;
    return solv::get_hdr_end(pool, id);
}

unsigned long long Package::get_install_time() const {
    Pool * pool = sack->p_impl->pool;
    return solv::get_install_time(pool, id);
}

unsigned long long Package::get_media_number() const {
    Pool * pool = sack->p_impl->pool;
    return solv::get_media_number(pool, id);
}

unsigned long long Package::get_rpmdbid() const {
    Pool * pool = sack->p_impl->pool;
    return solv::get_rpmdbid(pool, id);
}

Repo * Package::get_repo() const {
    Pool * pool = sack->p_impl->pool;
    return solv::get_repo(pool, id);
}

std::string Package::get_repo_id() const {
    return get_repo()->get_id();
}

Checksum Package::get_checksum() const {
    Pool * pool = sack->p_impl->pool;

    Solvable * solvable = solv::get_solvable(pool, id);
    int type;
    solv::SolvPrivate::internalize_libsolv_repo(solvable->repo);
    const char * chksum = solvable_lookup_checksum(solvable, SOLVABLE_CHECKSUM, &type);
    Checksum checksum(chksum, type);

    return checksum;
}

Checksum Package::get_hdr_checksum() const {
    Pool * pool = sack->p_impl->pool;

    Solvable * solvable = solv::get_solvable(pool, id);
    int type;
    solv::SolvPrivate::internalize_libsolv_repo(solvable->repo);
    const char * chksum = solvable_lookup_checksum(solvable, SOLVABLE_HDRID, &type);
    Checksum checksum(chksum, type);

    return checksum;
}

}  // namespace libdnf::rpm
