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

#ifndef DNF5_COMMANDS_COPR_JSON_HPP
#define DNF5_COMMANDS_COPR_JSON_HPP

#include <json.h>
#include <libdnf5/base/base.hpp>


class Json {
private:
    bool cleanup;
    struct json_object * root;

public:
    Json(libdnf5::Base & base, const std::string & url);
    explicit Json(struct json_object * root);
    std::unique_ptr<Json> get_array_item(size_t i);
    std::unique_ptr<Json> get_dict_item(const std::string & key);
    bool has_key(const std::string & key);
    std::vector<std::string> keys();
    std::string string();
    bool boolean();
    size_t array_length();
    ~Json();
};

#endif  // DNF5_COMMANDS_COPR_JSON_HPP
