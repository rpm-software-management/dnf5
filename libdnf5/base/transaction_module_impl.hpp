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

#ifndef LIBDNF5_BASE_TRANSACTION_MODULE_IMPL_HPP
#define LIBDNF5_BASE_TRANSACTION_MODULE_IMPL_HPP

#include "libdnf5/base/transaction_module.hpp"

namespace libdnf5::base {

class TransactionModule::Impl {
public:
    Impl(std::string module_name, std::string module_stream, Action action, Reason reason);

    void replaces_append(std::string && module_name, std::string && module_stream);
    void replaced_by_append(std::string && module_name, std::string && module_stream);

private:
    friend TransactionModule;

    State state{State::STARTED};
    Action action;
    Reason reason;
    std::string module_name;
    std::string module_stream;
    std::vector<std::pair<std::string, std::string>> replaces;
    // Not used for switching module streams, but it can be used in the future for module obsoletes
    std::vector<std::pair<std::string, std::string>> replaced_by;
};

}  // namespace libdnf5::base

#endif  // LIBDNF5_BASE_TRANSACTION_MODULE_IMPL_HPP
