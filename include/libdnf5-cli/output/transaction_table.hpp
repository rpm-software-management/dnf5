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
