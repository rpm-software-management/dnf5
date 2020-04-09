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
}

#include "DependencyContainer.hpp"
#include "Dependency.hpp"
#include "../DependencySplitter.hpp"

namespace libdnf {

DependencyContainer::DependencyContainer(const DependencyContainer &src)
        : sack(src.sack)
{
    queue_init_clone(&this->queue, &queue);
}


DependencyContainer::DependencyContainer(DnfSack *sack)
        : sack(sack)
{
    queue_init(&queue);
}

DependencyContainer::DependencyContainer(DnfSack *sack, Queue queue)
        : sack(sack)
{
    queue_init_clone(&this->queue, &queue);
}

DependencyContainer::~DependencyContainer()
{
    queue_free(&queue);
}

DependencyContainer &DependencyContainer::operator=(DependencyContainer &&src) noexcept
{
    sack = src.sack;
    queue_init_clone(&queue, &src.queue);
    return *this;
}

bool DependencyContainer::operator!=(const DependencyContainer &r) const { return !(*this == r); }
bool DependencyContainer::operator==(const DependencyContainer &r) const
{
    if (queue.count != r.queue.count)
        return false;

    for (int i = 0; i < queue.count; i++) {
        if (queue.elements[i] != r.queue.elements[i]) {
            return false;
        }
    }

    return dnf_sack_get_pool(sack) == dnf_sack_get_pool(r.sack);
}

void DependencyContainer::add(Dependency *dependency)
{
    queue_push(&queue, dependency->getId());
}

void DependencyContainer::add(Id id)
{
    queue_push(&queue, id);
}

bool DependencyContainer::addReldepWithGlob(const char *reldepStr)
{
    DependencySplitter depSplitter;
    if(!depSplitter.parse(reldepStr))
        return false;
    Dataiterator di;
    Pool *pool = dnf_sack_get_pool(sack);

    dataiterator_init(&di, pool, 0, 0, 0, depSplitter.getNameCStr(),
                      SEARCH_STRING | SEARCH_GLOB);
    while (dataiterator_step(&di)) {
        Id id = Dependency::getReldepId(sack, di.kv.str, depSplitter.getEVRCStr(),
                                        depSplitter.getCmpType());
        add(id);
    }
    dataiterator_free(&di);
    return true;
}

bool DependencyContainer::addReldep(const char *reldepStr)
{
    try {
        Id id = Dependency::getReldepId(sack, reldepStr);
        add(id);
        return true;
    }
    catch (...) {
        return false;
    }
}

void DependencyContainer::extend(DependencyContainer *container)
{
    queue_insertn(&queue, 0, container->queue.count, container->queue.elements);
}

std::unique_ptr<Dependency> DependencyContainer::get(int index) const noexcept
{
    Id id = queue.elements[index];
    return std::unique_ptr<Dependency> (new Dependency(sack, id));
}

Dependency *DependencyContainer::getPtr(int index) const noexcept
{
    Id id = queue.elements[index];
    return new Dependency(sack, id);
}

int DependencyContainer::count() const noexcept { return queue.count; }
const Queue &DependencyContainer::getQueue() const noexcept { return queue; }

}
