// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later


#include "smartcols_table_wrapper.hpp"

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
