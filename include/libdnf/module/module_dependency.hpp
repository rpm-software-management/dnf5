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

#ifndef LIBDNF_MODULE_MODULE_DEPENDENCY_HPP
#define LIBDNF_MODULE_MODULE_DEPENDENCY_HPP

#include <string>
#include <vector>

namespace libdnf::module {


class ModuleDependency {
public:
    ModuleDependency(const std::string & module_name, const std::vector<std::string> & streams)
        : module_name(module_name),
          streams(streams) {}

    ModuleDependency(std::string && module_name, std::vector<std::string> && streams)
        : module_name(std::move(module_name)),
          streams(std::move(streams)) {}

    /// @return Name of the required module.
    /// @since 5.0
    const std::string & get_module_name() const { return module_name; };

    /// @return Vector of streams. Prefix '-' denotes conflicting stream, otherwise, the stream is one of required.
    ///         If there are no other streams, any active stream of the module can satisfy the require.
    /// @since 5.0
    const std::vector<std::string> & get_streams() const { return streams; };

    std::string to_string();

private:
    friend class ModuleItem;

    std::string module_name;
    std::vector<std::string> streams;
};


}  // namespace libdnf::module


#endif  // LIBDNF_MODULE_MODULE_DEPENDENCY_HPP
