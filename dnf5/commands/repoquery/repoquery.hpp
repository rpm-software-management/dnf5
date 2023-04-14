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


#ifndef DNF5_COMMANDS_REPOQUERY_REPOQUERY_HPP
#define DNF5_COMMANDS_REPOQUERY_REPOQUERY_HPP

#include "../advisory_shared.hpp"

#include <dnf5/context.hpp>
#include <libdnf/conf/option_bool.hpp>

#include <memory>
#include <vector>


namespace dnf5 {


class RepoqueryCommand : public Command {
public:
    explicit RepoqueryCommand(Context & context) : Command(context, "repoquery") {}
    void set_parent_command() override;
    void set_argument_parser() override;
    void configure() override;
    void load_additional_packages() override;
    void run() override;

private:
    bool only_system_repo_needed = false;

    libdnf::OptionBool * available_option{nullptr};
    libdnf::OptionBool * installed_option{nullptr};
    libdnf::OptionBool * userinstalled_option{nullptr};
    libdnf::OptionBool * leaves_option{nullptr};
    libdnf::OptionBool * info_option{nullptr};
    libdnf::OptionNumber<std::int32_t> * latest_limit_option{nullptr};
    std::vector<std::string> pkg_specs;
    std::vector<libdnf::rpm::Package> cmdline_packages;

    libdnf::OptionStringList * whatdepends_option{nullptr};
    libdnf::OptionStringList * whatconflicts_option{nullptr};
    libdnf::OptionStringList * whatenhances_option{nullptr};
    libdnf::OptionStringList * whatobsoletes_option{nullptr};
    libdnf::OptionStringList * whatprovides_option{nullptr};
    libdnf::OptionStringList * whatrecommends_option{nullptr};
    libdnf::OptionStringList * whatrequires_option{nullptr};
    libdnf::OptionStringList * whatsuggests_option{nullptr};
    libdnf::OptionStringList * whatsupplements_option{nullptr};

    std::unique_ptr<libdnf::cli::session::BoolOption> exactdeps{nullptr};
    std::unique_ptr<libdnf::cli::session::BoolOption> duplicates{nullptr};

    libdnf::OptionString * query_format_option{nullptr};
    libdnf::OptionEnum<std::string> * pkg_attr_option{nullptr};

    std::unique_ptr<AdvisoryOption> advisory_name{nullptr};
    std::unique_ptr<SecurityOption> advisory_security{nullptr};
    std::unique_ptr<BugfixOption> advisory_bugfix{nullptr};
    std::unique_ptr<EnhancementOption> advisory_enhancement{nullptr};
    std::unique_ptr<NewpackageOption> advisory_newpackage{nullptr};
    std::unique_ptr<AdvisorySeverityOption> advisory_severity{nullptr};
    std::unique_ptr<BzOption> advisory_bz{nullptr};
    std::unique_ptr<CveOption> advisory_cve{nullptr};
};


}  // namespace dnf5


#endif  // DNF5_COMMANDS_REPOQUERY_REPOQUERY_HPP
