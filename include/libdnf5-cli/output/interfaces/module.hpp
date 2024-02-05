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


#ifndef LIBDNF5_CLI_OUTPUT_INTERFACES_MODULE_HPP
#define LIBDNF5_CLI_OUTPUT_INTERFACES_MODULE_HPP

#include <libdnf5/module/module_status.hpp>

#include <memory>
#include <string>
#include <vector>

namespace libdnf5::cli::output {

class IModuleDependency {
public:
    virtual ~IModuleDependency() = default;

    virtual std::string to_string() = 0;
};


class IModuleProfile {
public:
    virtual ~IModuleProfile() = default;

    virtual std::string get_name() const = 0;
    virtual std::string get_description() const = 0;
    virtual std::vector<std::string> get_rpms() const = 0;
    virtual bool is_default() const = 0;
};


class IModuleItem {
public:
    virtual ~IModuleItem() = default;

    virtual std::string get_name() const = 0;
    virtual std::string get_stream() const = 0;
    virtual long long get_version() const = 0;
    virtual std::string get_version_str() const = 0;
    virtual std::string get_context() const = 0;
    virtual std::string get_arch() const = 0;
    virtual std::string get_full_identifier() const = 0;
    virtual std::string get_summary() const = 0;
    virtual std::string get_description() const = 0;
    virtual std::vector<std::string> get_artifacts() const = 0;
    virtual std::vector<std::unique_ptr<IModuleProfile>> get_profiles() const = 0;
    virtual std::vector<std::string> get_default_profiles() const = 0;
    virtual std::vector<std::unique_ptr<IModuleDependency>> get_module_dependencies(
        bool remove_platform = false) const = 0;
    virtual std::string get_repo_id() const = 0;
    virtual bool is_active() const = 0;
    virtual module::ModuleStatus get_status() const = 0;
    virtual bool is_default() const = 0;
};

}  // namespace libdnf5::cli::output

#endif  // LIBDNF5_CLI_OUTPUT_INTERFACES_MODULE_HPP
