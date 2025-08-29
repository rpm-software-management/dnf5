// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later


#ifndef DNF5_PLUGINS_AUTOMATIC_PLUGIN_AUTOMATIC_HPP
#define DNF5_PLUGINS_AUTOMATIC_PLUGIN_AUTOMATIC_HPP

#include "config_automatic.hpp"

#include <dnf5/context.hpp>
#include <dnf5/shared_options.hpp>
#include <libdnf5-cli/session.hpp>
#include <libdnf5/conf/option_bool.hpp>

#include <iostream>
#include <sstream>
#include <vector>


namespace dnf5 {


class AutomaticCommand : public Command {
public:
    explicit AutomaticCommand(Context & context) : Command(context, "automatic") {}
    ~AutomaticCommand();
    void set_parent_command() override;
    void set_argument_parser() override;
    void pre_configure() override;
    void configure() override;
    void run() override;

private:
    void wait_for_network();

    std::unique_ptr<libdnf5::cli::session::BoolOption> timer{nullptr};
    ConfigAutomatic config_automatic;
    bool download_callbacks_set{false};
    std::stringstream output_stream;
};


}  // namespace dnf5


#endif  // DNF5_PLUGINS_AUTOMATIC_PLUGIN_AUTOMATIC_HPP
