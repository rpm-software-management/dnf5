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


#include "libdnf5-cli/output/smartcols_table_wrapper.hpp"

#include <libsmartcols/libsmartcols.h>


namespace libdnf5::cli::output {

SmartcolsTableWrapper::SmartcolsTableWrapper() {
    tb = scols_new_table();
    sb = scols_new_symbols();
    scols_symbols_set_branch(sb, " ");
    scols_symbols_set_right(sb, " ");
    scols_symbols_set_vertical(sb, " ");
    scols_table_set_symbols(tb, sb);
}

SmartcolsTableWrapper::~SmartcolsTableWrapper() {
    scols_unref_symbols(sb);
    scols_unref_table(tb);
}

}  // namespace libdnf5::cli::output
