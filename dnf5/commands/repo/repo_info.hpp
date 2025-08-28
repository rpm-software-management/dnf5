// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef DNF5_COMMANDS_REPO_REPO_INFO_HPP
#define DNF5_COMMANDS_REPO_REPO_INFO_HPP

#include "arguments.hpp"
#include "repo_list.hpp"

#include <dnf5/context.hpp>

namespace dnf5 {

class RepoInfoCommand : public RepoListCommand {
public:
    explicit RepoInfoCommand(Context & context) : RepoListCommand(context, "info") {}

    void set_argument_parser() override {
        RepoListCommand::set_argument_parser();
        get_argument_parser_command()->set_description("Print details about repositories");
    }

    void configure() override;

protected:
    void print(const libdnf5::repo::RepoQuery & query, [[maybe_unused]] bool with_status) override;
};

}  // namespace dnf5

#endif  // DNF5_COMMANDS_REPO_REPO_INFO_HPP
