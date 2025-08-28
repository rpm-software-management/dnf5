// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

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
