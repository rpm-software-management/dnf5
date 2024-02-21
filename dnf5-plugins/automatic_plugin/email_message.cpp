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

#include "email_message.hpp"

#include <libdnf5/utils/format.hpp>

#include <chrono>

namespace dnf5 {

// TODO(mblaha): use some library to create an email instead of this template
constexpr const char * MESSAGE_TEMPLATE =
    "Date: {date}\r\n"
    "To: {to}\r\n"
    "From: {from}\r\n"
    "Subject: {subject}\r\n"
    "X-Mailer: dnf5-automatic\r\n"
    "\r\n"
    "{body}";

std::string EmailMessage::str() {
    const auto now = std::chrono::system_clock::now();
    std::string date = std::format("{:%a, %d %b %Y %H:%M:%S %z}", now);

    std::string to_str;
    for (const auto & eml : to) {
        if (!to_str.empty()) {
            to_str += ", ";
        }
        to_str += eml;
    }

    std::string msg;
    msg = libdnf5::utils::sformat(
        MESSAGE_TEMPLATE,
        fmt::arg("date", date),
        fmt::arg("to", to_str),
        fmt::arg("from", from),
        fmt::arg("subject", subject),
        fmt::arg("body", body));
    return msg;
}

}  // namespace dnf5
