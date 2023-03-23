#include "helpers.hpp"

#include <regex>

namespace dnf5 {

std::vector<std::string> repo_fallbacks(const std::string & name_version) {
    std::smatch match;
    if (std::regex_match(name_version, match, std::regex("^(rhel|centos|almalinux)-([0-9]+)$")))
        // try rhel-9-x86_64, then epel-9-x86_64
        return {name_version, "epel-" + match.str(2)};
    if (std::regex_match(name_version, match, std::regex("^(rhel|centos|almalinux)-([0-9]+).([0-9]+)$")))
        // try rhel-9.1-x86_64, then rhel-9-x86_64, then epel-9-x86_64
        return {name_version, match.str(1) + "-" + match.str(2), "epel-" + match.str(2)};
    return {name_version};
}

}  // namespace dnf5
