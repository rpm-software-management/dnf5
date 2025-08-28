// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef LIBDNF5_COMPS_GROUP_PACKAGE_HPP
#define LIBDNF5_COMPS_GROUP_PACKAGE_HPP

#include "package_errors.hpp"
#include "package_type.hpp"

#include "libdnf5/defs.h"

#include <string>

namespace libdnf5::comps {

// TODO(dmach): isn't it more a package dependency rather than a package?

// @replaces dnf:dnf/comps.py:class:Package
// @replaces dnf:dnf/comps.py:class:CompsTransPkg
class LIBDNF_API Package {
public:
    Package(const std::string & name, PackageType type, const std::string & condition);

    ~Package();

    Package(const Package & src);
    Package & operator=(const Package & src);

    Package(Package && src) noexcept;
    Package & operator=(Package && src) noexcept;

    bool operator==(const Package & other) const noexcept;

    bool operator!=(const Package & other) const noexcept;

    /// @return The Package name.
    /// @since 5.0
    //
    // @replaces dnf:dnf/comps.py:attribute:Package.name
    std::string get_name() const;
    void set_name(const std::string & value);

    /// @return The PackageType.
    /// @since 5.0
    PackageType get_type() const;
    void set_type(const PackageType & value);

    /// @return std::string that corresponds to the PackageType.
    /// @since 5.0
    std::string get_type_string() const;

    /// @return The condition (name of package) under which the package gets installed.
    /// @since 5.0
    std::string get_condition() const;
    void set_condition(const std::string & value);

private:
    class LIBDNF_LOCAL Impl;
    std::unique_ptr<Impl> p_impl;
};


}  // namespace libdnf5::comps

#endif
