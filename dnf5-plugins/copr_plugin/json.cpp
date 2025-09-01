// Copyright Contributors to the DNF5 project.
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

#include "json.hpp"

#include "download_file.hpp"

#include "libdnf5/utils/fs/temp.hpp"

#include <fstream>
#include <iostream>

Json::Json(libdnf5::Base & base, const std::string & url) {
    auto temp_file = libdnf5::utils::fs::TempFile("/tmp", "dnf5-copr-plugin");
    download_file(base, url, temp_file.get_path());
    std::ifstream file(temp_file.get_path());
    std::stringstream buffer;
    buffer << file.rdbuf();
    root = json_tokener_parse(buffer.str().c_str());
    this->cleanup = true;
}

Json::Json(struct json_object * root) {
    this->root = root;
    this->cleanup = false;
}

std::unique_ptr<Json> Json::get_array_item(size_t i) {
    struct json_object * object = json_object_array_get_idx(root, i);
    return std::make_unique<Json>(object);
}

std::unique_ptr<Json> Json::get_dict_item(const std::string & key) {
    struct json_object * object;
    json_object_object_get_ex(root, key.c_str(), &object);
    return std::make_unique<Json>(object);
}

bool Json::has_key(const std::string & key) {
    return json_object_object_get_ex(root, key.c_str(), nullptr);
}

std::vector<std::string> Json::keys() {
    std::vector<std::string> retval;
    json_object_object_foreach(root, key, val) {
        retval.push_back(key);
    }
    return retval;
}

std::string Json::string() {
    return json_object_get_string(root);
}

bool Json::boolean() {
    std::string str = json_object_get_string(root);
    return str == "1" || str == "True" || str == "true";
}

size_t Json::array_length() {
    return json_object_array_length(root);
}

Json::~Json() {
    if (cleanup)
        json_object_put(root);
}
