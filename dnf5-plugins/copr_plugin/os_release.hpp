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

#ifndef DNF5_COMMANDS_COPR_OS_RELEASE_HPP
#define DNF5_COMMANDS_COPR_OS_RELEASE_HPP

#include <map>
#include <string>

class OSRelease {
private:
    static bool initialized_;
    static std::map<std::string, std::string> map;
    void initialize();

public:
    OSRelease();
    std::string get_value(const std::string & key, const std::string & default_value = "UNSET");
};

#endif  // DNF5_COMMANDS_COPR_OS_RELEASE_HPP
