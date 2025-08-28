// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef DNF5_COMMANDS_VERSIONLOCK_VERSIONLOCK_DELETE_HPP
#define DNF5_COMMANDS_VERSIONLOCK_VERSIONLOCK_DELETE_HPP


#include <dnf5/context.hpp>


namespace dnf5 {


class VersionlockDeleteCommand : public Command {
public:
    explicit VersionlockDeleteCommand(Context & context) : VersionlockDeleteCommand(context, "delete") {}
    void set_argument_parser() override;
    void run() override;

protected:
    VersionlockDeleteCommand(Context & context, const std::string & name) : Command(context, name) {}

private:
    std::vector<std::string> pkg_specs;
};

}  // namespace dnf5


#endif  // DNF5_COMMANDS_VERSIONLOCK_VERSIONLOCK_DELETE_HPP
