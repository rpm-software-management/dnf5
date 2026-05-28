// Copyright Contributors to the DNF5 project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "interaction_callbacks.hpp"

#include <fmt/format.h>
#include <libdnf5-cli/utils/userconfirm.hpp>

namespace dnf5 {

void InteractionCallbacks::message(MessageLevel level, const libdnf5::Message & message) {
    auto msg = message.format(true);
    switch (level) {
        case MessageLevel::INFO:
            context.print_output(msg);
            break;
        case MessageLevel::NOTICE:
            context.print_info(msg);
            break;
        case MessageLevel::WARNING:
        case MessageLevel::ERROR:
            context.print_error(msg);
    }
}


int32_t InteractionCallbacks::confirm(const libdnf5::Message & message, bool default_answer) {
    auto msg = message.format(true);
    std::cerr << msg << std::endl;
    bool result = libdnf5::cli::utils::userconfirm::userconfirm(context.get_base().get_config(), default_answer);
    return result ? libdnf5::base::ANSWER_YES : libdnf5::base::ANSWER_NO;
}


int32_t InteractionCallbacks::choice(
    const libdnf5::Message & message, const std::vector<libdnf5::Message *> & choices, int32_t default_option) {
    std::cerr << message.format(true) << std::endl;

    unsigned int idx = 0;
    for (const auto & option : choices) {
        std::cerr << fmt::format("{}: {}", ++idx, option->format(true)) << std::endl;
    }

    while (true) {
        std::string option;
        std::cerr << fmt::format("Option index [default: {}]: ", default_option + 1);
        std::getline(std::cin, option);

        if (option.empty()) {
            return default_option;
        }

        try {
            auto option_idx = std::stoul(option);
            if (option_idx == 0 || option_idx > choices.size()) {
                std::cerr << fmt::format("Option index {} is outside available choices", option) << std::endl;
                continue;
            }
            return static_cast<int32_t>(option_idx) - 1;
        } catch (const std::exception & ex) {
            std::cerr << fmt::format("Invalid option index {}: {}", option, ex.what()) << std::endl;
        }
    }
}


int32_t InteractionCallbacks::input_text(
    std::string & out_text,
    const libdnf5::Message & message,
    const char * default_text,
    libdnf5::base::TextValidator * validator) {
    std::cerr << message.format(true) << std::endl;

    std::string input;
    do {
        if (default_text != nullptr) {
            std::cerr << fmt::format("Input [\"{}\"]: ", default_text);
        } else {
            std::cerr << "Input: ";
        }
        if (!std::getline(std::cin, input)) {
            return libdnf5::base::ANSWER_ABORT;
        }

        if (input.empty()) {
            return libdnf5::base::ANSWER_DEFAULT;
        }

        if (!validator) {
            break;
        }

        auto err_msg = validator->validate(input);
        if (!err_msg) {
            break;
        }

        std::cerr << err_msg->format(true) << std::endl;
    } while (true);

    out_text = input;
    return libdnf5::base::ANSWER_YES;
}

int InteractionCallbacks::progress(
    int handle,
    ProgressState state,
    const libdnf5::Message * msg,
    [[maybe_unused]] int64_t count,
    [[maybe_unused]] int64_t total) {
    if (state == ProgressState::NEW) {
        int new_handle = next_handle++;
        if (msg) {
            auto label = msg->format(true);
            progress_labels[new_handle] = label;
            context.print_info(label);
        }
        return new_handle;
    }
    if (state == ProgressState::UPDATE) {
        return handle;
    }
    auto it = progress_labels.find(handle);
    if (msg) {
        std::string text = (it != progress_labels.end() ? it->second + ": " : "") + msg->format(true);
        switch (state) {
            case ProgressState::WARNING:
            case ProgressState::ERROR:
                context.print_error(text);
                break;
            default:
                context.print_info(text);
        }
    }
    if (state == ProgressState::END_OK || state == ProgressState::END_ERROR) {
        if (it != progress_labels.end()) {
            progress_labels.erase(it);
        }
    }
    return handle;
}

}  // namespace dnf5
