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

#ifndef LIBDNF_ADVISORY_ADVISORY_MODULE_PRIVATE_HPP
#define LIBDNF_ADVISORY_ADVISORY_MODULE_PRIVATE_HPP

#include "libdnf/advisory/advisory.hpp"
#include "libdnf/advisory/advisory_module.hpp"

#include <solv/pooltypes.h>

namespace libdnf::advisory {

//TODO(amatej): We might want to remove the Impl idiom to speed up iterating over
//              AdvisoryModules (less classes, less overhead), but we would end
//              up with libsolv ints (Ids) in a public header.
class AdvisoryModule::Impl {
private:
    friend class AdvisoryCollection;
    friend AdvisoryModule;

    explicit Impl(
        const libdnf::BaseWeakPtr & base,
        AdvisoryId advisory,
        int owner_collection_index,
        Id name,
        Id stream,
        Id version,
        Id context,
        Id arch);

    libdnf::BaseWeakPtr base;
    AdvisoryId advisory;
    int owner_collection_index;

    Id name;
    Id stream;
    Id version;
    Id context;
    Id arch;
};

}  // namespace libdnf::advisory


#endif  // LIBDNF_ADVISORY_ADVISORY_MODULE_PRIVATE_HPP
