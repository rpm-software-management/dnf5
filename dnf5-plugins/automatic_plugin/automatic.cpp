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

#include "automatic.hpp"

#include "download_callbacks_simple.hpp"
#include "emitters.hpp"
#include "transaction_callbacks_simple.hpp"

#include <curl/curl.h>
#include <libdnf5-cli/output/transaction_table.hpp>
#include <libdnf5/conf/const.hpp>
#include <libdnf5/repo/package_downloader.hpp>
#include <libdnf5/rpm/package_query.hpp>
#include <libdnf5/rpm/package_set.hpp>
#include <libdnf5/utils/bgettext/bgettext-mark-domain.h>
#include <libdnf5/utils/format.hpp>
#include <libsmartcols/libsmartcols.h>
#include <netdb.h>
#include <stdlib.h>

#include <chrono>
#include <filesystem>
#include <iostream>
#include <random>
#include <set>
#include <string>
#include <thread>

namespace {

/// Sleep for random number of seconds in interval <0, max_value>
static void random_wait(unsigned int max_value) {
    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_int_distribution<unsigned int> distribution(0U, max_value);

    sleep(distribution(rng));
}

static bool reboot_needed(libdnf5::Base & base, const libdnf5::base::Transaction & transaction) {
    libdnf5::rpm::PackageSet transaction_packages(base);
    for (const auto & pkg : transaction.get_transaction_packages()) {
        transaction_packages.add(pkg.get_package());
    }
    libdnf5::rpm::PackageQuery reboot_suggested(transaction_packages);
    reboot_suggested.filter_reboot_suggested();
    return !reboot_suggested.empty();
}

static bool server_available(std::string_view host, std::string_view service) {
    // Resolve host name/service to IP address/port
    struct addrinfo * server_info;
    if (getaddrinfo(host.data(), service.data(), nullptr, &server_info) != 0) {
        return false;
    }

    // Create a socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        freeaddrinfo(server_info);
        return false;
    }

    // Attempt to connect to the server
    bool retval = true;
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    if (connect(sockfd, server_info->ai_addr, server_info->ai_addrlen) < 0) {
        retval = false;
    }

    close(sockfd);
    freeaddrinfo(server_info);
    return retval;
}

}  // namespace


namespace dnf5 {

void AutomaticCommand::wait_for_network() {
    auto timeout = config_automatic.config_commands.network_online_timeout.get_value();
    if (timeout <= 0) {
        return;
    }

    auto & context = get_context();
    auto & base = context.base;
    auto & logger = *base.get_logger();
    logger.debug("Waiting for internet connection...");

    const auto time_0 = std::chrono::system_clock::now();
    const auto time_end = time_0 + std::chrono::seconds(timeout);

    // collect repository addresses to check network availability on
    libdnf5::repo::RepoQuery repos(base);
    repos.filter_enabled(true);
    std::vector<std::string> urls{};
    for (const auto & repo : repos) {
        auto proxy = repo->get_config().get_proxy_option();
        if (!proxy.empty() && !proxy.get_value().empty()) {
            urls.emplace_back(proxy.get_value());
            continue;
        }
        auto mirrorlist = repo->get_config().get_mirrorlist_option();
        if (!mirrorlist.empty() && !mirrorlist.get_value().empty()) {
            urls.emplace_back(mirrorlist.get_value());
        }
        auto metalink = repo->get_config().get_metalink_option();
        if (!metalink.empty() && !metalink.get_value().empty()) {
            urls.emplace_back(metalink.get_value());
        }
        auto baseurl = repo->get_config().get_baseurl_option();
        if (!baseurl.empty()) {
            for (auto u : baseurl.get_value()) {
                if (!u.empty()) {
                    urls.emplace_back(std::move(u));
                }
            }
        }
    }

    // parse url to [(host, service(port number or scheme))] using libcurl
    std::vector<std::tuple<std::string, std::string>> addresses;
    CURLUcode rc;
    CURLU * url = curl_url();
    unsigned int set_flags = CURLU_NON_SUPPORT_SCHEME;
    unsigned int get_flags = 0;
    for (auto & u : urls) {
        rc = curl_url_set(url, CURLUPART_URL, u.c_str(), set_flags);
        if (!rc) {
            std::string host;
            std::string port;
            char * val = nullptr;
            rc = curl_url_get(url, CURLUPART_HOST, &val, get_flags);
            if (rc != CURLUE_OK) {
                continue;
            }
            host = val;
            curl_free(val);
            // service is port or (if not given) scheme
            rc = curl_url_get(url, CURLUPART_PORT, &val, get_flags);
            if (rc == CURLUE_OK) {
                port = val;
            } else {
                rc = curl_url_get(url, CURLUPART_SCHEME, &val, get_flags);
                if (rc != CURLUE_OK) {
                    continue;
                }
                port = val;
            }
            curl_free(val);
            addresses.emplace_back(std::move(host), std::move(port));
        }
    }
    curl_url_cleanup(url);

    if (addresses.size() == 0) {
        // do not have any address to check availability for
        return;
    }

    while (std::chrono::system_clock::now() < time_end) {
        for (auto const & [host, service] : addresses) {
            if (server_available(host, service)) {
                return;
            }
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    logger.warning("No network connection detected.");
}

void AutomaticCommand::set_parent_command() {
    auto * arg_parser_parent_cmd = get_session().get_argument_parser().get_root_command();
    auto * arg_parser_this_cmd = get_argument_parser_command();
    arg_parser_parent_cmd->register_command(arg_parser_this_cmd);
}

void AutomaticCommand::set_argument_parser() {
    auto & cmd = *get_argument_parser_command();
    cmd.set_long_description(
        _("An alternative CLI to 'dnf upgrade' suitable to be executed automatically and regularly."));

    auto & context = get_context();
    auto & parser = context.get_argument_parser();

    timer = std::make_unique<libdnf5::cli::session::BoolOption>(
        *this, "timer", '\0', _("Apply random delay before execution."), false);
    auto downloadupdates = std::make_unique<libdnf5::cli::session::BoolOption>(
        *this,
        "downloadupdates",
        '\0',
        _("Automatically download updated packages"),
        false,
        &config_automatic.config_commands.download_updates);
    auto nodownloadupdates = std::make_unique<libdnf5::cli::session::BoolOption>(
        *this,
        "no-downloadupdates",
        '\0',
        _("Do not automatically download updated packages"),
        true,
        &config_automatic.config_commands.download_updates);
    // TODO(mblaha): there is inconsistency in naming between
    // --(no-)installupdates CLI option which overrides `apply_updates` config option
    auto installupdates = std::make_unique<libdnf5::cli::session::BoolOption>(
        *this,
        "installupdates",
        '\0',
        _("Automatically install downloaded updates"),
        false,
        &config_automatic.config_commands.apply_updates);
    auto noinstallupdates = std::make_unique<libdnf5::cli::session::BoolOption>(
        *this,
        "no-installupdates",
        '\0',
        _("Do not automatically install downloaded updates"),
        true,
        &config_automatic.config_commands.apply_updates);

    // downloadupdates and no-downloadupdates options conflict with each other.
    {
        auto conflicts =
            parser.add_conflict_args_group(std::make_unique<std::vector<libdnf5::cli::ArgumentParser::Argument *>>());
        conflicts->push_back(nodownloadupdates->arg);
        downloadupdates->arg->set_conflict_arguments(conflicts);
    }
    // installupdates and no-installupdates options conflict with each other.
    // installupdates and no-downloadupdates options conflict with each other.
    {
        auto conflicts =
            parser.add_conflict_args_group(std::make_unique<std::vector<libdnf5::cli::ArgumentParser::Argument *>>());
        conflicts->push_back(downloadupdates->arg);
        conflicts->push_back(installupdates->arg);
        nodownloadupdates->arg->set_conflict_arguments(conflicts);
    }
    {
        auto conflicts =
            parser.add_conflict_args_group(std::make_unique<std::vector<libdnf5::cli::ArgumentParser::Argument *>>());
        conflicts->push_back(noinstallupdates->arg);
        conflicts->push_back(nodownloadupdates->arg);
        installupdates->arg->set_conflict_arguments(conflicts);
    }
    {
        auto conflicts =
            parser.add_conflict_args_group(std::make_unique<std::vector<libdnf5::cli::ArgumentParser::Argument *>>());
        conflicts->push_back(installupdates->arg);
        noinstallupdates->arg->set_conflict_arguments(conflicts);
    }
}

void AutomaticCommand::pre_configure() {
    auto & context = get_context();
    auto & base = context.base;

    auto random_sleep = config_automatic.config_commands.random_sleep.get_value();
    if (timer->get_value() && random_sleep > 0) {
        random_wait(random_sleep);
    }

    auto download_callbacks_uptr = std::make_unique<dnf5::DownloadCallbacksSimple>(output_stream);
    base.set_download_callbacks(std::move(download_callbacks_uptr));
    download_callbacks_set = true;

    // read the config file, use the first existing file in following locations:
    //      - [installroot]/etc/dnf/dnf5-plugins/automatic.conf
    //      - [installroot]/usr/share/dnf5/d  nf5-plugins/automatic.conf
    auto & main_config = base.get_config();
    bool use_host_config{main_config.get_use_host_config_option().get_value()};
    std::filesystem::path installroot_path{main_config.get_installroot_option().get_value()};
    std::vector<std::filesystem::path> possible_paths{"/etc/dnf/dnf5-plugins", "/usr/share/dnf5/dnf5-plugins"};
    for (const auto & pth : possible_paths) {
        std::filesystem::path conf_file_path{pth / "automatic.conf"};
        if (!use_host_config) {
            conf_file_path = installroot_path / conf_file_path.relative_path();
        }
        if (std::filesystem::exists(conf_file_path)) {
            libdnf5::ConfigParser parser;
            parser.read(conf_file_path);
            base.get_config().load_from_parser(
                parser, "base", *base.get_vars(), *base.get_logger(), libdnf5::Option::Priority::AUTOMATICCONFIG);
            config_automatic.load_from_parser(parser, *base.get_vars(), *base.get_logger());
            break;
        }
    }

    context.set_output_stream(output_stream);
}

void AutomaticCommand::configure() {
    auto & context = get_context();
    context.set_load_system_repo(true);
    context.update_repo_metadata_from_advisory_options(
        {}, config_automatic.config_commands.upgrade_type.get_value() == "security", false, false, false, {}, {}, {});
    context.set_load_available_repos(Context::LoadAvailableRepos::ENABLED);

    wait_for_network();
}

void AutomaticCommand::run() {
    auto & context = get_context();
    auto & base = context.base;
    bool success = true;

    // setup upgrade transaction goal
    auto settings = libdnf5::GoalJobSettings();

    // TODO(mblaha): Use advisory_query_from_cli_input from dnf5/commands/advisory_shared.hpp?
    if (config_automatic.config_commands.upgrade_type.get_value() == "security") {
        auto advisories = libdnf5::advisory::AdvisoryQuery(base);
        advisories.filter_type("security");
        settings.set_advisory_filter(advisories);
        // TODO(mblaha): set also minimal=true?
    }
    libdnf5::Goal goal(base);
    goal.add_rpm_upgrade(settings);

    auto transaction = goal.resolve();

    // print resolve logs and the transaction table to the output stream
    {
        output_stream << std::endl << _("Resolved transaction:") << std::endl;
        libdnf5::cli::output::print_resolve_logs(transaction, output_stream);

        if (!transaction.empty()) {
            libdnf5::cli::output::TransactionSummary summary;
            auto tt = libdnf5::cli::output::create_transaction_table(transaction, summary);
            scols_table_enable_colors(*tt, false);
            scols_table_set_termwidth(*tt, 80);
            char * tt_string = nullptr;
            scols_print_table_to_string(*tt, &tt_string);
            output_stream << tt_string << std::endl;
            free(tt_string);

            summary.print(output_stream);
        }
    }

    bool do_reboot = false;
    if (!transaction.empty()) {
        auto download_updates = config_automatic.config_commands.download_updates.get_value();
        auto apply_updates = config_automatic.config_commands.apply_updates.get_value();
        if (download_updates || apply_updates) {
            output_stream << _("Downloading packages:") << std::endl;
            try {
                transaction.download();
            } catch (const libdnf5::repo::PackageDownloadError & e) {
                success = false;
            } catch (const libdnf5::repo::RepoCacheonlyError & e) {
                success = false;
                output_stream << e.what() << std::endl;
            }
            if (success) {
                output_stream << _("Packages downloaded.") << std::endl;
                // TODO: handle downloadonly config option
                if (apply_updates) {
                    output_stream << _("Running transaction:") << std::endl;
                    transaction.set_callbacks(std::make_unique<TransactionCallbacksSimple>(context, output_stream));
                    transaction.set_description(context.get_cmdline());
                    auto comment = context.get_comment();
                    if (comment) {
                        transaction.set_comment(comment);
                    }
                    auto result = transaction.run();
                    if (result == libdnf5::base::Transaction::TransactionRunResult::SUCCESS) {
                        auto reboot = config_automatic.config_commands.reboot.get_value();
                        if (reboot == "when-changed" ||
                            (reboot == "when-needed" and reboot_needed(base, transaction))) {
                            do_reboot = true;
                        }
                        output_stream << _("Transaction finished.") << std::endl;
                    } else {
                        output_stream << _("Transaction failed: ")
                                      << libdnf5::base::Transaction::transaction_result_to_string(result) << std::endl;
                        for (auto const & entry : transaction.get_gpg_signature_problems()) {
                            output_stream << entry << std::endl;
                        }
                        for (auto & problem : transaction.get_transaction_problems()) {
                            output_stream << "  - " << problem << std::endl;
                        }
                        success = false;
                    }
                }
            }
        }
    }

    for (const auto & emitter_name : config_automatic.config_emitters.emit_via.get_value()) {
        std::unique_ptr<Emitter> emitter;
        if (emitter_name == "stdio") {
            emitter = std::make_unique<EmitterStdIO>(config_automatic, transaction, output_stream, success);
        } else if (emitter_name == "motd") {
            emitter = std::make_unique<EmitterMotd>(config_automatic, transaction, output_stream, success);
        } else if (emitter_name == "command") {
            emitter = std::make_unique<EmitterCommand>(config_automatic, transaction, output_stream, success);
        } else if (emitter_name == "command_email") {
            emitter = std::make_unique<EmitterCommandEmail>(config_automatic, transaction, output_stream, success);
        } else if (emitter_name == "email") {
            emitter = std::make_unique<EmitterEmail>(config_automatic, transaction, output_stream, success);
        } else {
            auto & logger = *base.get_logger();
            logger.warning(_("Unknown report emitter for dnf5 automatic: \"{}\"."), emitter_name);
            continue;
        }
        emitter->notify();
    }

    if (!success) {
        throw libdnf5::cli::SilentCommandExitError(1);
    }

    if (do_reboot) {
        auto reboot_command = config_automatic.config_commands.reboot_command.get_value();
        int rc = system(reboot_command.c_str());
        if (rc != 0) {
            throw libdnf5::cli::CommandExitError(
                1, M_("Error: reboot command returned nonzero exit code: {}"), WEXITSTATUS(rc));
        }
    }
}

AutomaticCommand::~AutomaticCommand() {
    auto & context = get_context();
    // dnf5::DownloadCallbacksSimple is part of the automatic.so plugin library, which
    // gets unloaded during ~Context. However, download_callback is destructed later,
    // during ~Base, resulting in a segmentation fault. Therefore, we need to reset
    // download_callbacks manually.
    if (download_callbacks_set) {
        context.base.set_download_callbacks(nullptr);
    }
}

}  // namespace dnf5
