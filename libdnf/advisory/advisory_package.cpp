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

#include "libdnf/advisory/advisory_package.hpp"

#include "advisory_package_private.hpp"

#include "libdnf/logger/logger.hpp"

#include <solv/chksum.h>
#include <solv/repo.h>
#include <solv/util.h>


namespace libdnf::advisory {

// AdvisoryPackage
AdvisoryPackage::AdvisoryPackage(AdvisoryPackage::Impl * private_pkg) : p_impl(private_pkg) {}

AdvisoryPackage::AdvisoryPackage(const AdvisoryPackage & src) : p_impl(new Impl(*src.p_impl)) {}

AdvisoryPackage & AdvisoryPackage::operator=(const AdvisoryPackage & src) {
    *p_impl = *src.p_impl;
    return *this;
}

AdvisoryPackage::~AdvisoryPackage() = default;


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
std::string AdvisoryPackage::get_nevra() const {
    return p_impl->get_name() + "-" + p_impl->get_evr() + "." + p_impl->get_arch();
}
AdvisoryId AdvisoryPackage::get_advisory_id() const {
    return p_impl->get_advisory_id();
}
Advisory AdvisoryPackage::get_advisory() const {
    return Advisory(p_impl->base, p_impl->get_advisory_id());
}
AdvisoryCollection AdvisoryPackage::get_advisory_collection() const {
    return AdvisoryCollection(p_impl->base, p_impl->get_advisory_id(), p_impl->owner_collection_index);
}
bool AdvisoryPackage::get_reboot_suggested() const {
    //TODO(amatej): implement
    return false;
}
bool AdvisoryPackage::get_restart_suggested() const {
    //TODO(amatej): implement
    return false;
}
bool AdvisoryPackage::get_relogin_suggested() const {
    //TODO(amatej): implement
    return false;
}

// AdvisoryPackage::Impl
AdvisoryPackage::Impl::Impl(
    const libdnf::BaseWeakPtr & base,
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
    , base(base) {}

std::string AdvisoryPackage::Impl::get_name() const {
    return get_pool(base).id2str(name);
}

std::string AdvisoryPackage::Impl::get_version() const {
    auto & pool = get_pool(base);
    return pool.split_evr(pool.id2str(evr)).v;
}

std::string AdvisoryPackage::Impl::get_evr() const {
    return get_pool(base).id2str(evr);
}

std::string AdvisoryPackage::Impl::get_arch() const {
    return get_pool(base).id2str(arch);
}

}  // namespace libdnf::advisory
