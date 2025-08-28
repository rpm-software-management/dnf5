// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later


#ifndef DNF5_COMMANDS_HISTORY_HISTORY_UNDO_HPP
#define DNF5_COMMANDS_HISTORY_HISTORY_UNDO_HPP

#include "commands/history/arguments.hpp"

#include <dnf5/context.hpp>


namespace dnf5 {


class HistoryUndoCommand : public Command {
public:
    explicit HistoryUndoCommand(Context & context) : Command(context, "undo") {}
    void set_argument_parser() override;
    void configure() override;
    void run() override;

    std::unique_ptr<TransactionSpecArguments> transaction_specs{nullptr};
    std::unique_ptr<libdnf5::cli::session::BoolOption> ignore_extras{nullptr};
    std::unique_ptr<libdnf5::cli::session::BoolOption> ignore_installed{nullptr};
};


}  // namespace dnf5


#endif  // DNF5_COMMANDS_HISTORY_HISTORY_UNDO_HPP
