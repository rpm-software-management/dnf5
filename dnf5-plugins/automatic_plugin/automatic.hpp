/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/


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
    std::unique_ptr<libdnf5::cli::session::BoolOption> timer{nullptr};
    ConfigAutomatic config_automatic;
    bool download_callbacks_set{false};
    std::stringstream output_stream;
};


}  // namespace dnf5


#endif  // DNF5_PLUGINS_AUTOMATIC_PLUGIN_AUTOMATIC_HPP
