// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef LIBDNF5_REPO_REPO_WEAK_HPP
#define LIBDNF5_REPO_REPO_WEAK_HPP

#include "libdnf5/common/weak_ptr.hpp"


namespace libdnf5::repo {

class Repo;
using RepoWeakPtr = WeakPtr<Repo, false>;

}  // namespace libdnf5::repo

#endif
