// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "libdnf5/advisory/advisory_module.hpp"

#include "advisory_module_private.hpp"
#include "solv/pool.hpp"


namespace libdnf5::advisory {

// AdvisoryModule
AdvisoryModule::AdvisoryModule(AdvisoryModule::Impl * private_module) : p_impl(private_module) {}

AdvisoryModule::AdvisoryModule(const AdvisoryModule & src) = default;
AdvisoryModule::AdvisoryModule(AdvisoryModule && src) noexcept = default;
AdvisoryModule & AdvisoryModule::operator=(const AdvisoryModule & src) = default;
AdvisoryModule & AdvisoryModule::operator=(AdvisoryModule && src) noexcept = default;
AdvisoryModule::~AdvisoryModule() = default;

std::string AdvisoryModule::get_name() const {
    return get_rpm_pool(p_impl->base).id2str(p_impl->name);
}

std::string AdvisoryModule::get_stream() const {
    return get_rpm_pool(p_impl->base).id2str(p_impl->stream);
}
std::string AdvisoryModule::get_version() const {
    return get_rpm_pool(p_impl->base).id2str(p_impl->version);
}
std::string AdvisoryModule::get_context() const {
    return get_rpm_pool(p_impl->base).id2str(p_impl->context);
}
std::string AdvisoryModule::get_arch() const {
    return get_rpm_pool(p_impl->base).id2str(p_impl->arch);
}
std::string AdvisoryModule::get_nsvca() const {
    return std::string(get_name() + ":" + get_stream() + ":" + get_version() + ":" + get_context() + ":" + get_arch());
}

AdvisoryId AdvisoryModule::get_advisory_id() const {
    return p_impl->advisory;
}
Advisory AdvisoryModule::get_advisory() const {
    return Advisory(p_impl->base, p_impl->advisory);
}
AdvisoryCollection AdvisoryModule::get_advisory_collection() const {
    return AdvisoryCollection(p_impl->base, p_impl->advisory, p_impl->owner_collection_index);
}

// AdvisoryModule::Impl
AdvisoryModule::Impl::Impl(
    const libdnf5::BaseWeakPtr & base,
    AdvisoryId advisory,
    int owner_collection_index,
    Id name,
    Id stream,
    Id version,
    Id context,
    Id arch)
    : base(base),
      advisory(advisory),
      owner_collection_index(owner_collection_index),
      name(name),
      stream(stream),
      version(version),
      context(context),
      arch(arch) {}

}  // namespace libdnf5::advisory
