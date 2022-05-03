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


#ifndef DNF5_COMMANDS_ADVISORY_ADVISORY_LIST_HPP
#define DNF5_COMMANDS_ADVISORY_ADVISORY_LIST_HPP


#include "arguments.hpp"

#include <libdnf-cli/session.hpp>
#include <libdnf/conf/option_bool.hpp>

#include <memory>
#include <vector>


namespace dnf5 {


class AdvisoryListCommand : public libdnf::cli::session::Command {
public:
    explicit AdvisoryListCommand(Command & parent);
    void run() override;

    std::unique_ptr<AdvisoryAvailableOption> available{nullptr};
    std::unique_ptr<AdvisoryInstalledOption> installed{nullptr};
    std::unique_ptr<AdvisoryAllOption> all{nullptr};
    std::unique_ptr<AdvisoryUpdatesOption> updates{nullptr};
    std::unique_ptr<AdvisorySpecArguments> package_specs{nullptr};

protected:
    // to be used by an alias command only
    explicit AdvisoryListCommand(Command & parent, const std::string & name);
};


}  // namespace dnf5


#endif  // DNF5_COMMANDS_ADVISORY_ADVISORY_LIST_HPP
