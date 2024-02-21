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

#include "libdnf5/advisory/advisory_reference.hpp"

#include "rpm/package_sack_impl.hpp"

#include <solv/chksum.h>
#include <solv/repo.h>
#include <solv/util.h>

namespace libdnf5::advisory {

class AdvisoryReference::Impl {
public:
    explicit Impl(const libdnf5::BaseWeakPtr & base, AdvisoryId advisory, int index)
        : base(base),
          advisory(advisory),
          index(index) {}

private:
    friend AdvisoryReference;

    BaseWeakPtr base;

    AdvisoryId advisory;

    /// We cannot store IDs of reference data (id, type, title, url) because they
    /// don't have ids set in libsolv (they are only strings), therefore we store
    /// index of the reference.
    int index;
};

AdvisoryReference::AdvisoryReference(const libdnf5::BaseWeakPtr & base, AdvisoryId advisory, int index)
    : p_impl(std::make_unique<Impl>(base, advisory, index)) {}

AdvisoryReference::~AdvisoryReference() = default;

AdvisoryReference::AdvisoryReference(const AdvisoryReference & src) : p_impl(new Impl(*src.p_impl)) {}
AdvisoryReference & AdvisoryReference::operator=(const AdvisoryReference & src) {
    if (this != &src) {
        if (p_impl) {
            *p_impl = *src.p_impl;
        } else {
            p_impl = std::make_unique<Impl>(*src.p_impl);
        }
    }

    return *this;
}

AdvisoryReference::AdvisoryReference(AdvisoryReference && src) noexcept = default;
AdvisoryReference & AdvisoryReference::operator=(AdvisoryReference && src) noexcept = default;

std::string AdvisoryReference::get_id() const {
    return std::string(
        get_rpm_pool(p_impl->base).get_str_from_pool(UPDATE_REFERENCE_ID, p_impl->advisory.id, p_impl->index));
}
std::string AdvisoryReference::get_type() const {
    return std::string(
        get_rpm_pool(p_impl->base).get_str_from_pool(UPDATE_REFERENCE_TYPE, p_impl->advisory.id, p_impl->index));
}
const char * AdvisoryReference::get_type_cstring() const {
    return get_rpm_pool(p_impl->base).get_str_from_pool(UPDATE_REFERENCE_TYPE, p_impl->advisory.id, p_impl->index);
}
std::string AdvisoryReference::get_title() const {
    return std::string(
        get_rpm_pool(p_impl->base).get_str_from_pool(UPDATE_REFERENCE_TITLE, p_impl->advisory.id, p_impl->index));
}
std::string AdvisoryReference::get_url() const {
    return std::string(
        get_rpm_pool(p_impl->base).get_str_from_pool(UPDATE_REFERENCE_HREF, p_impl->advisory.id, p_impl->index));
}

}  // namespace libdnf5::advisory
