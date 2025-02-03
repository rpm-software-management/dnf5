/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2.1 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

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
