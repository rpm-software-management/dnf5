// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later
//
//    This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
//    Libdnf is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 2 of the License, or
//    (at your option) any later version.
//
//    Libdnf is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with libdnf.  If not, see <https://www.gnu.org/licenses/>.

#include "repomanage.hpp"

#include "utils/url.hpp"

#include <libdnf5-cli/argument_parser.hpp>
#include <libdnf5/rpm/package.hpp>
#include <libdnf5/rpm/package_query.hpp>
#include <libdnf5/utils/bgettext/bgettext-lib.h>
#include <libdnf5/utils/format.hpp>

constexpr const char * REPOMANAGE_REPO_NAME{"repomanage_repo"};

namespace dnf5 {

void RepomanageCommand::set_parent_command() {
    auto * arg_parser_parent_cmd = get_session().get_argument_parser().get_root_command();
    auto * arg_parser_this_cmd = get_argument_parser_command();
    arg_parser_parent_cmd->register_command(arg_parser_this_cmd);
}

void RepomanageCommand::set_argument_parser() {
    auto & ctx = get_context();
    auto & parser = ctx.get_argument_parser();

    auto & cmd = *get_argument_parser_command();
    cmd.set_description(_("Manage a directory with repodata or with rpm packages"));

    auto path_arg = parser.add_new_positional_arg("path", 1, nullptr, nullptr);
    path_arg->set_description(_("Path to a directory with rpms or path/url to repodata"));
    path_arg->set_parse_hook_func([this](
                                      [[maybe_unused]] libdnf5::cli::ArgumentParser::PositionalArg * arg,
                                      [[maybe_unused]] int argc,
                                      const char * const argv[]) {
        repo_path = argv[0];
        return true;
    });
    cmd.register_positional_arg(path_arg);

    keep_count = dynamic_cast<libdnf5::OptionNumber<std::int32_t> *>(
        parser.add_init_value(std::make_unique<libdnf5::OptionNumber<std::int32_t>>(1)));
    auto keep_arg = parser.add_new_named_arg("keep");
    keep_arg->set_long_name("keep");
    keep_arg->set_short_name('k');
    keep_arg->set_has_value(true);
    keep_arg->set_arg_value_help("N");
    keep_arg->set_description(_("Set package count N for --new and --old (default: 1)"));
    keep_arg->link_value(keep_count);
    cmd.register_named_arg(keep_arg);

    old_option = std::make_unique<libdnf5::cli::session::BoolOption>(
        *this, "old", 'o', _("Print all packages except the N newest for each name.arch"), false);
    new_option = std::make_unique<libdnf5::cli::session::BoolOption>(
        *this, "new", 'n', _("Print N newest packages for each name.arch (default)"), false);

    old_option->get_arg()->add_conflict_argument(*new_option->get_arg());
    new_option->get_arg()->add_conflict_argument(*old_option->get_arg());

    space_option = std::make_unique<libdnf5::cli::session::BoolOption>(
        *this, "space", 's', _("Print packages separated by spaces instead of new lines"), false);
}

void RepomanageCommand::pre_configure() {
    // We are not intereseted in any configured repositories.
    // Don't create them and don't load them.
    get_context().set_create_repos(false);
}

void RepomanageCommand::configure() {
    auto & context = get_context();
    context.set_load_system_repo(false);

    if (new_option->get_value()) {
        mode = Mode::NEW;
    } else if (old_option->get_value()) {
        mode = Mode::OLD;
    } else {
        // New is the default
        mode = Mode::NEW;
    }

    // Require repodata if we have remote url or if there is local repomd.xml
    repo_with_repodata =
        ((libdnf5::utils::url::is_url(repo_path) && !repo_path.string().starts_with("file://")) ||
         (std::filesystem::exists(repo_path / "repodata/repomd.xml")));

    // Don't load any repositories, even though we don't create repo configurations
    // some plugins could still configure repositoreis. We don't want to load them.
    context.set_load_available_repos(Context::LoadAvailableRepos::NONE);
    context.set_load_system_repo(false);
}

void RepomanageCommand::load_additional_packages() {
    auto & context = get_context();
    if (repo_with_repodata) {
        auto repo_sack = context.get_base().get_repo_sack();
        // In case some other repo was created disable it
        libdnf5::repo::RepoQuery repos(context.get_base());
        for (auto repo : repos) {
            repo->disable();
        }

        // Create and configure repomanage repo (download to temp dir, don't create persistent cache)
        auto repo = repo_sack->create_repo(REPOMANAGE_REPO_NAME);
        repo->get_config().get_baseurl_option().set(repo_path);
        repo->get_config().get_skip_if_unavailable_option().set(false);
        repodata_cache = libdnf5::utils::fs::TempDir(REPOMANAGE_REPO_NAME);
        context.get_base().get_config().get_cachedir_option().set(repodata_cache->get_path());

        // Manually load only our repomanage repo
        repo_sack->load_repos(libdnf5::repo::Repo::Type::AVAILABLE);
    } else {
        // No repodata, gather all rpm files manually
        std::error_code ec;
        std::filesystem::recursive_directory_iterator file_iterator(repo_path, ec);
        if (ec) {
            std::cerr << libdnf5::utils::sformat(
                             _("Failed to create directory '{0}' iterator: {1}"), repo_path.string(), ec.message())
                      << std::endl;
            return;
        }
        std::vector<std::string> found_rpms;
        for (const auto & entry : file_iterator) {
            if (entry.is_regular_file() && entry.path().extension() == ".rpm") {
                found_rpms.push_back(entry.path());
            }
        }
        context.get_base().get_repo_sack()->add_cmdline_packages(found_rpms);
    }
}

void RepomanageCommand::run() {
    auto & ctx = get_context();
    libdnf5::rpm::PackageQuery base_query(ctx.get_base());

    switch (mode) {
        case Mode::NEW:
            base_query.filter_latest_evr(keep_count->get_value());
            break;
        case Mode::OLD:
            base_query.filter_latest_evr(-keep_count->get_value());
            break;
    }

    std::vector<std::string> pkg_locations;
    for (const auto & pkg : base_query) {
        pkg_locations.emplace_back(pkg.get_location());
    }

    std::sort(pkg_locations.begin(), pkg_locations.end(), std::less<std::string>());

    char separator = '\n';
    if (space_option->get_value()) {
        separator = ' ';
    }

    for (const auto & pkg_loc : pkg_locations) {
        if (repo_with_repodata) {
            printf("%s%c", (repo_path / pkg_loc).c_str(), separator);
        } else {
            printf("%s%c", pkg_loc.c_str(), separator);
        }
    }
}
}  // namespace dnf5
