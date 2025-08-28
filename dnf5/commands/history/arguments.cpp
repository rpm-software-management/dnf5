// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later


#include "arguments.hpp"


namespace dnf5 {


std::function<std::vector<std::string>(const char * arg)> create_history_id_autocomplete(Context & ctx) {
    return [&ctx](const char * arg) {
        const std::string_view to_complete{arg};
        libdnf5::transaction::TransactionHistory history(ctx.get_base());
        std::vector<int64_t> ids = history.list_transaction_ids();
        std::vector<std::string> all_string_ids;
        std::transform(
            ids.begin(), ids.end(), std::back_inserter(all_string_ids), [](int num) { return std::to_string(num); });
        std::string last;
        std::vector<std::string> possible_ids;
        for (const auto & id : all_string_ids) {
            if (id.compare(0, to_complete.length(), to_complete) == 0) {
                possible_ids.emplace_back(id);
                last = id;
            }
        }
        if (possible_ids.size() == 1) {
            possible_ids[0] = last + " ";
        }
        return possible_ids;
    };
}


}  // namespace dnf5
