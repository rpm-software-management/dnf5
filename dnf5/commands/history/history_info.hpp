// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef DNF5_COMMANDS_HISTORY_HISTORY_INFO_HPP
#define DNF5_COMMANDS_HISTORY_HISTORY_INFO_HPP

#include "arguments.hpp"

#include <dnf5/context.hpp>

namespace dnf5 {

class HistoryInfoCommand : public Command {
public:
    explicit HistoryInfoCommand(Context & context) : Command(context, "info") {}
    void set_argument_parser() override;
    void run() override;

    std::unique_ptr<TransactionSpecArguments> transaction_specs{nullptr};
    std::unique_ptr<ReverseOption> reverse{nullptr};
    std::unique_ptr<HistoryContainsPkgsOption> contains_pkgs{nullptr};
};

}  // namespace dnf5

#endif  // DNF5_COMMANDS_HISTORY_HISTORY_INFO_HPP
