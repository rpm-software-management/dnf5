/*
Copyright (C) 2018-2020 Red Hat, Inc.

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

#include "libdnf/rpm/reldep_list.hpp"

#include "reldep_list_impl.hpp"
#include "solv/reldep_parser.hpp"
#include "solv_sack_impl.hpp"

#include "libdnf/rpm/reldep.hpp"

// libsolv
extern "C" {
#include <solv/dataiterator.h>
#include <solv/queue.h>
}


namespace libdnf::rpm {

ReldepList::ReldepList(const ReldepList & src) : pImpl(new Impl(*src.pImpl)) {}

ReldepList::ReldepList(ReldepList && src) noexcept : pImpl(std::move(src.pImpl)) {}

ReldepList::ReldepList(SolvSack * sack) : pImpl(new Impl(sack)) {}

ReldepList::~ReldepList() = default;

ReldepId ReldepList::get_id(int index) const noexcept {
    return ReldepId(pImpl->queue[index]);
}

ReldepList & ReldepList::operator=(ReldepList && src) noexcept {
    pImpl.swap(src.pImpl);
    return *this;
}

bool ReldepList::operator!=(const ReldepList & other) const noexcept {
    return !(*this == other);
}
bool ReldepList::operator==(const ReldepList & other) const noexcept {
    auto & this_queue = pImpl->queue;
    auto & other_queue = other.pImpl->queue;
    auto this_count = this_queue.size();
    if (this_count != other_queue.size())
        return false;

    for (int i = 0; i < this_count; i++) {
        if (this_queue[i] != other_queue[i]) {
            return false;
        }
    }

    return pImpl->sack->pImpl->pool == other.pImpl->sack->pImpl->pool;
}

ReldepList & ReldepList::operator=(const ReldepList & src) {
    pImpl->queue = src.pImpl->queue;
    pImpl->sack = src.pImpl->sack;
    return *this;
}

void ReldepList::add(Reldep & reldep) {
    pImpl->queue.push_back(reldep.id.id);
}

void ReldepList::add(ReldepId id) {
    pImpl->queue.push_back(id.id);
}

bool ReldepList::add_reldep_with_glob(const std::string & reldep_str) {
    solv::ReldepParser dep_splitter;
    if (!dep_splitter.parse(reldep_str))
        return false;

    auto * sack = pImpl->sack.get();
    Pool * pool = sack->pImpl->pool;

    Dataiterator di;
    dataiterator_init(&di, pool, 0, 0, 0, dep_splitter.get_name_cstr(), SEARCH_STRING | SEARCH_GLOB);
    while (dataiterator_step(&di)) {
        ReldepId id = Reldep::get_reldep_id(sack, di.kv.str, dep_splitter.get_evr_cstr(), dep_splitter.get_cmp_type());
        add(id);
    }
    dataiterator_free(&di);
    return true;
}

bool ReldepList::add_reldep(const std::string & reldep_str) {
    try {
        ReldepId id = Reldep::get_reldep_id(pImpl->sack.get(), reldep_str);
        add(id);
        return true;
        // TODO(jmracek) Make catch error more specific
    } catch (...) {
        return false;
    }
}

void ReldepList::append(ReldepList & source) {
    pImpl->queue.append(source.pImpl->queue);
}

Reldep ReldepList::get(int index) const noexcept {
    ReldepId id(pImpl->queue[index]);
    return Reldep(pImpl->sack.get(), id);
}

int ReldepList::size() const noexcept {
    return pImpl->queue.size();
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

SolvSack * ReldepList::get_sack() const {
    return pImpl->get_sack();
}

}  // namespace libdnf::rpm
