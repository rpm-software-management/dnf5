// Copyright Contributors to the DNF5 project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "shared.hpp"

namespace dnf5 {

std::string get_url_part(const std::string & url, CURLUPart what_part) {
    std::string ret;
    CURLUcode rc;
    CURLU * c_url = curl_url();
    rc = curl_url_set(c_url, CURLUPART_URL, url.c_str(), 0);
    if (!rc) {
        char * part;
        rc = curl_url_get(c_url, what_part, &part, 0);
        if (!rc) {
            ret = part;
            curl_free(part);
        }
    }
    curl_url_cleanup(c_url);
    return ret;
}

}  // namespace dnf5
