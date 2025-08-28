// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef LIBDNF5_BASE_BASE_WEAK_HPP
#define LIBDNF5_BASE_BASE_WEAK_HPP

#include "libdnf5/common/weak_ptr.hpp"


namespace libdnf5 {

class Base;
using BaseWeakPtr = WeakPtr<Base, false>;

}  // namespace libdnf5

#endif
