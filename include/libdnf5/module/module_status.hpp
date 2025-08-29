// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef LIBDNF5_MODULE_MODULE_STATUS_HPP
#define LIBDNF5_MODULE_MODULE_STATUS_HPP

#include "libdnf5/common/exception.hpp"
#include "libdnf5/defs.h"

#include <string>

namespace libdnf5::module {

// TODO(pkratoch): Make this a docstring.
// ENABLED - a module that has an enabled stream.
// DISABLED - a module that is disabled.
// AVAILABLE - otherwise.
enum class ModuleStatus { AVAILABLE, ENABLED, DISABLED };

class LIBDNF_API InvalidModuleStatus : public libdnf5::Error {
public:
    InvalidModuleStatus(const std::string & status);

    const char * get_domain_name() const noexcept override { return "libdnf5::module"; }
    const char * get_name() const noexcept override { return "InvalidModuleStatus"; }
};


LIBDNF_API std::string module_status_to_string(ModuleStatus status);
LIBDNF_API ModuleStatus module_status_from_string(const std::string & status);


}  // namespace libdnf5::module

#endif  // LIBDNF5_MODULE_MODULE_STATUS_HPP
