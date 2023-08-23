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

#include <libdnf5-cli/output/transaction_table.hpp>
#include <libdnf5/conf/const.hpp>
#include <libdnf5/repo/package_downloader.hpp>
#include <libdnf5/utils/bgettext/bgettext-mark-domain.h>
#include <libdnf5/utils/format.hpp>
#include <libsmartcols/libsmartcols.h>
#include <stdlib.h>

#include <iostream>
#include <random>
#include <set>
#include <string>

namespace {

/// Sleep for random number of seconds in interval <0, max_value>
static void random_wait(int max_value) {
    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_int_distribution<int> distribution(0, max_value);

    sleep(distribution(rng));
}

static bool reboot_needed(const libdnf5::base::Transaction & transaction) {
    static const std::set<std::string> need_reboot = {
        "kernel", "kernel-rt", "glibc", "linux-firmware", "systemd", "dbus", "dbus-broker", "dbus-daemon"};
    for (const auto & pkg : transaction.get_transaction_packages()) {
        if (need_reboot.find(pkg.get_package().get_name()) != need_reboot.end()) {
            return true;
        }
    }
    return false;
}

}  // namespace


namespace dnf5 {

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

    auto arg_config_file = parser.add_new_positional_arg(
        "config_path", libdnf5::cli::ArgumentParser::PositionalArg::OPTIONAL, nullptr, nullptr);
    arg_config_file->set_description(_("Path to dnf5-automatic config file."));
    arg_config_file->set_parse_hook_func(
        [&]([[maybe_unused]] libdnf5::cli::ArgumentParser::PositionalArg * arg, int argc, const char * const argv[]) {
            if (argc > 0) {
                config_automatic.automatic_config_file_path.set(argv[0]);
            }
            return true;
        });
    cmd.register_positional_arg(arg_config_file);

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

    // TODO wait for network

    auto random_sleep = config_automatic.config_commands.random_sleep.get_value();
    if (timer->get_value() && random_sleep > 0) {
        random_wait(random_sleep);
    }

    auto download_callbacks_uptr = std::make_unique<dnf5::DownloadCallbacksSimple>(output_stream);
    base.set_download_callbacks(std::move(download_callbacks_uptr));
    download_callbacks_set = true;

    libdnf5::ConfigParser parser;
    parser.read(config_automatic.automatic_config_file_path.get_value());
    base.get_config().load_from_parser(
        parser, "base", *base.get_vars(), *base.get_logger(), libdnf5::Option::Priority::AUTOMATICCONFIG);
    config_automatic.load_from_parser(parser, *base.get_vars(), *base.get_logger());

    context.set_output_stream(output_stream);
}

void AutomaticCommand::configure() {
    auto & context = get_context();
    context.set_load_system_repo(true);
    context.update_repo_metadata_from_advisory_options(
        {}, config_automatic.config_commands.upgrade_type.get_value() == "security", false, false, false, {}, {}, {});
    context.set_load_available_repos(Context::LoadAvailableRepos::ENABLED);
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
                    transaction.set_callbacks(std::make_unique<TransactionCallbacksSimple>(output_stream));
                    transaction.set_description(context.get_cmdline());
                    auto comment = context.get_comment();
                    if (comment) {
                        transaction.set_comment(comment);
                    }
                    auto result = transaction.run();
                    if (result == libdnf5::base::Transaction::TransactionRunResult::SUCCESS) {
                        auto reboot = config_automatic.config_commands.reboot.get_value();
                        if (reboot == "when-changed" || (reboot == "when-needed" and reboot_needed(transaction))) {
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
