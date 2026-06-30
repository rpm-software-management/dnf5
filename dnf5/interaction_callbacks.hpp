// Copyright Contributors to the DNF5 project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef DNF5_INTERACTION_CALLBACKS_HPP
#define DNF5_INTERACTION_CALLBACKS_HPP

#include "dnf5/context.hpp"

#include <libdnf5-cli/progressbar/multi_progress_bar.hpp>
#include <libdnf5/base/interaction_callbacks.hpp>
#include <libdnf5/repo/download_callbacks.hpp>

#include <chrono>
#include <map>
#include <string>

namespace dnf5 {

class InteractionCallbacks final : public libdnf5::base::InteractionCallbacks {
public:
    InteractionCallbacks(Context & context) : context(context) {}

private:
    void message(MessageLevel level, const libdnf5::Message & message) override;

    int32_t confirm(const libdnf5::Message & message, bool default_answer) override;

    int32_t choice(
        const libdnf5::Message & message,
        const std::vector<libdnf5::Message *> & choices,
        int32_t default_option) override;

    int32_t input_text(
        std::string & out_text,
        const libdnf5::Message & message,
        const char * default_text,
        libdnf5::base::TextValidator * validator) override;

    int progress(int handle, ProgressState state, const libdnf5::Message * msg, int64_t count, int64_t total) override;

    Context & context;
    int next_handle = 0;
    std::map<int, std::string> progress_labels;
};

}  // namespace dnf5

#endif
