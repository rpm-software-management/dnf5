// Copyright Contributors to the DNF5 project
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef DNF5_COMMANDS_ENVIRONMENT_ENVIRONMENT_INSTALL_HPP
#define DNF5_COMMANDS_ENVIRONMENT_ENVIRONMENT_INSTALL_HPP

#include "../group/group_install.hpp"

#include <dnf5/context.hpp>


namespace dnf5 {


class EnvironmentInstallCommand : public GroupInstallCommand {
public:
    explicit EnvironmentInstallCommand(Context & context) : GroupInstallCommand(context) {}

protected:
    const libdnf5::CompsTypePreferred & get_comps_type_preferred() const override { return comps_type_preferred; }

private:
    static inline constexpr libdnf5::CompsTypePreferred comps_type_preferred = libdnf5::CompsTypePreferred::ENVIRONMENT;
};


}  // namespace dnf5


#endif  // DNF5_COMMANDS_ENVIRONMENT_ENVIRONMENT_INSTALL_HPP
