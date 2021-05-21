/*
Copyright (C) 2017-2020 Red Hat, Inc.

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

#ifndef LIBDNF_RPM_SOLV_SOLV_PRIVATE_HPP
#define LIBDNF_RPM_SOLV_SOLV_PRIVATE_HPP


#include "../package_sack_impl.hpp"
#include "../repo_impl.hpp"

namespace libdnf::rpm::solv {

class SolvPrivate {
public:
    static void internalize_libsolv_repo(LibsolvRepo * libsolv_repo);

private:
};

inline void SolvPrivate::internalize_libsolv_repo(LibsolvRepo * libsolv_repo) {
    PackageSack::Impl::internalize_libsolv_repo(libsolv_repo);
}

}  // namespace libdnf::rpm::solv

#endif  // LIBDNF_RPM_SOLV_SOLV_PRIVATE_HPP
