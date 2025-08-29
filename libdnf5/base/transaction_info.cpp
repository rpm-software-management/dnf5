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

#include "libdnf5/base/transaction_info.hpp"

namespace libdnf5::base {

class TransactionInfo::Impl {
public:
    std::string description;
    std::string comment;
    uint32_t user_id;
    time_t start_time;
};

TransactionInfo::TransactionInfo() : p_impl(new Impl()) {}

TransactionInfo::TransactionInfo(const TransactionInfo & src) : p_impl(new Impl(*src.p_impl)) {}
TransactionInfo & TransactionInfo::operator=(const TransactionInfo & src) {
    if (this != &src) {
        if (p_impl) {
            *p_impl = *src.p_impl;
        } else {
            p_impl = std::make_unique<Impl>(*src.p_impl);
        }
    }

    return *this;
}

TransactionInfo::TransactionInfo(TransactionInfo && src) noexcept = default;
TransactionInfo & TransactionInfo::operator=(TransactionInfo && src) noexcept = default;

TransactionInfo::~TransactionInfo() = default;

const std::string & TransactionInfo::get_description() const noexcept {
    return p_impl->description;
}

const std::string & TransactionInfo::get_comment() const noexcept {
    return p_impl->comment;
}

uint32_t TransactionInfo::get_user_id() const noexcept {
    return p_impl->user_id;
}

time_t TransactionInfo::get_start_time() const noexcept {
    return p_impl->start_time;
}

void TransactionInfo::set_description(const std::string & description) {
    p_impl->description = description;
}

void TransactionInfo::set_comment(const std::string & comment) {
    p_impl->comment = comment;
}

void TransactionInfo::set_user_id(uint32_t user_id) {
    p_impl->user_id = user_id;
}

void TransactionInfo::set_start_time(time_t start_time) {
    p_impl->start_time = start_time;
}

}  // namespace libdnf5::base
