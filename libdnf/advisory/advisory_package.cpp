/*
Copyright (C) 2021 Red Hat, Inc.

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

#include "libdnf/advisory/advisory_package.hpp"

#include "advisory_package_private.hpp"

#include "libdnf/logger/logger.hpp"
#include "libdnf/rpm/nevra.hpp"
#include "libdnf/rpm/solv/package_private.hpp"

#include <solv/chksum.h>
#include <solv/repo.h>
#include <solv/util.h>


namespace libdnf::advisory {

// AdvisoryPackage
AdvisoryPackage::AdvisoryPackage(AdvisoryPackage::Impl * private_pkg) : p_impl(private_pkg) {}

AdvisoryPackage::~AdvisoryPackage() = default;

AdvisoryPackage::AdvisoryPackage(const AdvisoryPackage & src) : p_impl(new Impl(*src.p_impl)) {}
AdvisoryPackage::AdvisoryPackage(AdvisoryPackage && src) : p_impl(new Impl(std::move(*src.p_impl))) {}

AdvisoryPackage & AdvisoryPackage::operator=(const AdvisoryPackage & src) {
    *p_impl = *src.p_impl;
    return *this;
}

AdvisoryPackage & AdvisoryPackage::operator=(AdvisoryPackage && src) noexcept {
    p_impl.swap(src.p_impl);
    return *this;
}


std::string AdvisoryPackage::get_name() const {
    return p_impl->get_name();
}

std::string AdvisoryPackage::get_version() const {
    return p_impl->get_version();
}

std::string AdvisoryPackage::get_evr() const {
    return p_impl->get_evr();
}
std::string AdvisoryPackage::get_arch() const {
    return p_impl->get_arch();
}
AdvisoryId AdvisoryPackage::get_advisory_id() const {
    return p_impl->get_advisory_id();
}
Advisory AdvisoryPackage::get_advisory() const {
    return Advisory(p_impl->sack, p_impl->get_advisory_id());
}
AdvisoryCollection AdvisoryPackage::get_advisory_collection() const {
    return AdvisoryCollection(p_impl->sack, p_impl->get_advisory_id(), p_impl->owner_collection_index);
}

// AdvisoryPackage::Impl
AdvisoryPackage::Impl::Impl(
    libdnf::rpm::PackageSack & sack,
    AdvisoryId advisory,
    int owner_collection_index,
    Id name,
    Id evr,
    Id arch,
    const char * filename)
    : advisory(advisory)
    , owner_collection_index(owner_collection_index)
    , name(name)
    , evr(evr)
    , arch(arch)
    , filename(filename)
    , sack(sack.get_weak_ptr()) {}

AdvisoryPackage::Impl::Impl(const Impl & other)
    : advisory(other.advisory)
    , owner_collection_index(other.owner_collection_index)
    , name(other.name)
    , evr(other.evr)
    , arch(other.arch)
    , filename(other.filename)
    , sack(other.sack) {}

AdvisoryPackage::Impl::Impl(Impl && other)
    : advisory(std::move(other.advisory))
    , owner_collection_index(other.owner_collection_index)
    , name(std::move(other.name))
    , evr(std::move(other.evr))
    , arch(std::move(other.arch))
    , filename(std::move(other.filename))
    , sack(std::move(other.sack)) {}

AdvisoryPackage::Impl & AdvisoryPackage::Impl::operator=(const Impl & other) {
    advisory = other.advisory;
    owner_collection_index = other.owner_collection_index;
    name = other.name;
    evr = other.evr;
    arch = other.arch;
    filename = other.filename;
    sack = other.sack;
    return *this;
}

AdvisoryPackage::Impl & AdvisoryPackage::Impl::operator=(Impl && other) {
    advisory = std::move(other.advisory);
    owner_collection_index = std::move(other.owner_collection_index);
    name = std::move(other.name);
    evr = std::move(other.evr);
    arch = std::move(other.arch);
    filename = std::move(other.filename);
    sack = std::move(other.sack);
    return *this;
}


std::string AdvisoryPackage::Impl::get_name() const {
    Pool * pool = sack->p_impl->get_pool();
    return pool_id2str(pool, name);
}

std::string AdvisoryPackage::Impl::get_version() const {
    Pool * pool = sack->p_impl->get_pool();
    char * e;
    char * v;
    char * r;
    libdnf::rpm::solv::pool_split_evr(pool, pool_id2str(pool, evr), &e, &v, &r);
    return std::string(v);
}

std::string AdvisoryPackage::Impl::get_evr() const {
    Pool * pool = sack->p_impl->get_pool();
    return pool_id2str(pool, evr);
}

std::string AdvisoryPackage::Impl::get_arch() const {
    Pool * pool = sack->p_impl->get_pool();
    return pool_id2str(pool, arch);
}

}  // namespace libdnf::advisory
