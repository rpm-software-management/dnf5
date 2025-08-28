// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later


#ifndef DNF5_PLUGINS_REPOCLOSURE_PLUGIN_REPOCLOSURE_HPP
#define DNF5_PLUGINS_REPOCLOSURE_PLUGIN_REPOCLOSURE_HPP


#include <dnf5/context.hpp>
#include <dnf5/shared_options.hpp>
#include <libdnf5-cli/session.hpp>
#include <libdnf5/conf/option_string_list.hpp>

#include <vector>


namespace dnf5 {


class RepoclosureCommand : public Command {
public:
    explicit RepoclosureCommand(Context & context) : Command(context, "repoclosure") {}
    void set_parent_command() override;
    void set_argument_parser() override;
    void configure() override;
    void run() override;

private:
    std::vector<std::string> pkg_specs{};
    std::vector<std::string> check_repos{};
    std::vector<std::string> arches{};
    std::unique_ptr<libdnf5::cli::session::BoolOption> newest{nullptr};
};


}  // namespace dnf5


#endif  // DNF5_PLUGINS_REPOCLOSURE_PLUGIN_REPOCLOSURE_HPP
