// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef DNF5_COMMANDS_VERSIONLOCK_VERSIONLOCK_EXCLUDE_HPP
#define DNF5_COMMANDS_VERSIONLOCK_VERSIONLOCK_EXCLUDE_HPP


#include <dnf5/context.hpp>


namespace dnf5 {


class VersionlockExcludeCommand : public Command {
public:
    explicit VersionlockExcludeCommand(Context & context) : VersionlockExcludeCommand(context, "exclude") {}
    void set_argument_parser() override;
    void configure() override;
    void run() override;

protected:
    VersionlockExcludeCommand(Context & context, const std::string & name) : Command(context, name) {}

private:
    std::vector<std::string> pkg_specs;
};

}  // namespace dnf5


#endif  // DNF5_COMMANDS_VERSIONLOCK_VERSIONLOCK_EXCLUDE_HPP
