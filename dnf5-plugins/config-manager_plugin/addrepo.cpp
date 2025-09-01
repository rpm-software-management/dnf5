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

#include "addrepo.hpp"

#include "shared.hpp"

#include <curl/curl.h>
#include <fmt/format.h>
#include <libdnf5/repo/file_downloader.hpp>
#include <libdnf5/utils/bgettext/bgettext-mark-domain.h>
#include <zlib.h>

#include <regex>
#include <string_view>

namespace dnf5 {

using namespace libdnf5;

namespace {

// Extracts a specific part of the URL from a URL string.
// Returns an empty string if the URL is not valid/supported or the required part is not present.
std::string get_url_part(const std::string & url, CURLUPart what_part) {
    std::string ret;
    CURLUcode rc;
    CURLU * c_url = curl_url();
    rc = curl_url_set(c_url, CURLUPART_URL, url.c_str(), 0);
    if (!rc) {
        char * part;
        rc = curl_url_get(c_url, what_part, &part, 0);
        if (!rc) {
            ret = part;
            curl_free(part);
        }
    }
    curl_url_cleanup(c_url);
    return ret;
}


// Converts all letters consider illegal in repository id to their "_XX" versions (XX - hex code).
std::string escape(const std::string & text) {
    static constexpr const char * digits = "0123456789ABCDEF";
    char tmp[] = "_XX";
    std::string ret;
    ret.reserve(text.size() * 3);
    for (const char ch : text) {
        if ((ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') || ch == '-' ||
            ch == '.' || ch == ':' || ch == '_') {
            ret += ch;
        } else {
            const auto uch = static_cast<unsigned char>(ch);
            tmp[2] = digits[uch & 0x0F];
            tmp[1] = digits[(uch >> 4) & 0x0F];
            ret += tmp;
        }
    }
    return ret;
}


// Regular expressions to sanitise repository id
const std::regex RE_SCHEME{R"(^\w+:/*(\w+:|www\.)?)"};
const std::regex RE_SLASH{R"([?/:&#|~\*\[\]\(\)'\\]+)"};
const std::regex RE_BEGIN{"^[,.]*"};
const std::regex RE_FINAL{"[,.]*$"};

// Generates a repository id from a URL.
// Strips dangerous and common characters, encodes some characters and limits the length.
std::string generate_repoid_from_url(const std::string & url) {
    std::string ret;
    ret = std::regex_replace(url, RE_SCHEME, "");
    ret = std::regex_replace(ret, RE_SLASH, "_");
    ret = std::regex_replace(ret, RE_BEGIN, "");
    ret = std::regex_replace(ret, RE_FINAL, "");
    ret = escape(ret);

    // Limits the length of the repository id.
    // Copies the first and last 100 characters. The substring in between is replaced by a crc32 hash.
    if (ret.size() > 250) {
        size_t sz = ret.size();
        ret = fmt::format(
            "{}-{:08X}-{}",
            ret.substr(0, 100),
            crc32_z(0, ((const unsigned char *)ret.c_str()) + 100, sz - 200),
            ret.substr(sz - 100));
    }

    return ret;
}

}  // namespace


void ConfigManagerAddRepoCommand::set_argument_parser() {
    auto & ctx = get_context();
    auto & parser = ctx.get_argument_parser();

    auto & cmd = *get_argument_parser_command();
    cmd.set_description(
        "Add repositories from the specified configuration file or define a new repository using user options");
    cmd.set_long_description(
        "Add repositories from the specified configuration file or define a new repository using user options.");

    auto from_repofile_opt = parser.add_new_named_arg("from-repofile");
    from_repofile_opt->set_long_name("from-repofile");
    from_repofile_opt->set_description("Download repository configuration file, test it and put it in reposdir");
    from_repofile_opt->set_has_value(true);
    from_repofile_opt->set_arg_value_help("REPO_CONFIGURATION_FILE_URL");
    from_repofile_opt->set_parse_hook_func([this](cli::ArgumentParser::NamedArg *, const char *, const char * value) {
        source_repofile.location = value;
        // If the URL is invalid. Scheme not found. It can be a path to a local file.
        source_repofile.is_local_path = get_url_part(source_repofile.location, CURLUPART_SCHEME) == "";
        if (source_repofile.is_local_path) {
            // Tests whether it is really a path to an existing local file.
            try {
                if (!std::filesystem::exists(source_repofile.location)) {
                    throw ConfigManagerError(M_("from-repofile: \"{}\" file does not exist"), source_repofile.location);
                }
            } catch (const std::filesystem::filesystem_error & ex) {
                throw ConfigManagerError(M_("from-repofile: {}"), std::string{ex.what()});
            }
        }
        return true;
    });
    cmd.register_named_arg(from_repofile_opt);

    auto repo_id_opt = parser.add_new_named_arg("id");
    repo_id_opt->set_long_name("id");
    repo_id_opt->set_description("Set id for newly created repository");
    repo_id_opt->set_has_value(true);
    repo_id_opt->set_arg_value_help("REPO_ID");
    repo_id_opt->set_parse_hook_func([this](cli::ArgumentParser::NamedArg *, const char *, const char * value) {
        repo_id = value;
        return true;
    });
    cmd.register_named_arg(repo_id_opt);

    auto set_opt = parser.add_new_named_arg("set");
    set_opt->set_long_name("set");
    set_opt->set_description("Set option in newly created repository");
    set_opt->set_has_value(true);
    set_opt->set_arg_value_help("REPO_OPTION=VALUE");
    set_opt->set_parse_hook_func([this](
                                     [[maybe_unused]] cli::ArgumentParser::NamedArg * arg,
                                     [[maybe_unused]] const char * option,
                                     const char * value) {
        auto val = strchr(value + 1, '=');
        if (!val) {
            throw cli::ArgumentParserError(
                M_("{}: Badly formatted argument value \"{}\""), std::string{"set"}, std::string{value});
        }
        std::string key{value, val};
        std::string key_value{val + 1};

        // Test if the repository option can be set.
        try {
            tmp_repo_conf.opt_binds().at(key).new_string(Option::Priority::RUNTIME, key_value);
        } catch (const Error & ex) {
            throw ConfigManagerError(
                M_("Cannot set repository option \"{}={}\": {}"), key, key_value, std::string{ex.what()});
        }

        // Save the repo option for later writing to a file.
        const auto [it, inserted] = repo_opts.insert({key, key_value});
        if (!inserted) {
            if (it->second != key_value) {
                throw ConfigManagerError(
                    M_("Sets the \"{}\" option again with a different value: \"{}\" != \"{}\""),
                    key,
                    it->second,
                    key_value);
            }
        }
        return true;
    });
    cmd.register_named_arg(set_opt);

    auto add_or_replace = parser.add_new_named_arg("add-or-replace");
    add_or_replace->set_long_name("add-or-replace");
    add_or_replace->set_description("Allow adding or replacing a repository in the existing configuration file");
    add_or_replace->set_has_value(false);
    add_or_replace->set_parse_hook_func([this](cli::ArgumentParser::NamedArg *, const char *, const char *) {
        file_policy = FilePolicy::ADD_OR_REPLACE;
        return true;
    });
    cmd.register_named_arg(add_or_replace);

    auto create_missing_dirs_opt = parser.add_new_named_arg("create-missing-dir");
    create_missing_dirs_opt->set_long_name("create-missing-dir");
    create_missing_dirs_opt->set_description("Allow creation of missing directories");
    create_missing_dirs_opt->set_has_value(false);
    create_missing_dirs_opt->set_parse_hook_func([this](cli::ArgumentParser::NamedArg *, const char *, const char *) {
        create_missing_dirs = true;
        return true;
    });
    cmd.register_named_arg(create_missing_dirs_opt);

    auto overwrite_opt = parser.add_new_named_arg("overwrite");
    overwrite_opt->set_long_name("overwrite");
    overwrite_opt->set_description("Allow overwriting of existing repository configuration file");
    overwrite_opt->set_has_value(false);
    overwrite_opt->set_parse_hook_func([this](cli::ArgumentParser::NamedArg *, const char *, const char *) {
        file_policy = FilePolicy::OVERWRITE;
        return true;
    });
    cmd.register_named_arg(overwrite_opt);

    auto save_filename_opt = parser.add_new_named_arg("save-filename");
    save_filename_opt->set_long_name("save-filename");
    save_filename_opt->set_description(
        "Set the name of the configuration file of the added repository. The \".repo\" extension is added if it is "
        "missing.");
    save_filename_opt->set_has_value(true);
    save_filename_opt->set_arg_value_help("FILENAME");
    save_filename_opt->set_parse_hook_func([this](cli::ArgumentParser::NamedArg *, const char *, const char * value) {
        save_filename = value;
        return true;
    });
    cmd.register_named_arg(save_filename_opt);

    // Set conflicting arguments
    add_or_replace->add_conflict_argument(*from_repofile_opt);
    repo_id_opt->add_conflict_argument(*from_repofile_opt);
    set_opt->add_conflict_argument(*from_repofile_opt);
}


void ConfigManagerAddRepoCommand::configure() {
    auto & ctx = get_context();
    auto & base = ctx.get_base();

    const auto & repo_dirs = base.get_config().get_reposdir_option().get_value();
    if (repo_dirs.empty()) {
        throw ConfigManagerError(M_("Missing path to repository configuration directory"));
    }
    std::filesystem::path dest_repo_dir = repo_dirs.front();
    resolve_missing_dir(dest_repo_dir, create_missing_dirs);

    if (source_repofile.location.empty()) {
        create_repo(repo_id, repo_opts, dest_repo_dir);
    } else {
        add_repos_from_repofile(source_repofile, dest_repo_dir);
    }
}


void ConfigManagerAddRepoCommand::add_repos_from_repofile(
    const SourceRepofile & source_repofile, const std::filesystem::path & dest_repo_dir) {
    auto & ctx = get_context();
    auto & base = ctx.get_base();
    auto logger = base.get_logger();

    if (save_filename.empty()) {
        if (source_repofile.is_local_path) {
            save_filename = std::filesystem::path(source_repofile.location).filename();
        } else {
            save_filename = std::filesystem::path(get_url_part(source_repofile.location, CURLUPART_PATH)).filename();
        }
    }
    if (!save_filename.ends_with(".repo")) {
        save_filename += ".repo";
    }
    auto dest_path = dest_repo_dir / save_filename;

    test_if_filepath_not_exist(dest_path, false);

    // Creates an open temporary file. It then closes it but does not remove it.
    // In the following code, this temporary file is used to store the copied/downloaded configuration.
    auto tmpfilepath = dest_path.string() + ".XXXXXX";
    auto fd = mkstemp(tmpfilepath.data());
    if (fd == -1) {
        throw std::filesystem::filesystem_error(
            "cannot create temporary file", tmpfilepath, std::error_code(errno, std::system_category()));
    }
    close(fd);

    try {
        if (source_repofile.is_local_path) {
            try {
                std::filesystem::copy_file(
                    source_repofile.location, tmpfilepath, std::filesystem::copy_options::overwrite_existing);
            } catch (const std::filesystem::filesystem_error & e) {
                throw ConfigManagerError(
                    M_("Failed to copy repository configuration file \"{}\": {}"),
                    source_repofile.location,
                    std::string{e.what()});
            }
        } else {
            try {
                repo::FileDownloader downloader(base);
                downloader.add(source_repofile.location, tmpfilepath);
                downloader.download();
            } catch (const repo::FileDownloadError & e) {
                throw ConfigManagerError(
                    M_("Failed to download repository configuration file \"{}\": {}"),
                    source_repofile.location,
                    std::string{e.what()});
            }
        }

        ConfigParser parser;
        parser.read(tmpfilepath);
        std::vector<std::string> repo_ids;
        repo_ids.reserve(parser.get_data().size());
        for (const auto & [repo_id, opts] : parser.get_data()) {
            repo_ids.emplace_back(repo_id);
        }
        test_if_ids_not_already_exist(repo_ids, dest_path);

        // Test if the repository options can be set.
        for (const auto & [repo_id, repo_opts] : parser.get_data()) {
            for (const auto & [key, key_val] : repo_opts) {
                // Skip empty lines and comment lines (ConfigParser stores the empty line and comment line
                // in an automatically generated key whose name starts with the '#' character).
                if (key.starts_with('#')) {
                    continue;
                }

                try {
                    tmp_repo_conf.opt_binds().at(key).new_string(Option::Priority::RUNTIME, key_val);
                } catch (const Error & ex) {
                    throw ConfigManagerError(
                        M_("Error in added repository configuration file. Cannot set repository option \"{}={}\": {}"),
                        key,
                        key_val,
                        std::string{ex.what()});
                }
            }
        }
    } catch (const Error & ex) {
        std::error_code ec;
        std::filesystem::remove(tmpfilepath, ec);
        throw;
    }

    // All tests passed. Renames the configuration file to the final name.
    std::filesystem::rename(tmpfilepath, dest_path);
    logger->info("config-manager: Added repofile \"{}\" from \"{}\"", dest_path.string(), source_repofile.location);
    set_file_permissions(dest_path);
}


void ConfigManagerAddRepoCommand::create_repo(
    std::string repo_id,
    const std::map<std::string, std::string> & repo_opts,
    const std::filesystem::path & dest_repo_dir) {
    auto & ctx = get_context();
    auto & base = ctx.get_base();
    auto logger = base.get_logger();

    // Test for presence of required arguments.
    // And sets the URL - used to create the repository ID if not specified.
    std::string url;
    if (const auto it = repo_opts.find("baseurl"); it != repo_opts.end() && !it->second.empty()) {
        const auto urls = OptionStringList(std::vector<std::string>{}).from_string(it->second);
        if (urls.empty() || (url = urls.front()).empty()) {
            throw ConfigManagerError(M_("Bad baseurl: {}={}"), it->first, it->second);
        }
    } else if (const auto it = repo_opts.find("mirrorlist"); it != repo_opts.end() && !it->second.empty()) {
        url = it->second;
    } else if (const auto it = repo_opts.find("metalink"); it != repo_opts.end() && !it->second.empty()) {
        url = it->second;
    } else {
        throw cli::ArgumentParserMissingDependentArgumentError(
            M_("One of --from-repofile=<URL>, --set=baseurl=<URL>, --set=mirrorlist=<URL>, --set=metalink=<URL> "
               "must be set to a non-empty URL"));
    }

    if (repo_id.empty()) {
        repo_id = generate_repoid_from_url(url);
    }

    if (save_filename.empty()) {
        save_filename = repo_id;
    }
    if (!save_filename.ends_with(".repo")) {
        save_filename += ".repo";
    }
    auto dest_path = dest_repo_dir / save_filename;

    test_if_filepath_not_exist(dest_path, true);
    test_if_ids_not_already_exist({repo_id}, dest_path);

    ConfigParser parser;

    if (file_policy == FilePolicy::ADD_OR_REPLACE && std::filesystem::exists(dest_path)) {
        parser.read(dest_path);
        if (parser.has_section(repo_id)) {
            // If the repository with the id already exists, it will be removed.
            parser.remove_section(repo_id);
        }
    }

    parser.add_section(repo_id);

    // Sets the default repository name. May be overwritten with "--set=name=<name>".
    parser.set_value(repo_id, "name", repo_id + " - Created by dnf5 config-manager");
    // Enables repository by default. The repository can be disabled with "--set=enabled=0".
    parser.set_value(repo_id, "enabled", "1");

    for (const auto & [key, key_val] : repo_opts) {
        parser.set_value(repo_id, key, key_val);
    }

    try {
        parser.write(dest_path, false);
        logger->info("config-manager: Added new repo \"{}\" to file \"{}\"", repo_id, dest_path.string());
    } catch (const std::runtime_error & e) {
        throw ConfigManagerError(
            M_("Failed to save repository configuration file \"{}\": {}"), dest_path.native(), std::string{e.what()});
    }
    set_file_permissions(dest_path);
}


void ConfigManagerAddRepoCommand::test_if_filepath_not_exist(
    const std::filesystem::path & path, bool show_hint_add_or_replace) const {
    if (file_policy == FilePolicy::ERROR && std::filesystem::exists(path)) {
        ConfigParser parser;
        parser.read(path);
        std::string repo_ids;
        bool first{true};
        for (const auto & [repo_id, opts] : parser.get_data()) {
            if (first) {
                first = false;
            } else {
                repo_ids += ' ';
            }
            repo_ids += repo_id;
        }
        constexpr BgettextMessage msg1 =
            M_("File \"{}\" already exists and configures repositories with IDs \"{}\"."
               " Add \"--add-or-replace\" or \"--overwrite\".");
        constexpr BgettextMessage msg2 =
            M_("File \"{}\" already exists and configures repositories with IDs \"{}\"."
               " Add \"--overwrite\" to overwrite.");
        throw ConfigManagerError(show_hint_add_or_replace ? msg1 : msg2, path.string(), repo_ids);
    }
}


void ConfigManagerAddRepoCommand::test_if_ids_not_already_exist(
    const std::vector<std::string> & repo_ids, const std::filesystem::path & ignore_path) const {
    auto & ctx = get_context();
    auto & base = ctx.get_base();
    auto logger = base.get_logger();

    // The repository can also be defined in the main configuration file.
    if (const auto & conf_path = get_config_file_path(base.get_config()); std::filesystem::exists(conf_path)) {
        ConfigParser parser;
        parser.read(conf_path);
        for (const auto & repo_id : repo_ids) {
            if (parser.has_section(repo_id)) {
                throw ConfigManagerError(
                    M_("A repository with id \"{}\" already configured in file: {}"), repo_id, conf_path.string());
            }
        }
    }

    const auto & repo_dirs = base.get_config().get_reposdir_option().get_value();
    for (const std::filesystem::path dir : repo_dirs) {
        if (std::filesystem::exists(dir)) {
            std::error_code ec;
            std::filesystem::directory_iterator di(dir, ec);
            if (ec) {
                write_warning(
                    *logger, M_("Cannot read repositories from directory \"{}\": {}"), dir.string(), ec.message());
                continue;
            }
            for (auto & dentry : di) {
                const auto & path = dentry.path();
                if (path == ignore_path) {
                    continue;
                }
                if (path.extension() == ".repo") {
                    ConfigParser parser;
                    parser.read(path);
                    for (const auto & repo_id : repo_ids) {
                        if (parser.has_section(repo_id)) {
                            throw ConfigManagerError(
                                M_("A repository with id \"{}\" already configured in file: {}"),
                                repo_id,
                                path.string());
                        }
                    }
                }
            }
        }
    }
}

}  // namespace dnf5
