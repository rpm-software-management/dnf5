// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later


#ifndef DNF5_COMMANDS_DEBUGINFO_INSTALL_DEBUGINFO_INSTALL_HPP
#define DNF5_COMMANDS_DEBUGINFO_INSTALL_DEBUGINFO_INSTALL_HPP

#include <dnf5/context.hpp>
#include <dnf5/shared_options.hpp>

namespace dnf5 {


class DebuginfoInstallCommand : public Command {
public:
    explicit DebuginfoInstallCommand(Context & context) : Command(context, "debuginfo-install") {}
    void set_parent_command() override;
    void set_argument_parser() override;
    void configure() override;
    void run() override;

    std::vector<std::unique_ptr<libdnf5::Option>> * patterns_to_debuginfo_install_options{nullptr};

    std::unique_ptr<AllowErasingOption> allow_erasing;
};


}  // namespace dnf5


#endif  // DNF5_COMMANDS_DEBUGINFO_INSTALL_DEBUGINFO_INSTALL_HPP
