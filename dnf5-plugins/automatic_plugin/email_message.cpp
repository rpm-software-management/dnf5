// Copyright Contributors to the DNF5 project.
// Copyright (C) 2022 Red Hat, Inc.
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "email_message.hpp"

#include <fmt/chrono.h>
#include <libdnf5/utils/format.hpp>

#include <chrono>
#include <string>

namespace dnf5 {

// TODO(mblaha): use some library to create an email instead of this template
constexpr const char * EMAIL_HEADER_TEMPLATE =
    "Date: {date}\r\n"
    "To: {to}\r\n"
    "From: {from}\r\n"
    "Subject: {subject}\r\n"
    "X-Mailer: dnf5-automatic\r\n"
    "\r\n";

void EmailMessage::set_body(std::stringstream & body) {
    this->body.clear();
    for (std::string line; std::getline(body, line);) {
        this->body.push_back(line);
    }
}

std::string EmailMessage::str() {
    // RFC 5322 requires dates with integral seconds.
    const auto now = std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::system_clock::now());
    std::string date = fmt::format("{:%a, %d %b %Y %H:%M:%S %z}", now);

    std::string to_str;
    for (const auto & eml : to) {
        if (!to_str.empty()) {
            to_str += ", ";
        }
        to_str += eml;
    }

    std::string msg;
    msg = libdnf5::utils::sformat(
        EMAIL_HEADER_TEMPLATE,
        fmt::arg("date", date),
        fmt::arg("to", to_str),
        fmt::arg("from", from),
        fmt::arg("subject", subject));
    for (const auto & line : body) {
        msg.append(line).append("\r\n");
    }

    return msg;
}

}  // namespace dnf5
