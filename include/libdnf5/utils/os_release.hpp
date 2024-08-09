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

#ifndef LIBDNF5_UTILS_OS_RELEASE_HPP
#define LIBDNF5_UTILS_OS_RELEASE_HPP

#include "libdnf5/defs.h"

#include <filesystem>
#include <map>
#include <string>
#include <utility>

namespace libdnf5::utils {

/// Object which loads and exposes values from an os-release file.
class LIBDNF_API OSRelease {
public:
    /// Creates an instance of `OSRelease` from the file at `path`.
    ///
    /// @param path The path to the os-release file to load and parse.
    explicit OSRelease(std::filesystem::path path = "/etc/os-release") : path(std::move(path)) {}

    /// Returns the corresponding os-release value for `key`.
    /// If the value for `key` isn't set, `default_value` is returned.
    ///
    /// @param key The os-release key to get the value of.
    /// @param default_value Default value to return if the value for `key` isn't set.
    std::string get_value(const std::string & key, const std::string & default_value = "UNSET");

private:
    std::filesystem::path path;
    bool initialized_ = false;
    std::map<std::string, std::string> map = {};
    void initialize();
};

}  // namespace libdnf5::utils

#endif  // LIBDNF5_UTILS_OS_RELEASE_HPP
