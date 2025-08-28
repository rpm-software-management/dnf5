// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later


#ifndef DNF5_COMMANDS_REINSTALL_REINSTALL_HPP
#define DNF5_COMMANDS_REINSTALL_REINSTALL_HPP

#include <dnf5/context.hpp>
#include <dnf5/shared_options.hpp>

#include <memory>
#include <vector>


namespace dnf5 {


class ReinstallCommand : public Command {
public:
    explicit ReinstallCommand(Context & context) : Command(context, "reinstall") {}
    void set_parent_command() override;
    void set_argument_parser() override;
    void configure() override;
    void run() override;

private:
    std::vector<std::string> pkg_specs;
    std::unique_ptr<AllowErasingOption> allow_erasing;
    std::vector<std::string> installed_from_repos;
    std::vector<std::string> from_repos;
};


}  // namespace dnf5


#endif  // DNF5_COMMANDS_REINSTALL_REINSTALL_HPP
