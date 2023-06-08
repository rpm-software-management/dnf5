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

#include "libdnf/advisory/advisory_reference.hpp"

#include "rpm/package_sack_impl.hpp"

#include <solv/chksum.h>
#include <solv/repo.h>
#include <solv/util.h>

namespace libdnf5::advisory {

AdvisoryReference::AdvisoryReference(const libdnf::BaseWeakPtr & base, AdvisoryId advisory, int index)
    : base(base),
      advisory(advisory),
      index(index) {}

std::string AdvisoryReference::get_id() const {
    return std::string(get_rpm_pool(base).get_str_from_pool(UPDATE_REFERENCE_ID, advisory.id, index));
}
std::string AdvisoryReference::get_type() const {
    return std::string(get_rpm_pool(base).get_str_from_pool(UPDATE_REFERENCE_TYPE, advisory.id, index));
}
const char * AdvisoryReference::get_type_cstring() const {
    return get_rpm_pool(base).get_str_from_pool(UPDATE_REFERENCE_TYPE, advisory.id, index);
}
std::string AdvisoryReference::get_title() const {
    return std::string(get_rpm_pool(base).get_str_from_pool(UPDATE_REFERENCE_TITLE, advisory.id, index));
}
std::string AdvisoryReference::get_url() const {
    return std::string(get_rpm_pool(base).get_str_from_pool(UPDATE_REFERENCE_HREF, advisory.id, index));
}

}  // namespace libdnf5::advisory
