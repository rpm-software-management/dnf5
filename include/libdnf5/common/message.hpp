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

#ifndef LIBDNF5_COMMON_MESSAGE_HPP
#define LIBDNF5_COMMON_MESSAGE_HPP

#include "impl_ptr.hpp"

#include "libdnf5/defs.h"
#include "libdnf5/utils/locale.hpp"

#include <string>

namespace libdnf5 {

/// A base class for passing a message whose formatting, including localization
/// (translation, argument format) is done at the destination.
/// Usage: The user creates a child of this class and implements the `format` method.
class LIBDNF_API Message {
public:
    Message();
    Message(const Message & src);
    Message(Message && src) noexcept;
    virtual ~Message();

    Message & operator=(const Message & src);
    Message & operator=(Message && src) noexcept;

    /// Formats the contained message according to the specified arguments and returns the result as a string.
    ///
    /// @param translate  If `true`, it will attempt to translate the message to the requested locale.
    /// @param locale     requested locale for translation and argument formating, nullptr = use global/thread locale
    /// @return A string object holding the formatted result.
    virtual std::string format(bool translate, const libdnf5::utils::Locale * locale = nullptr) const = 0;

private:
    class LIBDNF_LOCAL Impl;
    ImplPtr<Impl> p_impl;
};


/// Class for passing an empty message.
class LIBDNF_API EmptyMessage final : public Message {
public:
    EmptyMessage();
    EmptyMessage(const EmptyMessage & src);
    EmptyMessage(EmptyMessage && src) noexcept;
    ~EmptyMessage() override;

    EmptyMessage & operator=(const EmptyMessage & src);
    EmptyMessage & operator=(EmptyMessage && src) noexcept;

    /// Returns empty string
    ///
    /// @param translate  ignored
    /// @param locale     ignored
    /// @return empty string object
    std::string format(bool translate, const libdnf5::utils::Locale * locale = nullptr) const override;
};

}  // namespace libdnf5

#endif  // LIBDNF5_COMMON_MESSAGE_HPP
