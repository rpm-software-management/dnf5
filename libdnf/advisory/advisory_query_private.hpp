/*
Copyright (C) 2021 Red Hat, Inc.

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

#ifndef LIBDNF_ADVISORY_ADVISORY_QUERY_PRIVATE_HPP
#define LIBDNF_ADVISORY_ADVISORY_QUERY_PRIVATE_HPP

#include "libdnf/advisory/advisory.hpp"
#include "libdnf/advisory/advisory_query.hpp"
#include "libdnf/rpm/solv/solv_map.hpp"

namespace libdnf::advisory {

//TODO(amatej): only temprary class to hide solv_map from advisory_query.hpp,
//              should be replaced most likely by AdvisorySet?
class AdvisoryQuery::Impl {
    Impl(libdnf::rpm::solv::SolvMap m) : data_map(m){};

private:
    friend AdvisoryQuery;

    libdnf::rpm::solv::SolvMap data_map;
};

}  // namespace libdnf::advisory


#endif  // LIBDNF_ADVISORY_ADVISORY_QUERY_PRIVATE_HPP
