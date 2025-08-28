// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef DNF5_COMMANDS_HISTORY_HISTORY_HPP
#define DNF5_COMMANDS_HISTORY_HISTORY_HPP

#include <dnf5/context.hpp>

namespace dnf5 {

class HistoryCommand : public Command {
public:
    explicit HistoryCommand(Context & context) : Command(context, "history") {}
    void set_parent_command() override;
    void set_argument_parser() override;
    void register_subcommands() override;
    void pre_configure() override;
};

}  // namespace dnf5

#endif  // DNF5_COMMANDS_HISTORY_HISTORY_HPP
