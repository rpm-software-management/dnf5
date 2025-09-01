// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 2.1 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.

#include "libdnf5/rpm/package.hpp"


namespace libdnf5::rpm {

class Changelog::Impl {
public:
    Impl(time_t timestamp, std::string author, std::string text)
        : timestamp(timestamp),
          author(std::move(author)),
          text(std::move(text)) {}

private:
    friend Changelog;

    time_t timestamp;
    std::string author;
    std::string text;
};

Changelog::Changelog(time_t timestamp, const std::string & author, const std::string & text)
    : p_impl(new Impl(timestamp, author, text)) {}

Changelog::~Changelog() = default;

Changelog::Changelog(const Changelog & src) = default;
Changelog::Changelog(Changelog && src) noexcept = default;

Changelog & Changelog::operator=(const Changelog & src) = default;
Changelog & Changelog::operator=(Changelog && src) noexcept = default;

const time_t & Changelog::get_timestamp() const {
    return p_impl->timestamp;
}

const std::string & Changelog::get_author() const {
    return p_impl->author;
}

const std::string & Changelog::get_text() const {
    return p_impl->text;
}

}  // namespace libdnf5::rpm
