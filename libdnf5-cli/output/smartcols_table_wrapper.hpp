// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later


#ifndef LIBDNF5_CLI_OUTPUT_SMARTCOLS_TABLE_WRAPPER_HPP
#define LIBDNF5_CLI_OUTPUT_SMARTCOLS_TABLE_WRAPPER_HPP

#include <libsmartcols/libsmartcols.h>


namespace libdnf5::cli::output {

/// C++ wrapper around libscols_table * structure to correctly handle memory
/// allocation/deallocation.
class SmartcolsTableWrapper {
public:
    SmartcolsTableWrapper();
    SmartcolsTableWrapper(SmartcolsTableWrapper && other) noexcept : tb(other.tb), sb(other.sb) {
        other.sb = nullptr;  // Transfer ownership
        other.tb = nullptr;
    }
    ~SmartcolsTableWrapper();

    // Disallow copy operations
    SmartcolsTableWrapper(const SmartcolsTableWrapper &) = delete;
    SmartcolsTableWrapper & operator=(const SmartcolsTableWrapper &) = delete;

    libscols_table * operator*() { return tb; }

    const libscols_table * operator*() const { return tb; }

private:
    libscols_table * tb;
    libscols_symbols * sb;
};

}  // namespace libdnf5::cli::output

#endif  // LIBDNF5_CLI_OUTPUT_SMARTCOLS_TABLE_WRAPPER_HPP
