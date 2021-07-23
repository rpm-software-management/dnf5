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

#include "libdnf/rpm/reldep_list.hpp"

#include "package_sack_impl.hpp"
#include "reldep_list_impl.hpp"
#include "libdnf/solv/reldep_parser.hpp"

#include "libdnf/rpm/reldep.hpp"

// libsolv
extern "C" {
#include <solv/dataiterator.h>
#include <solv/queue.h>
}


namespace libdnf::rpm {

ReldepList::ReldepList(const ReldepList & src) : p_impl(new Impl(*src.p_impl)) {}

ReldepList::ReldepList(ReldepList && src) noexcept : p_impl(std::move(src.p_impl)) {}

ReldepList::ReldepList(const PackageSackWeakPtr & sack) : p_impl(new Impl(sack)) {}

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

    return p_impl->sack->p_impl->pool == other.p_impl->sack->p_impl->pool;
}

ReldepList & ReldepList::operator=(const ReldepList & src) {
    p_impl->queue = src.p_impl->queue;
    p_impl->sack = src.p_impl->sack;
    return *this;
}

void ReldepList::add(Reldep & reldep) {
    p_impl->queue.push_back(reldep.id.id);
}

void ReldepList::add(ReldepId id) {
    p_impl->queue.push_back(id.id);
}

bool ReldepList::add_reldep_with_glob(const std::string & reldep_str) {
    libdnf::solv::ReldepParser dep_splitter;
    if (!dep_splitter.parse(reldep_str))
        return false;

    auto * sack = p_impl->sack.get();
    Pool * pool = sack->p_impl->pool;

    Dataiterator di;
    dataiterator_init(&di, pool, 0, 0, 0, dep_splitter.get_name_cstr(), SEARCH_STRING | SEARCH_GLOB);
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
                add(Reldep::get_reldep_id(sack, di.kv.str, dep_splitter.get_evr_cstr(), dep_splitter.get_cmp_type()));
        }
    }
    dataiterator_free(&di);
    return true;
}

bool ReldepList::add_reldep(const std::string & reldep_str, int create) {
    try {
        ReldepId id = Reldep::get_reldep_id(p_impl->sack.get(), reldep_str, create);
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

void ReldepList::append(ReldepList & source) {
    p_impl->queue += source.p_impl->queue;
}

Reldep ReldepList::get(int index) const noexcept {
    ReldepId id(p_impl->queue[index]);
    return Reldep(p_impl->sack.get(), id);
}

int ReldepList::size() const noexcept {
    return p_impl->queue.size();
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

PackageSack * ReldepList::get_sack() const {
    return p_impl->get_sack();
}

}  // namespace libdnf::rpm
