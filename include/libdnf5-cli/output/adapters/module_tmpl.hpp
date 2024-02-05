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


#ifndef LIBDNF5_CLI_OUTPUT_ADAPTERS_MODULE_TMPL_HPP
#define LIBDNF5_CLI_OUTPUT_ADAPTERS_MODULE_TMPL_HPP

#include "../interfaces/module.hpp"

namespace libdnf5::cli::output {

template <class T>
class ModuleDependencyAdapter : public IModuleDependency {
public:
    ModuleDependencyAdapter(const T & dep) : dep{dep} {}

    ModuleDependencyAdapter(T && dep) : dep{std::move(dep)} {}

    std::string to_string() { return dep.to_string(); }

private:
    T dep;
};


template <class T>
class ModuleProfileAdapter : public IModuleProfile {
public:
    ModuleProfileAdapter(const T & module_profile) : module_profile{module_profile} {}

    ModuleProfileAdapter(T && module_profile) : module_profile{std::move(module_profile)} {}

    std::string get_name() const override { return module_profile.get_name(); }

    std::string get_description() const override { return module_profile.get_description(); }

    std::vector<std::string> get_rpms() const override { return module_profile.get_rpms(); }

    bool is_default() const override { return module_profile.is_default(); }

private:
    T module_profile;
};


template <class T>
class ModuleItemAdapter : public IModuleItem {
public:
    ModuleItemAdapter(const T & pkg) : pkg{pkg} {}

    ModuleItemAdapter(T && pkg) : pkg{std::move(pkg)} {}

    std::string get_name() const override { return pkg.get_name(); }

    std::string get_stream() const override { return pkg.get_stream(); }

    long long get_version() const override { return pkg.get_version(); }

    std::string get_version_str() const override { return pkg.get_version_str(); }

    std::string get_context() const override { return pkg.get_context(); }

    std::string get_arch() const override { return pkg.get_arch(); }

    std::string get_full_identifier() const override { return pkg.get_full_identifier(); }

    std::string get_summary() const override { return pkg.get_summary(); }
    std::string get_description() const override { return pkg.get_description(); }

    std::vector<std::string> get_artifacts() const override { return pkg.get_artifacts(); }

    std::vector<std::unique_ptr<IModuleProfile>> get_profiles() const override {
        std::vector<std::unique_ptr<IModuleProfile>> ret;
        const auto & profiles = pkg.get_profiles();
        ret.reserve(profiles.size());
        for (const auto & profile : profiles) {
            ret.emplace_back(new ModuleProfileAdapter(profile));
        }
        return ret;
    }

    std::vector<std::string> get_default_profiles() const override { return pkg.get_default_profiles(); }

    std::vector<std::unique_ptr<IModuleDependency>> get_module_dependencies(bool remove_platform) const override {
        std::vector<std::unique_ptr<IModuleDependency>> ret;
        const auto & deps = pkg.get_module_dependencies(remove_platform);
        ret.reserve(deps.size());
        for (const auto & dep : deps) {
            ret.emplace_back(new ModuleDependencyAdapter(dep));
        }
        return ret;
    }

    std::string get_repo_id() const override { return pkg.get_repo_id(); }

    bool is_active() const override { return pkg.is_active(); }

    module::ModuleStatus get_status() const override { return pkg.get_status(); }

    bool is_default() const override { return pkg.is_default(); }

private:
    T pkg;
};

}  // namespace libdnf5::cli::output

#endif  // LIBDNF5_CLI_OUTPUT_ADAPTERS_MODULE_TMPL_HPP
