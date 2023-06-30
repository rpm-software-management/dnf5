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

#include "libdnf5/advisory/advisory_package.hpp"

#include "advisory_package_private.hpp"
#include "rpm/package_set_impl.hpp"

#include <solv/chksum.h>
#include <solv/repo.h>
#include <solv/util.h>


namespace libdnf5::advisory {

// AdvisoryPackage
AdvisoryPackage::AdvisoryPackage(AdvisoryPackage::Impl * private_pkg) : p_impl(private_pkg) {}

AdvisoryPackage::AdvisoryPackage(const AdvisoryPackage & src) = default;
AdvisoryPackage::AdvisoryPackage(AdvisoryPackage && src) noexcept = default;
AdvisoryPackage & AdvisoryPackage::operator=(const AdvisoryPackage & src) = default;
AdvisoryPackage & AdvisoryPackage::operator=(AdvisoryPackage && src) noexcept = default;
AdvisoryPackage::~AdvisoryPackage() = default;


std::string AdvisoryPackage::get_name() const {
    return p_impl->get_name();
}

std::string AdvisoryPackage::get_epoch() const {
    return p_impl->get_epoch();
}

std::string AdvisoryPackage::get_version() const {
    return p_impl->get_version();
}

std::string AdvisoryPackage::get_release() const {
    return p_impl->get_release();
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
    return p_impl->get_reboot_suggested();
}
bool AdvisoryPackage::get_restart_suggested() const {
    return p_impl->get_restart_suggested();
}
bool AdvisoryPackage::get_relogin_suggested() const {
    return p_impl->get_relogin_suggested();
}

// AdvisoryPackage::Impl
AdvisoryPackage::Impl::Impl(
    const libdnf5::BaseWeakPtr & base,
    AdvisoryId advisory,
    int owner_collection_index,
    Id name,
    Id evr,
    Id arch,
    bool reboot_suggested,
    bool restart_suggested,
    bool relogin_suggested,
    const char * filename)
    : advisory(advisory),
      owner_collection_index(owner_collection_index),
      name(name),
      evr(evr),
      arch(arch),
      reboot_suggested(reboot_suggested),
      restart_suggested(restart_suggested),
      relogin_suggested(relogin_suggested),
      filename(filename),
      base(base) {}

std::string AdvisoryPackage::Impl::get_name() const {
    return get_rpm_pool(base).id2str(name);
}

std::string AdvisoryPackage::Impl::get_epoch() const {
    auto & pool = get_rpm_pool(base);
    return pool.split_evr(pool.id2str(evr)).e_def();
}

std::string AdvisoryPackage::Impl::get_version() const {
    auto & pool = get_rpm_pool(base);
    return pool.split_evr(pool.id2str(evr)).v;
}

std::string AdvisoryPackage::Impl::get_release() const {
    auto & pool = get_rpm_pool(base);
    return pool.split_evr(pool.id2str(evr)).r;
}

std::string AdvisoryPackage::Impl::get_evr() const {
    return get_rpm_pool(base).id2str(evr);
}

std::string AdvisoryPackage::Impl::get_arch() const {
    return get_rpm_pool(base).id2str(arch);
}

bool AdvisoryPackage::Impl::is_resolved_in(const libdnf5::rpm::PackageSet & pkgs) const {
    auto & pool = get_rpm_pool(base);
    auto sack = base->get_rpm_package_sack();
    auto & sorted_solvables = sack->p_impl->get_sorted_solvables();

    auto low = std::lower_bound(
        sorted_solvables.begin(),
        sorted_solvables.end(),
        *this,
        libdnf5::advisory::AdvisoryPackage::Impl::name_arch_compare_lower_solvable);
    while (low != sorted_solvables.end() && (*low)->name == get_name_id() && (*low)->arch == get_arch_id()) {
        int libsolv_cmp = pool.evrcmp((*low)->evr, get_evr_id(), EVRCMP_COMPARE);
        if (libsolv_cmp >= 0) {  // We are interested only in lower or equal evr
            if (pkgs.p_impl->contains(pool.solvable2id(*low))) {
                return true;
            }
        }
        ++low;
    }

    return false;
}

}  // namespace libdnf5::advisory
