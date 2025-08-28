// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later


#ifndef DNF5_COMMANDS_HISTORY_HISTORY_ROLLBACK_HPP
#define DNF5_COMMANDS_HISTORY_HISTORY_ROLLBACK_HPP

#include "commands/history/arguments.hpp"

#include <dnf5/context.hpp>


namespace dnf5 {


class HistoryRollbackCommand : public Command {
public:
    explicit HistoryRollbackCommand(Context & context) : Command(context, "rollback") {}
    void set_argument_parser() override;
    void configure() override;
    void run() override;

private:
    std::unique_ptr<TransactionSpecArguments> transaction_specs{nullptr};
    std::unique_ptr<libdnf5::cli::session::BoolOption> ignore_extras{nullptr};
    std::unique_ptr<libdnf5::cli::session::BoolOption> ignore_installed{nullptr};
};


}  // namespace dnf5


#endif  // DNF5_COMMANDS_HISTORY_HISTORY_ROLLBACK_HPP
