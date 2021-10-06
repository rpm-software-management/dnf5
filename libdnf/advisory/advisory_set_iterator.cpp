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


#include "libdnf/advisory/advisory_set_impl.hpp"
#include "libdnf/advisory/advisory_set_iterator.hpp"
#include "libdnf/solv/solv_map.hpp"


namespace libdnf::advisory {

class AdvisorySetIterator::Impl : public libdnf::solv::SolvMap::iterator {
public:
    Impl(const AdvisorySet & advisory_set)
    : libdnf::solv::SolvMap::iterator(advisory_set.p_impl->get_map()),
      advisory_set{advisory_set} {}

    Impl(const AdvisorySetIterator::Impl & advisory_set_iterator_impl) = default;

    AdvisorySetIterator::Impl & operator++() {
        libdnf::solv::SolvMap::iterator::operator++();
        return *this;
    }

private:
    friend AdvisorySetIterator;
    const AdvisorySet & advisory_set;
};


AdvisorySetIterator::AdvisorySetIterator(const AdvisorySet & advisory_set) : p_impl{new Impl(advisory_set)} {}

AdvisorySetIterator::AdvisorySetIterator(const AdvisorySetIterator & other) : p_impl{new Impl(*other.p_impl)} {}

AdvisorySetIterator::~AdvisorySetIterator() {}

void AdvisorySetIterator::begin() {
    p_impl->begin();
}


void AdvisorySetIterator::end() {
    p_impl->end();
}


Advisory AdvisorySetIterator::operator*() {
    return {p_impl->advisory_set.get_base(), libdnf::advisory::AdvisoryId(**p_impl)};
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


}  // namespace libdnf::advisory
