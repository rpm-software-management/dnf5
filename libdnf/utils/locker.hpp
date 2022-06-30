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

#ifndef LIBDNF_UTILS_LOCKER_HPP
#define LIBDNF_UTILS_LOCKER_HPP

#include <string>

namespace libdnf::utils {

class Locker {
public:
    explicit Locker(const std::string & path) : path(path){};
    ~Locker();
    bool read_lock();
    bool write_lock();
    void unlock();

private:
    bool lock(short int type);

    std::string path;
    int lock_fd{-1};
};

}  // namespace libdnf::utils

#endif  // LIBDNF_UTILS_LOCKER_HPP
