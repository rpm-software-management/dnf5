/*
Copyright Contributors to the dnf5 project.

This file is part of dnf5: https://github.com/rpm-software-management/dnf5/

Dnf5 is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2.1 of the License, or
(at your option) any later version.

Dnf5 is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with dnf5.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "from_repo.hpp"

#include <libdnf5-cli/argument_parser.hpp>
#include <libdnf5/conf/option_string_list.hpp>
#include <libdnf5/utils/bgettext/bgettext-lib.h>
#include <libdnf5/utils/bgettext/bgettext-mark-domain.h>


namespace dnf5 {

void create_from_repo_option(Command & command, std::vector<std::string> & from_repos, bool detect_conflict) {
    auto & parser = command.get_context().get_argument_parser();
    auto from_repo_opt = parser.add_new_named_arg("from-repo");
    from_repo_opt->set_long_name("from-repo");
    from_repo_opt->set_description(
        _("The following items can be selected only from the specified repositories. All enabled repositories will "
          "still be used to satisfy dependencies."));
    from_repo_opt->set_has_value(true);
    from_repo_opt->set_arg_value_help(_("REPO_ID,..."));
    from_repo_opt->set_parse_hook_func(
        [&command, &from_repos, detect_conflict](
            libdnf5::cli::ArgumentParser::NamedArg *, [[maybe_unused]] const char * option, const char * value) {
            if (!detect_conflict || from_repos.empty()) {
                from_repos = libdnf5::OptionStringList(value).get_value();

                // We need to ensure repositories for requested packages are enabled. We'll explicitly enable them
                // using setopts as a safeguard. setopts will be applied later during repository configuration loading.
                for (const auto & repoid_pattern : from_repos) {
                    command.get_context().get_setopts().emplace_back(repoid_pattern + ".enabled", "1");
                }
            } else {
                if (from_repos != libdnf5::OptionStringList(value).get_value()) {
                    throw libdnf5::cli::ArgumentParserConflictingArgumentsError(
                        M_("\"--from_repo\" already defined with diferent value"));
                }
            }
            return true;
        });
    command.get_argument_parser_command()->register_named_arg(from_repo_opt);
}


libdnf5::cli::ArgumentParser::NamedArg * create_installed_from_repo_option(
    Command & command, std::vector<std::string> & from_repos, bool detect_conflict) {
    auto & parser = command.get_context().get_argument_parser();
    auto * from_repo_opt = parser.add_new_named_arg("installed-from-repo");
    from_repo_opt->set_long_name("installed-from-repo");
    from_repo_opt->set_description(
        _("Filters installed packages by the ID of the repository they were installed from."));
    from_repo_opt->set_has_value(true);
    from_repo_opt->set_arg_value_help(_("REPO_ID,..."));
    from_repo_opt->set_parse_hook_func(
        [&from_repos, detect_conflict](
            libdnf5::cli::ArgumentParser::NamedArg *, [[maybe_unused]] const char * option, const char * value) {
            if (!detect_conflict || from_repos.empty()) {
                from_repos = libdnf5::OptionStringList(value).get_value();
            } else {
                if (from_repos != libdnf5::OptionStringList(value).get_value()) {
                    throw libdnf5::cli::ArgumentParserConflictingArgumentsError(
                        M_("\"--installed-from_repo\" already defined with diferent value"));
                }
            }
            return true;
        });
    command.get_argument_parser_command()->register_named_arg(from_repo_opt);
    return from_repo_opt;
}


}  // namespace dnf5
