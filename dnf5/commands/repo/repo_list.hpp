// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef DNF5_COMMANDS_REPO_REPO_LIST_HPP
#define DNF5_COMMANDS_REPO_REPO_LIST_HPP

#include "arguments.hpp"

#include <dnf5/context.hpp>

namespace dnf5 {

class RepoListCommand : public Command {
public:
    explicit RepoListCommand(Context & context) : RepoListCommand(context, "list") {}
    void set_argument_parser() override;
    void run() override;

    std::unique_ptr<RepoAllOption> all{nullptr};
    std::unique_ptr<RepoEnabledOption> enabled{nullptr};
    std::unique_ptr<RepoDisabledOption> disabled{nullptr};
    std::unique_ptr<RepoSpecArguments> repo_specs{nullptr};

protected:
    // for RepoInfoCommand
    explicit RepoListCommand(Context & context, const std::string & name) : Command(context, name) {}

    virtual void print(const libdnf5::repo::RepoQuery & query, bool with_status);
};

}  // namespace dnf5

#endif  // DNF5_COMMANDS_REPO_REPO_LIST_HPP
