/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "libdnf5/utils/proc.hpp"

#include <cstdlib>
#include <string>
#include <vector>

#include <unistd.h>
#include <sys/wait.h>

namespace libdnf5::utils::proc
{

int
call(const std::string & command, const std::vector<std::string> & args)
{
    std::vector<char *> c_args;
    c_args.emplace_back(const_cast<char *>(command.c_str()));
    for (const auto & arg : args) {
        c_args.emplace_back(const_cast<char *>(arg.c_str()));
    }
    c_args.emplace_back(nullptr);

    const auto pid = fork();
    if (pid == -1) {
        return -1;
    }
    if (pid == 0) {
        int rc = execvp(command.c_str(), c_args.data());
        _exit(rc == 0 ? 0 : -1);
    } else {
        int status;
        int rc = waitpid(pid, &status, 0);
        if (rc == -1) {
            return -1;
        }
        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        }
        if (WIFSIGNALED(status)) {
            return 128 + WTERMSIG(status);
        }
        return -1;
    }
}

}  // namespace libdnf5::utils::proc
