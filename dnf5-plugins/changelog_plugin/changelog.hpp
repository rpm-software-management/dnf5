// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.

#ifndef DNF5_COMMANDS_CHANGELOG_HPP
#define DNF5_COMMANDS_CHANGELOG_HPP

#include <dnf5/context.hpp>
#include <libdnf5/conf/option_bool.hpp>
#include <libdnf5/conf/option_number.hpp>

#include <memory>
#include <vector>

namespace dnf5 {

class ChangelogCommand : public Command {
public:
    explicit ChangelogCommand(Context & context) : Command(context, "changelog") {}
    void set_parent_command() override;
    void set_argument_parser() override;
    void configure() override;
    void run() override;

private:
    libdnf5::OptionNumber<std::int32_t> * count_option{nullptr};
    libdnf5::OptionNumber<std::int64_t> * since_option{nullptr};
    libdnf5::OptionBool * upgrades_option{nullptr};
    std::vector<std::unique_ptr<libdnf5::Option>> * pkgs_spec_to_show_options{nullptr};
};

}  // namespace dnf5

#endif  // DNF5_COMMANDS_CHANGELOG_HPP
