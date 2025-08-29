// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later


#ifndef DNF5_PLUGINS_AUTOMATIC_PLUGIN_EMAIL_MESSAGE_HPP
#define DNF5_PLUGINS_AUTOMATIC_PLUGIN_EMAIL_MESSAGE_HPP

#include <sstream>
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
    void set_body(std::stringstream & body);

    /// Return string representation of the message
    std::string str();

private:
    std::string subject;
    std::string from;
    std::vector<std::string> to;
    std::vector<std::string> body;
};

}  // namespace dnf5


#endif  // DNF5_PLUGINS_AUTOMATIC_PLUGIN_EMAIL_MESSAGE_HPP
