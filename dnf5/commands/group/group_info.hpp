// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef DNF5_COMMANDS_GROUP_GROUP_INFO_HPP
#define DNF5_COMMANDS_GROUP_GROUP_INFO_HPP

#include "group_list.hpp"

namespace dnf5 {


class GroupInfoCommand : public GroupListCommand {
public:
    explicit GroupInfoCommand(Context & context) : GroupListCommand(context, "info") {}

private:
    void print(const libdnf5::comps::GroupQuery & query) override;
};


}  // namespace dnf5


#endif  // DNF5_COMMANDS_GROUP_GROUP_INFO_HPP
