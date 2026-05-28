// Copyright Contributors to the DNF5 project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "libdnf5/base/interaction_callbacks.hpp"

#include "libdnf5/base/base.hpp"

namespace libdnf5::base {

InteractionCallbacks::InteractionCallbacks() = default;


InteractionCallbacks::~InteractionCallbacks() = default;


BaseWeakPtr InteractionCallbacks::get_base() const {
    return base;
}


void InteractionCallbacks::register_base(Base & base) {
    this->base = base.get_weak_ptr();
}


void InteractionCallbacks::message([[maybe_unused]] MessageLevel level, [[maybe_unused]] const libdnf5::Message & msg) {
}


int32_t InteractionCallbacks::confirm(
    [[maybe_unused]] const libdnf5::Message & msg, [[maybe_unused]] bool default_answer) {
    auto & config = base->get_config();
    if (config.get_assumeno_option().get_value()) {
        return ANSWER_NO;
    }
    if (config.get_assumeyes_option().get_value()) {
        return ANSWER_YES;
    }
    return ANSWER_DEFAULT;
}


int32_t InteractionCallbacks::choice(
    [[maybe_unused]] const libdnf5::Message & msg,
    [[maybe_unused]] const std::vector<libdnf5::Message *> & options,
    [[maybe_unused]] int32_t default_option) {
    return ANSWER_DEFAULT;
}


int InteractionCallbacks::progress(
    int handle,
    [[maybe_unused]] ProgressState state,
    [[maybe_unused]] const libdnf5::Message * msg,
    [[maybe_unused]] int64_t count,
    [[maybe_unused]] int64_t total) {
    if (state == ProgressState::NEW) {
        return 0;
    }
    return handle;
}


int32_t InteractionCallbacks::input_text(
    [[maybe_unused]] std::string & out_text,
    [[maybe_unused]] const libdnf5::Message & msg,
    [[maybe_unused]] const char * default_text,
    [[maybe_unused]] libdnf5::base::TextValidator * validator) {
    return ANSWER_DEFAULT;
}

}  // namespace libdnf5::base
