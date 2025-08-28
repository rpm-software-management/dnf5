// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef DNF5_COMMANDS_PROVIDES_PROVIDES_HPP
#define DNF5_COMMANDS_PROVIDES_PROVIDES_HPP

#include "libdnf5-cli/output/provides.hpp"

#include <dnf5/context.hpp>

#include <string>
#include <vector>

namespace dnf5 {

class ProvidesCommand : public Command {
public:
    explicit ProvidesCommand(Context & context) : Command(context, "provides") {}
    void set_parent_command() override;
    void set_argument_parser() override;
    void configure() override;
    void run() override;

private:
    std::vector<std::string> pkg_specs;
    std::pair<libdnf5::rpm::PackageQuery, libdnf5::cli::output::ProvidesMatchedBy> filter_spec(
        std::string, const libdnf5::rpm::PackageQuery &);
};
}  // namespace dnf5

#endif  // DNF5_COMMANDS_PROVIDES_PROVIDES_HPP
