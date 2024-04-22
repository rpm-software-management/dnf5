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

#ifndef LIBDNF5_CLI_OUTPUT_PROVIDES_HPP
#define LIBDNF5_CLI_OUTPUT_PROVIDES_HPP

#include "interfaces/package.hpp"

namespace libdnf5::cli::output {

enum ProvidesMatchedBy : int { NO_MATCH = 0, PROVIDES = 1, FILENAME = 2, BINARY = 3 };

void print_provides_table(IPackage & package, const char * spec, int match);

}  // namespace libdnf5::cli::output

#endif  // LIBDNF5_CLI_OUTPUT_PROVIDES_HPP
