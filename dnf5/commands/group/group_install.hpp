// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef DNF5_COMMANDS_GROUP_GROUP_INSTALL_HPP
#define DNF5_COMMANDS_GROUP_GROUP_INSTALL_HPP

#include "arguments.hpp"

#include <dnf5/context.hpp>
#include <dnf5/shared_options.hpp>

#include <memory>
#include <vector>


namespace dnf5 {


class GroupInstallCommand : public Command {
public:
    explicit GroupInstallCommand(Context & context) : GroupInstallCommand(context, "install") {}
    void set_argument_parser() override;
    void configure() override;
    void run() override;

    std::unique_ptr<AllowErasingOption> allow_erasing;

    std::unique_ptr<GroupWithOptionalOption> with_optional{nullptr};
    std::unique_ptr<GroupNoPackagesOption> no_packages{nullptr};
    std::unique_ptr<GroupSpecArguments> group_specs{nullptr};

protected:
    // to be used by an alias command only
    explicit GroupInstallCommand(Context & context, const std::string & name) : Command(context, name) {}
};


}  // namespace dnf5


#endif  // DNF5_COMMANDS_GROUP_GROUP_INSTALL_HPP
