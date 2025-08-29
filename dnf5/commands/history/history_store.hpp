// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later


#ifndef DNF5_COMMANDS_HISTORY_HISTORY_STORE_HPP
#define DNF5_COMMANDS_HISTORY_HISTORY_STORE_HPP

#include "commands/history/arguments.hpp"

#include <dnf5/context.hpp>


namespace dnf5 {


class HistoryStoreCommand : public Command {
public:
    explicit HistoryStoreCommand(Context & context) : Command(context, "store") {}
    void set_argument_parser() override;
    void run() override;

private:
    std::unique_ptr<TransactionSpecArguments> transaction_specs{nullptr};
    libdnf5::OptionString * output_option{nullptr};
};


}  // namespace dnf5


#endif  // DNF5_COMMANDS_HISTORY_HISTORY_STORE_HPP
