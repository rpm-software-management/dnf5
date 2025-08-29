// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef DNF5_COMMANDS_VERSIONLOCK_VERSIONLOCK_LIST_HPP
#define DNF5_COMMANDS_VERSIONLOCK_VERSIONLOCK_LIST_HPP


#include <dnf5/context.hpp>


namespace dnf5 {


class VersionlockListCommand : public Command {
public:
    explicit VersionlockListCommand(Context & context) : VersionlockListCommand(context, "list") {}
    void set_argument_parser() override;
    void run() override;

protected:
    VersionlockListCommand(Context & context, const std::string & name) : Command(context, name) {}
};


}  // namespace dnf5


#endif  // DNF5_COMMANDS_VERSIONLOCK_VERSIONLOCK_LIST_HPP
