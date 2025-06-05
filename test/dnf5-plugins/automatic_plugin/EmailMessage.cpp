/*
Copyright Contributors to the DNF5 project.

This file is part of DNF5: https://github.com/rpm-software-management/dnf5/

DNF5 is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

DNF5 is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with DNF5.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "EmailMessage.hpp"

#include "email_message.hpp"

#include <chrono>
#include <sstream>
#include <vector>

using namespace std;

// Redefine std::chrono::system_clock::now() used by dnf5::EmailMessage::str()
// to always return fixed time 1970-01-01T00:00:00.000000.
std::chrono::time_point<std::chrono::system_clock> std::chrono::system_clock::now() noexcept {
    return std::chrono::time_point<std::chrono::system_clock>();
}

void EmailMessageTest::test_email_emitter() {
    std::stringstream body{"text"};
    dnf5::EmailMessage message;
    message.set_subject("subject");
    message.set_from("from@test");
    message.set_to(std::vector<std::string>{"to1@test", "to2@test"});
    message.set_body(body);
    std::string got = message.str();

    std::string expected{
        "Date: Thu, 01 Jan 1970 00:00:00 +0000\r\n"
        "To: to1@test, to2@test\r\n"
        "From: from@test\r\n"
        "Subject: subject\r\n"
        "X-Mailer: dnf5-automatic\r\n"
        "\r\n"
        "text\r\n"};

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unexpectedly formatted e-mail message from dnf5::EmailMessage::str()", expected, got);
}

CPPUNIT_TEST_SUITE_REGISTRATION(EmailMessageTest);
