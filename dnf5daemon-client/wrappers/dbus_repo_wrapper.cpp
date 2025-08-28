// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: GPL-2.0-or-later

#include "dbus_repo_wrapper.hpp"

namespace dnfdaemon::client {

std::vector<std::pair<std::string, std::string>> DbusRepoWrapper::get_distro_tags() const {
    // sdbus::Variant cannot handle vector<pair<string,string>> so values are
    // serialized to vector<string>.
    // convert [tag1, val1, tag2, val2,...] back to [(tag1, val1), (tag2, val2),...]
    std::vector<std::pair<std::string, std::string>> dt{};
    auto tags_raw = std::vector<std::string>(rawdata.at("distro_tags"));
    if (!tags_raw.empty()) {
        for (size_t i = 0; i < (tags_raw.size() - 1); i += 2) {
            dt.emplace_back(tags_raw[i], tags_raw[i + 1]);
        }
    }
    return dt;
}

}  // namespace dnfdaemon::client
