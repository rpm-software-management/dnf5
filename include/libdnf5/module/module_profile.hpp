// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 2.1 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.

#ifndef LIBDNF5_MODULE_MODULE_PROFILE_HPP
#define LIBDNF5_MODULE_MODULE_PROFILE_HPP

#include "libdnf5/defs.h"

#include <memory>
#include <string>
#include <vector>

struct _ModulemdProfile;


namespace libdnf5::module {


// @replaces libdnf:module:modulemd/ModuleProfile.hpp:class:ModuleProfile
class LIBDNF_API ModuleProfile {
public:
    /// @return The profile name.
    /// @since 5.0
    //
    // @replaces libdnf:module:modulemd/ModuleProfile.hpp:method:ModuleProfile.getName()
    std::string get_name() const;

    /// @return The profile description.
    /// @since 5.0
    //
    // @replaces libdnf:module:modulemd/ModuleProfile.hpp:method:ModuleProfile.getDescription()
    std::string get_description() const;

    // TODO(pkratoch): Describe the format of the RPM strings in the docstring (is it NEVRAs?).
    /// @return The list of RPMs.
    /// @since 5.0
    //
    // @replaces libdnf:module:modulemd/ModuleProfile.hpp:method:ModuleProfile.getContent()
    std::vector<std::string> get_rpms() const;

    /// @return `true` if the profile is default for the associated stream.
    /// @since 5.0
    //
    // @replaces libdnf:module:modulemd/ModuleProfile.hpp:method:ModuleProfile.isDefault()
    bool is_default() const;

    // @replaces libdnf:module:modulemd/ModuleProfile.hpp:ctor:ModuleProfile.ModuleProfile()
    ModuleProfile(const ModuleProfile & src);
    ModuleProfile & operator=(const ModuleProfile & src);
    ModuleProfile(ModuleProfile && src) noexcept;
    ModuleProfile & operator=(ModuleProfile && src) noexcept;
    ~ModuleProfile();

private:
    friend class ModuleItem;

    // @replaces libdnf:module:modulemd/ModuleProfile.hpp:ctor:ModuleProfile.ModuleProfile(ModulemdProfile * profile)
    LIBDNF_LOCAL ModuleProfile(_ModulemdProfile * profile, const bool is_default);

    class LIBDNF_LOCAL Impl;
    std::unique_ptr<Impl> p_impl;
};


}  // namespace libdnf5::module


#endif  // LIBDNF5_MODULE_MODULE_PROFILE_HPP
