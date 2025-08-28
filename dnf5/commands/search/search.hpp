// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later


#ifndef DNF5_COMMANDS_SEARCH_SEARCH_HPP
#define DNF5_COMMANDS_SEARCH_SEARCH_HPP

#include "arguments.hpp"

#include <dnf5/context.hpp>
#include <libdnf5-cli/session.hpp>

#include <memory>
#include <vector>


namespace dnf5 {

class SearchCommand : public Command {
public:
    explicit SearchCommand(Context & context) : Command(context, "search") {}
    void set_parent_command() override;
    void set_argument_parser() override;
    void configure() override;
    void run() override;

private:
    std::unique_ptr<SearchAllOption> all{nullptr};
    std::unique_ptr<SearchPatternsArguments> patterns{nullptr};
    std::unique_ptr<libdnf5::cli::session::BoolOption> show_duplicates{nullptr};
};

}  // namespace dnf5


#endif  // DNF5_COMMANDS_SEARCH_SEARCH_HPP
