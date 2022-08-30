/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "transaction_id.hpp"

#include "utils/bgettext/bgettext-mark-domain.h"
#include "utils/string.hpp"


namespace dnf5 {


InvalidIdRangeError::InvalidIdRangeError(const std::string & id_range)
    : libdnf::Error(
          M_("Invalid transaction ID range \"{}\", \"ID\" or \"ID..ID\" expected, where ID is \"NUMBER\", \"last\" or "
             "\"last-NUMBER\"."),
          id_range) {}


int64_t parse_id(const std::string & id) {
    if (id == "last") {
        return -1;
    }

    if (id.rfind("last-", 0) == 0) {
        // shift by -1. we return -1 for "last", -2 for "last-1" etc.
        return std::stol(id.substr(4), nullptr, 10) - 1;
    }

    // prevent ambiguity
    if (id[0] == '-' || id[0] == '+') {
        throw std::runtime_error("Invalid ID");  // gets replaced with a proper exception up the stack
    }

    return stol(id);
}


std::pair<int64_t, int64_t> parse_transaction_id_range(const std::string & id_range) {
    auto splitted = libdnf::utils::string::split(id_range, "..");

    if (splitted.size() > 2 || splitted.size() < 1) {
        throw InvalidIdRangeError(id_range);
    }

    try {
        std::pair<int64_t, int64_t> res;

        res.first = parse_id(splitted[0]);
        if (splitted.size() == 2) {
            res.second = parse_id(splitted[1]);
        }

        return res;
    } catch (const std::exception & e) {
        throw InvalidIdRangeError(id_range);
    }
}


std::vector<libdnf::transaction::Transaction> list_transactions_from_specs(
    libdnf::transaction::TransactionHistory & ts_history, const std::vector<std::string> & specs) {
    std::vector<int64_t> trans_id_cache;
    std::vector<int64_t> single_ids_to_get;
    std::vector<libdnf::transaction::Transaction> result;

    for (auto & i : specs) {
        auto id_range = parse_transaction_id_range(i);

        if (id_range.first < 0 || id_range.second < 0) {
            if (trans_id_cache.empty()) {
                trans_id_cache = ts_history.list_transaction_ids();
            }

            if (id_range.first < 0) {
                if (static_cast<uint64_t>(std::abs(id_range.first)) <= trans_id_cache.size()) {
                    id_range.first = trans_id_cache.end()[id_range.first];
                } else {
                    // X in last-X goes out of range; we start with ID 0 if
                    // spec is a range, or search for ID 0 (which shouldn't
                    // exist, so we don't find anything)
                    id_range.first = 0;
                }
            }

            if (id_range.second < 0) {
                if (static_cast<uint64_t>(std::abs(id_range.second)) <= trans_id_cache.size()) {
                    id_range.second = trans_id_cache.end()[id_range.second];
                } else {
                    // X in last-X goes out of range for a range upper
                    // boundary, we shouldn't match anything
                    id_range.first = 0;
                    id_range.second = 0;
                }
            }
        }

        // The ID range is just a single ID; accumulate into a vector and fetch them all at once
        if (id_range.second == 0) {
            single_ids_to_get.push_back(id_range.first);
            continue;
        }

        auto transactions = ts_history.list_transactions(id_range.first, id_range.second);
        result.insert(result.end(), transactions.begin(), transactions.end());
    }

    if (!single_ids_to_get.empty()) {
        auto transactions = ts_history.list_transactions(single_ids_to_get);
        result.insert(result.end(), transactions.begin(), transactions.end());
    }

    std::sort(result.begin(), result.end());

    return result;
}

}  // namespace dnf5
