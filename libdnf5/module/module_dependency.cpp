// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "libdnf5/module/module_dependency.hpp"

#include "utils/string.hpp"

#include <modulemd-2.0/modulemd-module-stream.h>
#include <modulemd-2.0/modulemd-profile.h>
#include <modulemd-2.0/modulemd.h>

extern "C" {
#include <solv/pool.h>
#include <solv/pool_parserpmrichdep.h>
#include <solv/repo.h>
}

#include <algorithm>
#include <string>
#include <vector>

namespace libdnf5::module {

class ModuleDependency::Impl {
public:
    Impl(const std::string & module_name, const std::vector<std::string> & streams)
        : module_name(module_name),
          streams(streams) {}

    Impl(std::string && module_name, std::vector<std::string> && streams)
        : module_name(std::move(module_name)),
          streams(std::move(streams)) {}

private:
    friend ModuleDependency;

    std::string module_name;
    std::vector<std::string> streams;
};

ModuleDependency::ModuleDependency(const std::string & module_name, const std::vector<std::string> & streams)
    : p_impl(std::make_unique<Impl>(module_name, streams)) {}

ModuleDependency::ModuleDependency(std::string && module_name, std::vector<std::string> && streams)
    : p_impl(std::make_unique<Impl>(std::move(module_name), std::move(streams))) {}

ModuleDependency::~ModuleDependency() = default;

ModuleDependency::ModuleDependency(const ModuleDependency & src) : p_impl(new Impl(*src.p_impl)) {}
ModuleDependency::ModuleDependency(ModuleDependency && src) noexcept = default;

ModuleDependency & ModuleDependency::operator=(const ModuleDependency & src) {
    if (this != &src) {
        if (p_impl) {
            *p_impl = *src.p_impl;
        } else {
            p_impl = std::make_unique<Impl>(*src.p_impl);
        }
    }

    return *this;
}
ModuleDependency & ModuleDependency::operator=(ModuleDependency && src) noexcept = default;

const std::string & ModuleDependency::get_module_name() const {
    return p_impl->module_name;
};

const std::vector<std::string> & ModuleDependency::get_streams() const {
    return p_impl->streams;
};

std::string ModuleDependency::to_string() {
    std::sort(p_impl->streams.begin(), p_impl->streams.end());
    return fmt::format("{}:[{}]", p_impl->module_name, utils::string::join(p_impl->streams, ","));
}


}  // namespace libdnf5::module
