// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef LIBDNF5_ADVISORY_ADVISORY_MODULE_PRIVATE_HPP
#define LIBDNF5_ADVISORY_ADVISORY_MODULE_PRIVATE_HPP

#include "libdnf5/advisory/advisory.hpp"
#include "libdnf5/advisory/advisory_module.hpp"

#include <solv/pooltypes.h>

namespace libdnf5::advisory {

//TODO(amatej): We might want to remove the Impl idiom to speed up iterating over
//              AdvisoryModules (less classes, less overhead), but we would end
//              up with libsolv ints (Ids) in a public header.
class AdvisoryModule::Impl {
private:
    friend class AdvisoryCollection;
    friend AdvisoryModule;

    explicit Impl(
        const libdnf5::BaseWeakPtr & base,
        AdvisoryId advisory,
        int owner_collection_index,
        Id name,
        Id stream,
        Id version,
        Id context,
        Id arch);

    libdnf5::BaseWeakPtr base;
    AdvisoryId advisory;
    int owner_collection_index;

    Id name;
    Id stream;
    Id version;
    Id context;
    Id arch;
};

}  // namespace libdnf5::advisory


#endif  // LIBDNF5_ADVISORY_ADVISORY_MODULE_PRIVATE_HPP
