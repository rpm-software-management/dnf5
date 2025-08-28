// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later


#ifndef LIBDNF5_CLI_OUTPUT_TRANSACTION_TABLE_HPP
#define LIBDNF5_CLI_OUTPUT_TRANSACTION_TABLE_HPP

#include "interfaces/transaction.hpp"

#include "libdnf5-cli/defs.h"

#include <libdnf5/common/impl_ptr.hpp>

#include <cstdio>
#include <iostream>

namespace libdnf5::cli::output {

class LIBDNF_CLI_API TransactionTable {
public:
    explicit TransactionTable(ITransaction & transaction);
    ~TransactionTable();

    TransactionTable(const TransactionTable &) = delete;
    TransactionTable & operator=(const TransactionTable &) = delete;

    TransactionTable(TransactionTable && src);
    TransactionTable & operator=(TransactionTable && src);

    void print_table();
    void print_summary() const;

    void set_colors_enabled(bool enable);
    void set_term_width(std::size_t width);
    void set_output_stream(std::FILE * fd);

private:
    class LIBDNF_CLI_LOCAL Impl;
    ImplPtr<Impl> p_impl;
};

LIBDNF_CLI_API void print_resolve_logs(const ITransaction & transaction, std::ostream & stream = std::cerr);

LIBDNF_CLI_API bool print_transaction_table(ITransaction & transaction);

}  // namespace libdnf5::cli::output

#endif  // LIBDNF5_CLI_OUTPUT_TRANSACTION_TABLE_HPP
