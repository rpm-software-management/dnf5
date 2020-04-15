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

// libsolv
extern "C" {
#include <solv/dataiterator.h>
#include <solv/queue.h>
}

#include "libdnf/rpm/reldep_list.hpp"
#include "libdnf/rpm/reldep.hpp"
#include "reldep_splitter.hpp"
#include "sack-impl.hpp"
#include "solv/id_queue.hpp"

namespace libdnf::rpm {

class ReldepList::Impl {
public:
    Impl(const ReldepList::Impl & src);
    Impl(Sack * sack);
    Impl(Sack * sack, libdnf::rpm::solv::IdQueue queue_src);
    ~Impl();

private:
    friend class ReldepList;
    Sack * sack;
    libdnf::rpm::solv::IdQueue queue;
};

ReldepList::Impl::Impl(const ReldepList::Impl & src)
        : sack(src.sack), queue(src.queue)
{}

ReldepList::Impl::Impl(Sack * sack)
        : sack(sack)
{}

ReldepList::Impl::Impl(Sack * sack, libdnf::rpm::solv::IdQueue queue_src)
        : sack(sack), queue(queue_src)
{}

ReldepList::Impl::~Impl()
{}

ReldepList::ReldepList(const ReldepList & src)
        : pImpl(new Impl(*src.pImpl))
{}


ReldepList::ReldepList(Sack * sack)
        : pImpl(new Impl(sack))
{}

ReldepList::~ReldepList() = default;

ReldepId ReldepList::get_id(int index) const noexcept
{
    return pImpl->queue[index];
}


ReldepList &ReldepList::operator=(ReldepList && src) noexcept
{
    // TODO Use move
    pImpl->sack = src.pImpl->sack;
    queue_init_clone(pImpl->queue.get_queue(), src.pImpl->queue.get_queue());
    return *this;
}

bool ReldepList::operator!=(const ReldepList &r) const { return !(*this == r); }
bool ReldepList::operator==(const ReldepList &r) const
{
    auto & this_queue = pImpl->queue;
    auto & other_queue = r.pImpl->queue;
    auto this_count = this_queue.size();
    if (this_count != other_queue.size())
        return false;

    for (int i = 0; i < this_count; i++) {
        if (this_queue[i] != other_queue[i]) {
            return false;
        }
    }

    return pImpl->sack->pImpl->pool == r.pImpl->sack->pImpl->pool;
}

void ReldepList::add(Reldep & reldep)
{
    pImpl->queue.push_back(reldep.id.id);
}

void ReldepList::add(ReldepId id)
{
    pImpl->queue.push_back(id.id);
}

bool ReldepList::add_reldep_with_glob(const std::string & reldep_str)
{
    ReldepSplitter dep_splitter;
    if(!dep_splitter.parse(reldep_str))
        return false;
    Dataiterator di;
    Pool *pool = pImpl->sack->pImpl->pool;

    dataiterator_init(&di, pool, 0, 0, 0, dep_splitter.get_name_cstr(),
                      SEARCH_STRING | SEARCH_GLOB);
    while (dataiterator_step(&di)) {
        ReldepId id = Reldep::get_reldep_id(pImpl->sack, di.kv.str, dep_splitter.get_evr_cstr(),
                                            dep_splitter.get_cmp_type());
        add(id);
    }
    dataiterator_free(&di);
    return true;
}

bool ReldepList::add_reldep(const std::string & reldep_str)
{
    try {
        ReldepId id = Reldep::get_reldep_id(pImpl->sack, reldep_str);
        add(id);
        return true;
    }
    catch (...) {
        return false;
    }
}

void ReldepList::insert(ReldepList & source)
{
    pImpl->queue.insert(source.pImpl->queue);
}

Reldep ReldepList::get(int index) const noexcept
{
    ReldepId id(pImpl->queue[index]);
    return Reldep(pImpl->sack, id);
}

int ReldepList::size() const noexcept { return pImpl->queue.size(); }

}  // namespace libdnf::rpm
