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

#include "check.hpp"

#include <fmt/format.h>
#include <libdnf5/rpm/nevra.hpp>
#include <libdnf5/rpm/package_query.hpp>
#include <libdnf5/utils/bgettext/bgettext-lib.h>
#include <libdnf5/utils/bgettext/bgettext-mark-domain.h>

#include <iostream>
#include <map>
#include <set>
#include <vector>

namespace dnf5 {

using namespace libdnf5;

namespace {

cli::ArgumentParser::NamedArg * create_option(
    cli::ArgumentParser & parser, const std::string & name, const std::string & descr, bool & opt_to_modify) {
    auto option = parser.add_new_named_arg(name);
    option->set_long_name(name);
    option->set_description(descr);
    option->set_parse_hook_func([&opt_to_modify](cli::ArgumentParser::NamedArg *, const char *, const char *) {
        opt_to_modify = true;
        return true;
    });
    return option;
}

struct ReldepCmp {
    bool operator()(const rpm::Reldep & lhs, const rpm::Reldep & rhs) const {
        return lhs.get_id().id < rhs.get_id().id;
    }
};

struct PackageEvrCmp {
    bool operator()(const rpm::Package & lhs, const rpm::Package & rhs) const { return evrcmp(lhs, rhs) < 0; }
};

}  // namespace

void CheckCommand::set_parent_command() {
    auto * arg_parser_parent_cmd = get_session().get_argument_parser().get_root_command();
    auto * arg_parser_this_cmd = get_argument_parser_command();
    arg_parser_parent_cmd->register_command(arg_parser_this_cmd);
    arg_parser_parent_cmd->get_group("software_management_commands").register_argument(arg_parser_this_cmd);
}

void CheckCommand::set_argument_parser() {
    auto & ctx = get_context();
    auto & parser = ctx.get_argument_parser();

    auto & cmd = *get_argument_parser_command();
    cmd.set_description("Check for problems in the packagedb");

    cmd.register_named_arg(create_option(parser, "dependencies", "Show dependency problems", dependencies));
    cmd.register_named_arg(create_option(parser, "duplicates", "Show duplicate problems", duplicates));
    cmd.register_named_arg(create_option(parser, "obsoleted", "Show obsoleted package", obsoleted));
}

void CheckCommand::configure() {
    auto & context = get_context();
    all = !(dependencies || duplicates || obsoleted);
    context.set_load_system_repo(true);
}

void CheckCommand::run() {
    auto & ctx = get_context();

    libdnf5::rpm::PackageQuery installed(ctx.base, sack::ExcludeFlags::IGNORE_EXCLUDES);
    installed.filter_installed();

    std::set<std::string> output_lines;

    if (all || dependencies) {
        for (auto pkg : installed) {
            // requires_set = (requires_pre - prereq_ignoreinst) + regular_requires
            std::set<rpm::Reldep, ReldepCmp> requires_set;
            auto require_pre = pkg.get_requires_pre();
            requires_set.insert(require_pre.begin(), require_pre.end());
            for (auto reldep : pkg.get_prereq_ignoreinst()) {
                requires_set.erase(reldep);
            }
            auto require = pkg.get_regular_requires();
            requires_set.insert(require.begin(), require.end());
            for (const auto & require : requires_set) {
                auto require_str = require.to_string();
                if (require_str.starts_with("rpmlib")) {
                    continue;
                }
                auto provides = installed;
                provides.filter_provides(require);
                if (provides.empty()) {
                    if (rpm::Reldep::is_rich_dependency(require_str)) {
                        // rich deps can be only tested by solver
                        Goal goal(ctx.base);
                        goal.add_provide_install(require_str);
                        // there is only system repo in sack, therefore solved is only in case
                        // when rich deps doesn't require any additional package
                        if (goal.resolve().get_problems() == GoalProblem::NO_PROBLEM) {
                            continue;
                        }
                    }
                    auto msg = fmt ::format("{} has missing requires of {}", pkg.get_full_nevra(), require_str);
                    output_lines.insert(msg);
                }
            }

            for (auto conflict : pkg.get_conflicts()) {
                auto conflicted = installed;
                conflicted.filter_provides(conflict);
                for (auto conflict_pkg : conflicted) {
                    if (conflict_pkg == pkg) {
                        // skip self conflicts
                        continue;
                    }
                    // If A conflicts with B then B conflicts with A.
                    // Prints the packages in a row sorted to find and remove duplicate rows.
                    bool swap = cmp_nevra(conflict_pkg, pkg);
                    auto msg = fmt::format(
                        "{} has installed conflict \"{}\": {}",
                        swap ? conflict_pkg.get_full_nevra() : pkg.get_full_nevra(),
                        conflict.to_string(),
                        swap ? pkg.get_full_nevra() : conflict_pkg.get_full_nevra());
                    output_lines.insert(msg);
                }
            }
        }
    }

    if (all || duplicates) {
        auto installonly = installed;
        installonly.filter_installonly();
        auto duplicated = installed;
        duplicated -= installonly;
        duplicated.filter_duplicates();

        if (!duplicated.empty()) {
            std::map<std::string, std::vector<rpm::Package>> na_pkgs;
            for (auto pkg : duplicated) {
                na_pkgs[pkg.get_na()].push_back(pkg);
            }
            for (auto & [na, pkgs] : na_pkgs) {
                std::sort(pkgs.begin(), pkgs.end(), PackageEvrCmp{});
                auto nevra = pkgs[0].get_full_nevra();
                for (size_t i = 1; i < pkgs.size(); ++i) {
                    auto msg = fmt::format("{} is a duplicate with {}", nevra, pkgs[i].get_full_nevra());
                    output_lines.insert(msg);
                }
            }
        }
    }

    if (all || obsoleted) {
        for (auto pkg : installed) {
            for (auto obsolete : pkg.get_obsoletes()) {
                auto obsoleted = installed;
                obsoleted.filter_provides(obsolete);
                auto obsolete_str = obsolete.to_string();
                obsoleted.filter_name({obsolete_str.substr(0, obsolete_str.find_first_of(" \t\n\r\f\v"))});
                if (!obsoleted.empty()) {
                    auto msg = fmt::format(
                        "{} is obsoleted by {}", (*obsoleted.begin()).get_full_nevra(), pkg.get_full_nevra());
                    output_lines.insert(msg);
                }
            }
        }
    }

    if (!output_lines.empty()) {
        for (const auto & line : output_lines) {
            std::cout << line << std::endl;
        }

        throw Error(M_("Check discovered {} problem(s)"), output_lines.size());
    }
}

}  // namespace dnf5
