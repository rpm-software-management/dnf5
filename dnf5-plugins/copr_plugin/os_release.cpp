#include "os_release.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>


OSRelease::OSRelease() {}

std::string OSRelease::get_value(const std::string & key, const std::string & default_value) {
    initialize();
    if (map.find(key) == map.end())
        return default_value;
    return map[key];
}


void OSRelease::initialize() {
    if (initialized_)
        return;

    initialized_ = true;
    std::filesystem::path filename;
    if (const char * pathname = std::getenv("TEST_COPR_CONFIG_DIR"))
        filename = pathname;
    else
        filename = "/etc";
    filename /= "os-release";
    std::ifstream infile(filename);
    if (!std::filesystem::exists(filename))
        return;

    const std::regex r_no_quotes("^([A-Z_]+)=(\\w+)");
    const std::regex r_quotes("^([A-Z_]+)=\"([\\w\\s]+)\"");
    std::smatch match;
    std::string line;

    while (std::getline(infile, line)) {
        if (std::regex_match(line, match, r_no_quotes)) {
            map[match[1]] = match[2];
            continue;
        }
        if (std::regex_match(line, match, r_quotes)) {
            map[match[1]] = match[2];
            continue;
        }
    }
}


bool OSRelease::initialized_ = false;
std::map<std::string, std::string> OSRelease::map = {};
