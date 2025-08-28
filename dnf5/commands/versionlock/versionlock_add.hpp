// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef DNF5_COMMANDS_VERSIONLOCK_VERSIONLOCK_ADD_HPP
#define DNF5_COMMANDS_VERSIONLOCK_VERSIONLOCK_ADD_HPP


#include <dnf5/context.hpp>


namespace dnf5 {


class VersionlockAddCommand : public Command {
public:
    explicit VersionlockAddCommand(Context & context) : VersionlockAddCommand(context, "add") {}
    void set_argument_parser() override;
    void configure() override;
    void run() override;

protected:
    VersionlockAddCommand(Context & context, const std::string & name) : Command(context, name) {}

private:
    std::vector<std::string> pkg_specs;
};

}  // namespace dnf5


#endif  // DNF5_COMMANDS_VERSIONLOCK_VERSIONLOCK_ADD_HPP
