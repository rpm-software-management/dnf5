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

#ifndef LIBDNF5_MODULE_MODULE_STATUS_HPP
#define LIBDNF5_MODULE_MODULE_STATUS_HPP

#include "libdnf5/common/exception.hpp"

#include <string>

namespace libdnf5::module {

// TODO(pkratoch): Make this a docstring.
// ENABLED - a module that has an enabled stream.
// DISABLED - a module that is disabled.
// AVAILABLE - otherwise.
enum class ModuleStatus { AVAILABLE, ENABLED, DISABLED };

class InvalidModuleStatus : public libdnf5::Error {
public:
    InvalidModuleStatus(const std::string & status);

    const char * get_domain_name() const noexcept override { return "libdnf5::module"; }
    const char * get_name() const noexcept override { return "InvalidModuleStatus"; }
};


std::string module_status_to_string(ModuleStatus status);
ModuleStatus module_status_from_string(const std::string & status);


}  // namespace libdnf5::module

#endif  // LIBDNF5_MODULE_MODULE_STATUS_HPP
