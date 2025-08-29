/*
Copyright Contributors to the DNF5 project.

This file is part of DNF5: https://github.com/rpm-software-management/dnf5/

DNF5 is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2.1 of the License, or
(at your option) any later version.

DNF5 is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with DNF5.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef LIBDNF5_BASE_TRANSACTION_INFO_HPP
#define LIBDNF5_BASE_TRANSACTION_INFO_HPP

#include "libdnf5/defs.h"

#include <memory>
#include <string>

namespace libdnf5::base {

class LIBDNF_API TransactionInfo {
public:
    TransactionInfo();
    TransactionInfo(const TransactionInfo &);
    TransactionInfo(TransactionInfo &&) noexcept;
    TransactionInfo & operator=(const TransactionInfo &);
    TransactionInfo & operator=(TransactionInfo &&) noexcept;
    ~TransactionInfo();

    // Getters
    const std::string & get_description() const noexcept;
    const std::string & get_comment() const noexcept;
    uint32_t get_user_id() const noexcept;
    time_t get_start_time() const noexcept;

    // Setters
    void set_description(const std::string & description);
    void set_comment(const std::string & comment);
    void set_user_id(uint32_t user_id);
    void set_start_time(time_t start_time);

private:
    class LIBDNF_LOCAL Impl;
    std::unique_ptr<Impl> p_impl;
};

}  // namespace libdnf5::base

#endif  // LIBDNF5_BASE_TRANSACTION_INFO_HPP
