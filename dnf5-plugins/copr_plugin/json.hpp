// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

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
