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


#ifndef DNF5_COMMANDS_LIST_LIST_HPP
#define DNF5_COMMANDS_LIST_LIST_HPP

#include <dnf5/context.hpp>
#include <dnf5/shared_options.hpp>
#include <libdnf5-cli/output/package_list_sections.hpp>
#include <libdnf5-cli/session.hpp>
#include <libdnf5/rpm/package_set.hpp>

#include <memory>
#include <vector>


namespace dnf5 {


class ListCommand : public Command {
public:
    explicit ListCommand(Context & context) : Command(context, "list") {}
    void set_parent_command() override;
    void set_argument_parser() override;
    void configure() override;
    void run() override;

protected:
    explicit ListCommand(Context & context, const std::string & name) : Command(context, name) {}
    virtual std::string get_command_description() const {
        return _("Lists packages depending on the packages' relation to the system");
    }

private:
    virtual std::unique_ptr<libdnf5::cli::output::PackageListSections> create_output();

    enum class PkgNarrow { ALL, INSTALLED, AVAILABLE, EXTRAS, OBSOLETES, RECENT, UPGRADES, AUTOREMOVE };

    std::vector<std::string> pkg_specs;

    // options to narrow the output packages
    PkgNarrow pkg_narrow{PkgNarrow::ALL};
    std::unique_ptr<libdnf5::cli::session::BoolOption> installed{nullptr};
    std::unique_ptr<libdnf5::cli::session::BoolOption> available{nullptr};
    std::unique_ptr<libdnf5::cli::session::BoolOption> extras{nullptr};
    std::unique_ptr<libdnf5::cli::session::BoolOption> obsoletes{nullptr};
    std::unique_ptr<libdnf5::cli::session::BoolOption> recent{nullptr};
    std::unique_ptr<libdnf5::cli::session::BoolOption> upgrades{nullptr};
    std::unique_ptr<libdnf5::cli::session::BoolOption> autoremove{nullptr};

    // show all NEVRAs, not only the latest one
    std::unique_ptr<libdnf5::cli::session::BoolOption> show_duplicates{nullptr};

    std::vector<std::string> installed_from_repos;
};


}  // namespace dnf5


#endif  // DNF5_COMMANDS_LIST_LIST_HPP
