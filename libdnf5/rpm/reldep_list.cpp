// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 2.1 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.

#include "libdnf5/rpm/reldep_list.hpp"

#include "../base/base_private.hpp"
#include "package_sack_impl.hpp"
#include "reldep_list_impl.hpp"
#include "solv/reldep_parser.hpp"

#include "libdnf5/rpm/reldep.hpp"

// libsolv
extern "C" {
#include <solv/dataiterator.h>
#include <solv/queue.h>
}


namespace libdnf5::rpm {

ReldepList::ReldepList(const ReldepList & src) : p_impl(new Impl(*src.p_impl)) {}

ReldepList::ReldepList(ReldepList && src) noexcept : p_impl(std::move(src.p_impl)) {}

ReldepList::ReldepList(const BaseWeakPtr & base) : p_impl(new Impl(base)) {}

ReldepList::ReldepList(libdnf5::Base & base) : ReldepList(base.get_weak_ptr()) {}

ReldepList::~ReldepList() = default;

ReldepId ReldepList::get_id(int index) const noexcept {
    return ReldepId(p_impl->queue[index]);
}

ReldepList & ReldepList::operator=(ReldepList && src) noexcept {
    p_impl.swap(src.p_impl);
    return *this;
}

bool ReldepList::operator!=(const ReldepList & other) const noexcept {
    return !(*this == other);
}
bool ReldepList::operator==(const ReldepList & other) const noexcept {
    auto & this_queue = p_impl->queue;
    auto & other_queue = other.p_impl->queue;
    auto this_count = this_queue.size();
    if (this_count != other_queue.size())
        return false;

    for (int i = 0; i < this_count; i++) {
        if (this_queue[i] != other_queue[i]) {
            return false;
        }
    }

    return *get_rpm_pool(p_impl->base) == *get_rpm_pool(other.p_impl->base);
}

ReldepList & ReldepList::operator=(const ReldepList & src) {
    p_impl->queue = src.p_impl->queue;
    p_impl->base = src.p_impl->base;
    return *this;
}

void ReldepList::add(const Reldep & reldep) {
    libdnf_assert_same_base(p_impl->base, reldep.get_base());
    p_impl->queue.push_back(reldep.get_id().id);
}

void ReldepList::add(ReldepId id) {
    p_impl->queue.push_back(id.id);
}

bool ReldepList::add_reldep_with_glob(const std::string & reldep_str) {
    libdnf5::solv::ReldepParser dep_splitter;
    if (!dep_splitter.parse(reldep_str))
        return false;

    Dataiterator di;
    dataiterator_init(
        &di, *get_rpm_pool(p_impl->base), 0, 0, 0, dep_splitter.get_name_cstr(), SEARCH_STRING | SEARCH_GLOB);
    while (dataiterator_step(&di)) {
        switch (di.key->name) {
            case SOLVABLE_PROVIDES:
            case SOLVABLE_OBSOLETES:
            case SOLVABLE_CONFLICTS:
            case SOLVABLE_REQUIRES:
            case SOLVABLE_RECOMMENDS:
            case SOLVABLE_SUGGESTS:
            case SOLVABLE_SUPPLEMENTS:
            case SOLVABLE_ENHANCES:
            case SOLVABLE_FILELIST:
                add(Reldep::get_reldep_id(
                    p_impl->base, di.kv.str, dep_splitter.get_evr_cstr(), dep_splitter.get_cmp_type()));
        }
    }
    dataiterator_free(&di);
    return true;
}

bool ReldepList::add_reldep(const std::string & reldep_str, int create) {
    try {
        ReldepId id = Reldep::get_reldep_id(p_impl->base, reldep_str, create);
        if (id.id == 0) {
            return false;
        }
        add(id);
        return true;
        // TODO(jmracek) Make catch error more specific
    } catch (...) {
        return false;
    }
}

bool ReldepList::add_reldep(const std::string & reldep_str) {
    return add_reldep(reldep_str, 1);
}

void ReldepList::append(ReldepList & source) {
    libdnf_assert_same_base(p_impl->base, source.get_base());
    p_impl->queue += source.p_impl->queue;
}

Reldep ReldepList::get(int index) const noexcept {
    ReldepId id(p_impl->queue[index]);
    return Reldep(p_impl->base, id);
}

int ReldepList::size() const noexcept {
    return p_impl->queue.size();
}

bool ReldepList::empty() const noexcept {
    return p_impl->queue.size() == 0;
}

void ReldepList::clear() {
    p_impl->queue.clear();
}

ReldepList::iterator ReldepList::begin() const {
    ReldepList::iterator it(*this);
    it.begin();
    return it;
}
ReldepList::iterator ReldepList::end() const {
    ReldepList::iterator it(*this);
    it.end();
    return it;
}

BaseWeakPtr ReldepList::get_base() const {
    return p_impl->get_base();
}

}  // namespace libdnf5::rpm
