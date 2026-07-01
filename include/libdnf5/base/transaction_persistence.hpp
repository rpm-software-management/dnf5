// Copyright Contributors to the DNF5 project
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef LIBDNF5_BASE_TRANSACTION_PERSISTENCE_HPP
#define LIBDNF5_BASE_TRANSACTION_PERSISTENCE_HPP

namespace libdnf5::base {

// @replaces libdnf:transaction/Types.hpp:enum:TransactionPersistence
enum class TransactionPersistence : int { UNKNOWN = 0, PERSIST = 1, TRANSIENT = 2 };

}  // namespace libdnf5::base

#endif  // LIBDNF5_BASE_TRANSACTION_PERSISTENCE_HPP
