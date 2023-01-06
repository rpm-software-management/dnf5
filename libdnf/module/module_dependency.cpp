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

#include "libdnf/module/module_dependency.hpp"

#include "utils/string.hpp"

#include "libdnf/module/module_item.hpp"
#include "libdnf/module/module_sack.hpp"
#include "libdnf/module/module_sack_weak.hpp"

#include <modulemd-2.0/modulemd-module-stream.h>
#include <modulemd-2.0/modulemd-profile.h>
#include <modulemd-2.0/modulemd.h>

extern "C" {
#include <solv/pool.h>
#include <solv/pool_parserpmrichdep.h>
#include <solv/repo.h>
}

#include <algorithm>
#include <string>
#include <vector>

namespace libdnf::module {


std::string ModuleDependency::to_string() {
    if (streams.empty()) {
        return module_name;
    }
    std::sort(streams.begin(), streams.end());
    return fmt::format("{}:[{}]", module_name, utils::string::join(streams, ","));
}


}  // namespace libdnf::module
