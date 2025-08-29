// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef DNF5_COMMANDS_ENVIRONMENT_ENVIRONMENT_LIST_HPP
#define DNF5_COMMANDS_ENVIRONMENT_ENVIRONMENT_LIST_HPP

#include "arguments.hpp"

#include <dnf5/context.hpp>

#include <memory>

namespace dnf5 {

class EnvironmentListCommand : public Command {
public:
    explicit EnvironmentListCommand(Context & context) : Command(context, "list") {}
    void set_argument_parser() override;
    void configure() override;
    void run() override;

    std::unique_ptr<EnvironmentAvailableOption> available{nullptr};
    std::unique_ptr<EnvironmentInstalledOption> installed{nullptr};
    std::unique_ptr<EnvironmentSpecArguments> environment_specs{nullptr};
};

}  // namespace dnf5

#endif  // DNF5_COMMANDS_ENVIRONMENT_ENVIRONMENT_LIST_HPP
