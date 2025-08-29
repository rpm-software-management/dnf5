// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef DNF5_COMMANDS_VERSIONLOCK_VERSIONLOCK_CLEAR_HPP
#define DNF5_COMMANDS_VERSIONLOCK_VERSIONLOCK_CLEAR_HPP


#include <dnf5/context.hpp>


namespace dnf5 {


class VersionlockClearCommand : public Command {
public:
    explicit VersionlockClearCommand(Context & context) : VersionlockClearCommand(context, "clear") {}
    void set_argument_parser() override;
    void run() override;

protected:
    VersionlockClearCommand(Context & context, const std::string & name) : Command(context, name) {}
};

}  // namespace dnf5


#endif  // DNF5_COMMANDS_VERSIONLOCK_VERSIONLOCK_CLEAR_HPP
