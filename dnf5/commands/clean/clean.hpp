// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later


#ifndef DNF5_COMMANDS_CLEAN_CLEAN_HPP
#define DNF5_COMMANDS_CLEAN_CLEAN_HPP

#include <dnf5/context.hpp>

#include <memory>


namespace dnf5 {


class CleanCommand : public Command {
public:
    explicit CleanCommand(Context & context) : Command(context, "clean") {}
    void set_parent_command() override;
    void set_argument_parser() override;
    void run() override;

    enum Actions : unsigned {
        NONE = 0,
        CLEAN_ALL = 1 << 0,
        CLEAN_PACKAGES = 1 << 1,
        CLEAN_DBCACHE = 1 << 2,
        CLEAN_METADATA = 1 << 3,
        EXPIRE_CACHE = 1 << 4
    };

private:
    Actions required_actions = NONE;
};


}  // namespace dnf5


#endif  // DNF5_COMMANDS_CLEAN_CLEAN_HPP
