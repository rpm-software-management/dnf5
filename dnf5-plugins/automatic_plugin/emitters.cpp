/*
Copyright (C) 2022 Red Hat, Inc.

This file is part of libdnf: https://github.com/rpm-software-management/dnf5/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "emitters.hpp"

#include "email_message.hpp"

#include <curl/curl.h>
#include <libdnf5/base/transaction_package.hpp>
#include <libdnf5/utils/bgettext/bgettext-lib.h>
#include <libdnf5/utils/format.hpp>
#include <stdio.h>
#include <string.h>

#include <cstdio>
#include <fstream>
#include <iomanip>
#include <iostream>

namespace dnf5 {

constexpr const char * MOTD_FILENAME = "/etc/motd.d/dnf5-automatic";
enum class AutomaticStage { CHECK, DOWNLOAD, APPLY };

int Emitter::upgrades_count() {
    int count = 0;
    for (const auto & pkg : transaction.get_transaction_packages()) {
        if (transaction_item_action_is_outbound(pkg.get_action())) {
            ++count;
        }
    }
    return count;
}

std::string Emitter::short_message() {
    std::string message;

    auto stage = AutomaticStage::CHECK;
    if (config_automatic.config_commands.apply_updates.get_value()) {
        stage = AutomaticStage::APPLY;
    } else if (config_automatic.config_commands.download_updates.get_value()) {
        stage = AutomaticStage::DOWNLOAD;
    }

    if (success) {
        if (transaction.empty()) {
            message = _("No new upgrades available.");
        } else {
            switch (stage) {
                case AutomaticStage::CHECK:
                    message = _("{} packages can be upgraded.");
                    break;
                case AutomaticStage::DOWNLOAD:
                    message = _("{} new upgrades have been downloaded.");
                    break;
                case AutomaticStage::APPLY:
                    message = _("{} new upgrades have been installed.");
                    break;
            }
            message = libdnf5::utils::sformat(message, upgrades_count());
        }
    } else {
        switch (stage) {
            case AutomaticStage::CHECK:
                message = _("Failed to check for upgrades.");
                break;
            case AutomaticStage::DOWNLOAD:
                message = _("Failed to download upgrades.");
                break;
            case AutomaticStage::APPLY:
                message = _("Failed to install upgrades.");
                break;
        }
    }
    return message;
}

void EmitterStdIO::notify() {
    std::cout << short_message() << std::endl;
    auto output = output_stream.str();
    if (!output.empty()) {
        std::cout << std::endl;
        std::cout << output;
    }
}

void EmitterMotd::notify() {
    std::ofstream motd_file_stream(MOTD_FILENAME);
    if (!motd_file_stream.is_open()) {
        return;
    }
    motd_file_stream << "dnf5-automatic: " << short_message() << std::endl;
    motd_file_stream.close();
}

std::string quote(std::string_view str) {
    std::ostringstream temp_stream;
    temp_stream << std::quoted(str);
    return temp_stream.str();
}

void EmitterCommand::notify() {
    std::string command_format = config_automatic.config_command.command_format.get_value();

    FILE * command_pipe = popen(command_format.c_str(), "w");
    if (command_pipe) {
        std::string stdin_format = config_automatic.config_command.stdin_format.get_value();
        fputs(libdnf5::utils::sformat(stdin_format, fmt::arg("body", output_stream.str())).c_str(), command_pipe);
        std::fflush(command_pipe);
        pclose(command_pipe);
    }
}

void EmitterCommandEmail::notify() {
    std::string command_format = config_automatic.config_command_email.command_format.get_value();
    std::string email_from = config_automatic.config_command_email.email_from.get_value();
    std::string email_to;
    for (const auto & email : config_automatic.config_command_email.email_to.get_value()) {
        if (!email_to.empty()) {
            email_to += " ";
        }
        email_to += email;
    }
    std::string subject = libdnf5::utils::sformat(
        _("[{}] dnf5-automatic: {}"), config_automatic.config_emitters.system_name.get_value(), short_message());

    std::string command_string = libdnf5::utils::sformat(
        command_format,
        fmt::arg("body", quote(output_stream.str())),
        fmt::arg("subject", quote(subject)),
        fmt::arg("email_from", quote(email_from)),
        fmt::arg("email_to", quote(email_to)));

    FILE * command_pipe = popen(command_string.c_str(), "w");
    if (command_pipe) {
        std::string stdin_format = config_automatic.config_command_email.stdin_format.get_value();
        fputs(libdnf5::utils::sformat(stdin_format, fmt::arg("body", output_stream.str())).c_str(), command_pipe);
        std::fflush(command_pipe);
        pclose(command_pipe);
    }
}

void EmitterEmail::notify() {
    EmailMessage message;
    std::string subject = libdnf5::utils::sformat(
        _("[{}] dnf5-automatic: {}"), config_automatic.config_emitters.system_name.get_value(), short_message());

    std::vector<std::string> to = config_automatic.config_email.email_to.get_value();
    std::string from = config_automatic.config_email.email_from.get_value();
    message.set_to(to);
    message.set_from(from);
    message.set_subject(subject);
    message.set_body(output_stream.str());

    {
        // use curl to send the message
        std::string payload = message.str();
        std::string tls = config_automatic.config_email.email_tls.get_value().c_str();

        CURL * curl;
        CURLcode res = CURLE_OK;
        struct curl_slist * recipients = NULL;

        curl = curl_easy_init();
        if (curl) {
            std::string username = config_automatic.config_email.email_username.get_value();
            std::string password = config_automatic.config_email.email_password.get_value();
            if (!username.empty()) {
                curl_easy_setopt(curl, CURLOPT_USERNAME, username.c_str());
                curl_easy_setopt(curl, CURLOPT_PASSWORD, password.c_str());
            }

            const char * protocol = "smtp";
            if (tls == "starttls") {
                curl_easy_setopt(curl, CURLOPT_USE_SSL, (long)CURLUSESSL_ALL);
            } else if (tls == "yes") {
                protocol = "smtps";
            }

            // TODO(mblaha): check smtp protocol availability?
            curl_version_info_data * ver;
            ver = curl_version_info(CURLVERSION_NOW);
            bool protocol_supported = false;
            for (auto ptr = ver->protocols; *ptr; ++ptr) {
                if (strcmp(*ptr, protocol) == 0) {
                    protocol_supported = true;
                    break;
                }
            }
            if (protocol_supported) {
                std::string email_host = libdnf5::utils::sformat(
                    "{}://{}:{}/",
                    protocol,
                    config_automatic.config_email.email_host.get_value(),
                    config_automatic.config_email.email_port.get_value());
                curl_easy_setopt(curl, CURLOPT_URL, email_host.c_str());

                curl_easy_setopt(curl, CURLOPT_MAIL_FROM, from.c_str());

                for (const auto & eml : to) {
                    recipients = curl_slist_append(recipients, eml.c_str());
                }
                curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

                FILE * payload_file = fmemopen(payload.data(), payload.size(), "r");
                curl_easy_setopt(curl, CURLOPT_READDATA, payload_file);
                fclose(payload_file);

                curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

                res = curl_easy_perform(curl);
                if (res != CURLE_OK) {
                    std::cerr << "libcurl error while sending e-mail: " << curl_easy_strerror(res) << std::endl;
                }
            } else {
                std::cerr << "Error: installed version of libcurl does not support " << protocol
                          << " protocol. Cannot use \"email\" emitter to send the results. On Fedora please check that "
                             "libcurl package is installed."
                          << std::endl;
            }

            curl_slist_free_all(recipients);
            curl_easy_cleanup(curl);
        }
    }
}

}  // namespace dnf5
