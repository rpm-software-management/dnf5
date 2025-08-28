// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later


#ifndef DNF5_COMMANDS_REPLAY__REPLAY_HPP
#define DNF5_COMMANDS_REPLAY__REPLAY_HPP

#include <dnf5/context.hpp>
#include <libdnf5/conf/option_bool.hpp>

namespace dnf5 {


class ReplayCommand : public Command {
public:
    explicit ReplayCommand(Context & context) : Command(context, "replay") {}
    void set_parent_command() override;
    void configure() override;
    void set_argument_parser() override;
    void run() override;

private:
    std::string transaction_path;

    std::unique_ptr<libdnf5::cli::session::BoolOption> ignore_extras{nullptr};
    std::unique_ptr<libdnf5::cli::session::BoolOption> ignore_installed{nullptr};
};


}  // namespace dnf5


#endif  // DNF5_COMMANDS_REPLAY__REPLAY_HPP
