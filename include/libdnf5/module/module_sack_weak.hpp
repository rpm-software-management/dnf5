// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef LIBDNF5_MODULE_MODULE_SACK_WEAK_HPP
#define LIBDNF5_MODULE_MODULE_SACK_WEAK_HPP

#include "libdnf5/common/weak_ptr.hpp"


namespace libdnf5::module {


class ModuleSack;
using ModuleSackWeakPtr = WeakPtr<ModuleSack, false>;


}  // namespace libdnf5::module


#endif  // LIBDNF5_MODULE_MODULE_SACK_WEAK_HPP
