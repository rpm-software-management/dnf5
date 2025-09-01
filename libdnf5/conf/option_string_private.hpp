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

#include "libdnf5/conf/option_string.hpp"

namespace libdnf5 {

class OptionString::Impl {
public:
    Impl(std::string && default_value) : icase(false), default_value(default_value), value(std::move(default_value)) {};
    Impl() : icase(false) {};
    Impl(std::string && regex, bool icase) : regex(std::move(regex)), icase(icase) {};
    Impl(std::string && default_value, std::string && regex, bool icase)
        : regex(std::move(regex)),
          icase(icase),
          default_value(default_value),
          value(std::move(default_value)) {};

    std::string regex;
    bool icase;
    std::string default_value;
    std::string value;
};


}  // namespace libdnf5
