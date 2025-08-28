// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "libdnf5/common/message.hpp"

namespace libdnf5 {

// The `Impl` class is now empty. It allows to add data members in the future.
class Message::Impl {};

// Constructors optimization:
// `p_impl` is initialized by the default constructor. -> It is empty (set to `nullptr`).
// The real object will need to be created when the `Impl` class will contain data members.
// For now, only a placeholder is enought.
Message::Message() = default;
Message::Message(const Message & src) = default;
Message::Message(Message && src) noexcept = default;

Message::~Message() = default;

Message & Message::operator=(const Message & src) = default;
Message & Message::operator=(Message && src) noexcept = default;


EmptyMessage::EmptyMessage() = default;
EmptyMessage::EmptyMessage(const EmptyMessage & src) = default;
EmptyMessage::EmptyMessage(EmptyMessage && src) noexcept = default;

EmptyMessage::~EmptyMessage() = default;

EmptyMessage & EmptyMessage::operator=(const EmptyMessage & src) = default;
EmptyMessage & EmptyMessage::operator=(EmptyMessage && src) noexcept = default;

std::string EmptyMessage::format(
    [[maybe_unused]] bool translate, [[maybe_unused]] const libdnf5::utils::Locale * locale) const {
    return "";
}

}  // namespace libdnf5
