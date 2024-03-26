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
#include <libdnf5/conf/option_bool.hpp>

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
    bool system_repo_needed = false;

    libdnf5::OptionBool * available_option{nullptr};
    libdnf5::OptionBool * installed_option{nullptr};
    libdnf5::OptionBool * userinstalled_option{nullptr};
    libdnf5::OptionBool * leaves_option{nullptr};
    libdnf5::OptionBool * info_option{nullptr};
    libdnf5::OptionNumber<std::int32_t> * latest_limit_option{nullptr};
    std::vector<std::string> pkg_specs;
    std::vector<libdnf5::rpm::Package> cmdline_packages;

    std::unique_ptr<libdnf5::cli::session::AppendStringListOption> whatdepends{nullptr};
    std::unique_ptr<libdnf5::cli::session::AppendStringListOption> whatconflicts{nullptr};
    std::unique_ptr<libdnf5::cli::session::AppendStringListOption> whatenhances{nullptr};
    std::unique_ptr<libdnf5::cli::session::AppendStringListOption> whatobsoletes{nullptr};
    std::unique_ptr<libdnf5::cli::session::AppendStringListOption> whatprovides{nullptr};
    std::unique_ptr<libdnf5::cli::session::AppendStringListOption> whatrecommends{nullptr};
    std::unique_ptr<libdnf5::cli::session::AppendStringListOption> whatrequires{nullptr};
    std::unique_ptr<libdnf5::cli::session::AppendStringListOption> whatsuggests{nullptr};
    std::unique_ptr<libdnf5::cli::session::AppendStringListOption> whatsupplements{nullptr};
    std::unique_ptr<libdnf5::cli::session::AppendStringListOption> arch{nullptr};
    std::unique_ptr<libdnf5::cli::session::AppendStringListOption> file{nullptr};

    std::unique_ptr<libdnf5::cli::session::BoolOption> exactdeps{nullptr};
    std::unique_ptr<libdnf5::cli::session::BoolOption> duplicates{nullptr};
    std::unique_ptr<libdnf5::cli::session::BoolOption> unneeded{nullptr};
    std::unique_ptr<libdnf5::cli::session::BoolOption> extras{nullptr};
    std::unique_ptr<libdnf5::cli::session::BoolOption> upgrades{nullptr};
    std::unique_ptr<libdnf5::cli::session::BoolOption> recent{nullptr};
    std::unique_ptr<libdnf5::cli::session::BoolOption> installonly{nullptr};
    std::unique_ptr<libdnf5::cli::session::BoolOption> srpm{nullptr};
    std::unique_ptr<libdnf5::cli::session::BoolOption> disable_modular_filtering{nullptr};
    std::unique_ptr<libdnf5::cli::session::BoolOption> changelogs{nullptr};
    std::unique_ptr<libdnf5::cli::session::BoolOption> recursive{nullptr};

    libdnf5::OptionBool * querytags_option{nullptr};
    libdnf5::OptionString * query_format_option{nullptr};
    libdnf5::OptionEnum * pkg_attr_option{nullptr};
    libdnf5::OptionEnum * providers_of_option{nullptr};

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
