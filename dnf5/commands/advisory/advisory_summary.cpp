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


#include "advisory_summary.hpp"

#include "dnf5/context.hpp"

#include <filesystem>
#include <fstream>
#include <set>


namespace dnf5 {


using namespace libdnf::cli;


AdvisorySummaryCommand::AdvisorySummaryCommand(Command & parent) : AdvisorySummaryCommand(parent, "summary") {}


AdvisorySummaryCommand::AdvisorySummaryCommand(Command & parent, const std::string & name) : Command(parent, name) {
    auto & cmd = *get_argument_parser_command();
    cmd.set_short_description("Summary advisories");

    available = std::make_unique<AdvisoryAvailableOption>(*this);
    installed = std::make_unique<AdvisoryInstalledOption>(*this);
    all = std::make_unique<AdvisoryAllOption>(*this);
    updates = std::make_unique<AdvisoryUpdatesOption>(*this);
    // TODO(amatej): set_conflicting_args({available, installed, all, updates});

    package_specs = std::make_unique<AdvisorySpecArguments>(*this);
}


void AdvisorySummaryCommand::run() {
    //TODO(amatej): implement
}


}  // namespace dnf5
