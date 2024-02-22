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

#ifndef LIBDNF5_MODULE_MODULE_DEPENDENCY_HPP
#define LIBDNF5_MODULE_MODULE_DEPENDENCY_HPP

#include <memory>
#include <string>
#include <vector>

namespace libdnf5::module {


class ModuleDependency {
public:
    ModuleDependency(const std::string & module_name, const std::vector<std::string> & streams);

    ModuleDependency(std::string && module_name, std::vector<std::string> && streams);

    ~ModuleDependency();

    ModuleDependency(const ModuleDependency & src);
    ModuleDependency & operator=(const ModuleDependency & src);

    ModuleDependency(ModuleDependency && src) noexcept;
    ModuleDependency & operator=(ModuleDependency && src) noexcept;

    /// @return Name of the required module.
    /// @since 5.0
    const std::string & get_module_name() const;

    /// @return Vector of streams. Prefix '-' denotes conflicting stream, otherwise, the stream is one of required.
    ///         If there are no other streams, any active stream of the module can satisfy the require.
    /// @since 5.0
    const std::vector<std::string> & get_streams() const;

    std::string to_string();

private:
    class Impl;
    std::unique_ptr<Impl> p_impl;
};


}  // namespace libdnf5::module


#endif  // LIBDNF5_MODULE_MODULE_DEPENDENCY_HPP
