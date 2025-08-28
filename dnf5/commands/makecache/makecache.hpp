// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef DNF5_COMMANDS_MAKECAHE_MAKECACHE_HPP
#define DNF5_COMMANDS_MAKECAHE_MAKECACHE_HPP

#include <dnf5/context.hpp>

namespace dnf5 {

class MakeCacheCommand : public Command {
public:
    explicit MakeCacheCommand(Context & context) : Command(context, "makecache") {}
    void set_parent_command() override;
    void set_argument_parser() override;
    void run() override;
};

}  // namespace dnf5

#endif  // DNF5_COMMANDS_MAKECAHE_MAKECACHE_HPP
