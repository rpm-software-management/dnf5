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

#ifndef LIBDNF_MODULE_MODULE_ITEM_CONTAINER_IMPL_HPP
#define LIBDNF_MODULE_MODULE_ITEM_CONTAINER_IMPL_HPP

#include "libdnf/module/module_item_container.hpp"

extern "C" {
#include <solv/pool.h>
}

namespace libdnf::module {


class ModuleItemContainer::Impl {
public:
    Impl(const BaseWeakPtr & base) : base(base), pool(pool_create()) {}
    ~Impl() { pool_free(pool); }

private:
    friend ModuleItemContainer;
    friend ModuleItem;

    BaseWeakPtr base;
    std::vector<std::unique_ptr<ModuleItem>> modules;
    Pool * pool;
    // Repositories containing any modules. Key is repoid, value is Id of the repo in libsolv.
    // This is needed in `ModuleItem::create_solvable` for creating solvable in the correct repository.
    std::map<std::string, Id> repositories;
};


}  // namespace libdnf::module

#endif  // LIBDNF_MODULE_MODULE_ITEM_CONTAINER_IMPL_HPP
