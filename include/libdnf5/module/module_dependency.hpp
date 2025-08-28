// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef LIBDNF5_MODULE_MODULE_DEPENDENCY_HPP
#define LIBDNF5_MODULE_MODULE_DEPENDENCY_HPP

#include "libdnf5/defs.h"

#include <memory>
#include <string>
#include <vector>

namespace libdnf5::module {


class LIBDNF_API ModuleDependency {
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
    class LIBDNF_LOCAL Impl;
    std::unique_ptr<Impl> p_impl;
};


}  // namespace libdnf5::module


#endif  // LIBDNF5_MODULE_MODULE_DEPENDENCY_HPP
