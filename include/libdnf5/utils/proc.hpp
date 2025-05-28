/*
Copyright (C) 2025 Red Hat, Inc.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef LIBDNF5_PROC_HPP
#define LIBDNF5_PROC_HPP

#include "libdnf5/defs.h"

#include <string>
#include <vector>

namespace libdnf5::utils::proc {

LIBDNF_API int call(const std::string & command, const std::vector<std::string> & args);

}  // namespace libdnf5::utils::proc

#endif  // LIBDNF5_PROC_HPP
