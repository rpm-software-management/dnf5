// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later


#include "libdnf5/advisory/advisory_set.hpp"

#include "advisory/advisory_package_private.hpp"
#include "advisory_set_impl.hpp"
#include "base/base_private.hpp"
#include "solv/solv_map.hpp"

#include "libdnf5/advisory/advisory_set_iterator.hpp"
#include "libdnf5/rpm/nevra.hpp"


namespace libdnf5::advisory {


AdvisorySet::AdvisorySet(const BaseWeakPtr & base) : p_impl(new Impl(base)) {}

AdvisorySet::AdvisorySet(libdnf5::Base & base) : AdvisorySet(base.get_weak_ptr()) {}

AdvisorySet::AdvisorySet(const AdvisorySet & other) : p_impl(new Impl(*other.p_impl)) {}


AdvisorySet::AdvisorySet(AdvisorySet && other) noexcept : p_impl(new Impl(std::move(*other.p_impl))) {}


AdvisorySet::AdvisorySet(const BaseWeakPtr & base, libdnf5::solv::SolvMap & solv_map)
    : p_impl(new Impl(base, solv_map)) {}


AdvisorySet::~AdvisorySet() = default;

AdvisorySet & AdvisorySet::operator=(const AdvisorySet & other) {
    *p_impl = *other.p_impl;
    return *this;
}

AdvisorySet & AdvisorySet::operator=(AdvisorySet && other) {
    *p_impl = std::move(*other.p_impl);
    return *this;
}


AdvisorySet & AdvisorySet::operator|=(const AdvisorySet & other) {
    libdnf_assert_same_base(p_impl->base, other.p_impl->base);
    *p_impl |= *other.p_impl;
    return *this;
}


AdvisorySet & AdvisorySet::operator-=(const AdvisorySet & other) {
    libdnf_assert_same_base(p_impl->base, other.p_impl->base);
    *p_impl -= *other.p_impl;
    return *this;
}


AdvisorySet & AdvisorySet::operator&=(const AdvisorySet & other) {
    libdnf_assert_same_base(p_impl->base, other.p_impl->base);
    *p_impl &= *other.p_impl;
    return *this;
}


void AdvisorySet::clear() noexcept {
    p_impl->clear();
}


bool AdvisorySet::empty() const noexcept {
    return p_impl->empty();
}


std::size_t AdvisorySet::size() const noexcept {
    return p_impl->size();
}


void AdvisorySet::swap(AdvisorySet & other) noexcept {
    p_impl.swap(other.p_impl);
}


void AdvisorySet::add(const Advisory & adv) {
    p_impl->add(adv.get_id().id);
}


bool AdvisorySet::contains(const Advisory & adv) const noexcept {
    return p_impl->contains(adv.get_id().id);
}


void AdvisorySet::remove(const Advisory & adv) {
    p_impl->remove(adv.get_id().id);
}


BaseWeakPtr AdvisorySet::get_base() const {
    return p_impl->base;
}

std::vector<AdvisoryPackage> AdvisorySet::get_advisory_packages_sorted_by_name_arch_evr(bool only_applicable) const {
    std::vector<AdvisoryPackage> out;
    for (Id candidate_id : *p_impl) {
        Advisory advisory2 = Advisory(p_impl->base, AdvisoryId(candidate_id));
        auto collections = advisory2.get_collections();
        for (auto & collection : collections) {
            if (only_applicable && !collection.is_applicable()) {
                continue;
            }
            collection.get_packages(out);
        }
    }

    std::sort(out.begin(), out.end(), AdvisoryPackage::Impl::nevra_compare_lower_id);

    return out;
}

std::vector<AdvisoryPackage> AdvisorySet::get_advisory_packages_sorted_by_name_arch_evr_string(
    bool only_applicable) const {
    std::vector<AdvisoryPackage> out;
    for (Id candidate_id : *p_impl) {
        Advisory advisory2 = Advisory(p_impl->base, AdvisoryId(candidate_id));
        auto collections = advisory2.get_collections();
        for (auto & collection : collections) {
            if (only_applicable && !collection.is_applicable()) {
                continue;
            }
            collection.get_packages(out);
        }
    }

    std::sort(out.begin(), out.end(), libdnf5::rpm::cmp_naevr<AdvisoryPackage>);

    return out;
}

}  // namespace libdnf5::advisory
