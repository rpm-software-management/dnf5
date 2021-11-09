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

#ifndef LIBDNF_REPO_SOLV_REPO_HPP
#define LIBDNF_REPO_SOLV_REPO_HPP

#include <solv/repo.h>


const constexpr int CHKSUM_BYTES = 32;

namespace libdnf::repo {

class SolvRepo {
public:
    // Returns "true" when all solvables in the repository are stored contiguously -> No interleaving
    // with solvables from other repositories.
    // Complexity: Linear to the current number of solvables in  repository
    bool is_one_piece() const;

    // Internalize repository if needed.
    void internalize();

    ::Repo * repo{nullptr};

    // Checksum of data in .solv file. Used for validity check of .solvx files.
    unsigned char checksum[CHKSUM_BYTES];

    // the following three elements are needed for repo cache (.solv and .solvx updateinfo) writting
    int main_nsolvables{0};
    int main_nrepodata{0};
    int main_end{0};

    void set_needs_internalizing() { needs_internalizing = true; };

private:
    bool needs_internalizing{false};
};

}  // namespace libdnf::repo

#endif  // LIBDNF_REPO_SOLV_REPO_HPP
