// Copyright Contributors to the DNF5 project
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef DNF5_COMMANDS_ENVIRONMENT_ENVIRONMENT_REMOVE_HPP
#define DNF5_COMMANDS_ENVIRONMENT_ENVIRONMENT_REMOVE_HPP

#include "../group/group_remove.hpp"

#include <dnf5/context.hpp>


namespace dnf5 {


class EnvironmentRemoveCommand : public GroupRemoveCommand {
public:
    explicit EnvironmentRemoveCommand(Context & context) : GroupRemoveCommand(context) {}

protected:
    const libdnf5::CompsTypePreferred & get_comps_type_preferred() const override { return comps_type_preferred; }

private:
    static inline constexpr libdnf5::CompsTypePreferred comps_type_preferred = libdnf5::CompsTypePreferred::ENVIRONMENT;
};


}  // namespace dnf5


#endif  // DNF5_COMMANDS_ENVIRONMENT_ENVIRONMENT_REMOVE_HPP
