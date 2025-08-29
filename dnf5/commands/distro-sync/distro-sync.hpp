// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later


#ifndef DNF5_COMMANDS_DISTRO_SYNC_DISTRO_SYNC_HPP
#define DNF5_COMMANDS_DISTRO_SYNC_DISTRO_SYNC_HPP

#include <dnf5/context.hpp>
#include <dnf5/shared_options.hpp>
#include <libdnf5/conf/option_bool.hpp>

#include <memory>
#include <vector>


namespace dnf5 {


class DistroSyncCommand : public Command {
public:
    explicit DistroSyncCommand(Context & context) : Command(context, "distro-sync") {}
    void set_parent_command() override;
    void set_argument_parser() override;
    void configure() override;
    void run() override;

    std::vector<std::unique_ptr<libdnf5::Option>> * patterns_to_distro_sync_options{nullptr};

    std::unique_ptr<AllowErasingOption> allow_erasing;
    std::vector<std::string> installed_from_repos;
    std::vector<std::string> from_repos;
};


}  // namespace dnf5


#endif  // DNF5_COMMANDS_DISTRO_SYNC_DISTRO_SYNC_HPP
