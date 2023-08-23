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


#ifndef DNF5_PLUGINS_AUTOMATIC_PLUGIN_EMAIL_MESSAGE_HPP
#define DNF5_PLUGINS_AUTOMATIC_PLUGIN_EMAIL_MESSAGE_HPP

#include <string>
#include <vector>

namespace dnf5 {

/// Class for creating simple email messages
class EmailMessage {
public:
    EmailMessage() {}

    /// Set the Subject header value
    void set_subject(std::string_view subject) { this->subject = subject; };
    /// Set the From header value
    void set_from(std::string_view from) { this->from = from; };
    /// Set the To header value
    void set_to(const std::vector<std::string> & to) { this->to = to; };
    /// Set the message body
    void set_body(std::string_view body) { this->body = body; };

    /// Return string representation of the message
    std::string str();

private:
    std::string subject;
    std::string from;
    std::vector<std::string> to;
    std::string body;
};

}  // namespace dnf5


#endif  // DNF5_PLUGINS_AUTOMATIC_PLUGIN_EMAIL_MESSAGE_HPP
