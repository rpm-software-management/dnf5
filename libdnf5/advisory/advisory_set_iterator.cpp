// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later


#include "libdnf5/advisory/advisory_set_iterator.hpp"

#include "advisory_set_impl.hpp"
#include "solv/solv_map.hpp"


namespace libdnf5::advisory {

class AdvisorySetIterator::Impl : private libdnf5::solv::SolvMap::iterator {
private:
    Impl(const AdvisorySet & advisory_set)
        : libdnf5::solv::SolvMap::iterator(advisory_set.p_impl->get_map()),
          advisory_set{&advisory_set} {}

    AdvisorySetIterator::Impl & operator++() {
        libdnf5::solv::SolvMap::iterator::operator++();
        return *this;
    }

    const AdvisorySet * advisory_set;

    friend AdvisorySetIterator;
};


AdvisorySetIterator::AdvisorySetIterator(const AdvisorySet & advisory_set) : p_impl{new Impl(advisory_set)} {}

AdvisorySetIterator::AdvisorySetIterator(const AdvisorySetIterator & other) : p_impl{new Impl(*other.p_impl)} {}

AdvisorySetIterator::~AdvisorySetIterator() = default;


AdvisorySetIterator & AdvisorySetIterator::operator=(const AdvisorySetIterator & other) {
    *p_impl = *other.p_impl;
    return *this;
}


AdvisorySetIterator AdvisorySetIterator::begin(const AdvisorySet & advisory_set) {
    AdvisorySetIterator it(advisory_set);
    it.begin();
    return it;
}


AdvisorySetIterator AdvisorySetIterator::end(const AdvisorySet & advisory_set) {
    AdvisorySetIterator it(advisory_set);
    it.end();
    return it;
}

void AdvisorySetIterator::begin() {
    p_impl->begin();
}


void AdvisorySetIterator::end() {
    p_impl->end();
}


Advisory AdvisorySetIterator::operator*() {
    return {p_impl->advisory_set->get_base(), libdnf5::advisory::AdvisoryId(**p_impl)};
}


AdvisorySetIterator & AdvisorySetIterator::operator++() {
    ++*p_impl;
    return *this;
}


AdvisorySetIterator AdvisorySetIterator::operator++(int) {
    AdvisorySetIterator it(*this);
    ++*p_impl;
    return it;
}


bool AdvisorySetIterator::operator==(const AdvisorySetIterator & other) const {
    return *p_impl == *other.p_impl;
}


bool AdvisorySetIterator::operator!=(const AdvisorySetIterator & other) const {
    return *p_impl != *other.p_impl;
}


}  // namespace libdnf5::advisory
