// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later


#ifndef DNF5_COMMANDS_HISTORY_HISTORY_REDO_HPP
#define DNF5_COMMANDS_HISTORY_HISTORY_REDO_HPP

#include "commands/history/arguments.hpp"

#include <dnf5/context.hpp>


namespace dnf5 {


class HistoryRedoCommand : public Command {
public:
    explicit HistoryRedoCommand(Context & context) : Command(context, "redo") {}
    void set_argument_parser() override;
    void configure() override;
    void run() override;

    std::unique_ptr<TransactionSpecArguments> transaction_specs{nullptr};
};


}  // namespace dnf5


#endif  // DNF5_COMMANDS_HISTORY_HISTORY_REDO_HPP
