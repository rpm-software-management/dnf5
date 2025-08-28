// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef DNF5DAEMON_CLIENT_COMMANDS_CLEAN_CLEAN_HPP
#define DNF5DAEMON_CLIENT_COMMANDS_CLEAN_CLEAN_HPP

#include "commands/command.hpp"

#include <libdnf5/conf/option_bool.hpp>

namespace dnfdaemon::client {

class CleanCommand : public TransactionCommand {
public:
    explicit CleanCommand(Context & context) : TransactionCommand(context, "clean") {}
    void set_parent_command() override;
    void set_argument_parser() override;
    void run() override;

private:
    std::vector<std::unique_ptr<libdnf5::Option>> * cache_types{nullptr};
};

}  // namespace dnfdaemon::client

#endif
