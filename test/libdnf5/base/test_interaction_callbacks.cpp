// Copyright Contributors to the DNF5 project.
// SPDX-License-Identifier: GPL-2.0-or-later

#include "test_interaction_callbacks.hpp"

#include <libdnf5/base/base.hpp>
#include <libdnf5/base/interaction_callbacks.hpp>
#include <libdnf5/base/text_validator_callback.hpp>
#include <libdnf5/common/message.hpp>

#include <memory>


CPPUNIT_TEST_SUITE_REGISTRATION(InteractionCallbacksTest);


// Test Message implementation
class TestMessage : public libdnf5::Message {
public:
    explicit TestMessage(std::string text) : text(std::move(text)) {}

    std::string format(
        [[maybe_unused]] bool translate, [[maybe_unused]] const libdnf5::utils::Locale * = nullptr) const override {
        return text;
    }

private:
    std::string text;
};


// Test TextValidatorCallback implementation
class NumberValidator : public libdnf5::base::TextValidatorCallback {
public:
    libdnf5::Message * validate(const std::string & input) override {
        if (input.empty()) {
            error_empty = TestMessage("Input cannot be empty");
            return &error_empty;
        }
        for (char c : input) {
            if (!std::isdigit(c)) {
                error_not_number = TestMessage("Input must be a number");
                return &error_not_number;
            }
        }
        return nullptr;  // Valid
    }

private:
    // Error message objects - must remain valid for the lifetime of the validator
    TestMessage error_empty{"Input cannot be empty"};
    TestMessage error_not_number{"Input must be a number"};
};


// Test InteractionCallbacks implementation
class TestInteractionCallbacks : public libdnf5::base::InteractionCallbacks {
public:
    int message_count = 0;
    int confirm_count = 0;
    int choice_count = 0;
    int input_count = 0;
    libdnf5::base::InteractionCallbacks::MessageLevel last_level =
        libdnf5::base::InteractionCallbacks::MessageLevel::INFO;
    std::string last_message;

    void message(MessageLevel level, const libdnf5::Message & msg) override {
        message_count++;
        last_level = level;
        last_message = msg.format(false);
    }

    int32_t confirm(const libdnf5::Message & msg, bool default_answer) override {
        confirm_count++;
        last_message = msg.format(false);
        return default_answer ? libdnf5::base::ANSWER_YES : libdnf5::base::ANSWER_NO;
    }

    int32_t choice(
        const libdnf5::Message & msg,
        const std::vector<libdnf5::Message *> & choices,
        int32_t default_option) override {
        choice_count++;
        last_message = msg.format(false);
        last_default_option = default_option;
        choice_options_count = static_cast<int>(choices.size());
        return 0;  // Select first option for testing
    }

    int32_t input_text(
        std::string & out_text,
        const libdnf5::Message & msg,
        const char * default_text,
        libdnf5::base::TextValidator * validator) override {
        input_count++;
        last_message = msg.format(false);
        last_default_text = default_text ? default_text : "";

        if (validator) {
            std::string test_input = "42";
            auto error = validator->validate(test_input);
            if (!error) {
                out_text = test_input;
                return libdnf5::base::ANSWER_YES;
            }
        }

        out_text = "test input";
        return libdnf5::base::ANSWER_YES;
    }

    int progress(int handle, ProgressState state, const libdnf5::Message * msg, int64_t count, int64_t total) override {
        progress_count++;
        last_progress_state = state;
        last_count = count;
        last_total = total;
        last_progress_msg = msg ? msg->format(false) : "";
        if (state == ProgressState::NEW) {
            return ++next_handle;
        }
        return handle;
    }

public:
    int choice_options_count = 0;
    int32_t last_default_option = 0;
    std::string last_default_text;
    int progress_count = 0;
    int next_handle = 0;
    ProgressState last_progress_state = ProgressState::INFO;
    int64_t last_count = 0;
    int64_t last_total = 0;
    std::string last_progress_msg;
};


void InteractionCallbacksTest::test_message() {
    auto base = get_preconfigured_base();

    // Create and set custom callbacks
    auto callbacks = std::make_unique<TestInteractionCallbacks>();
    auto * callbacks_ptr = callbacks.get();
    base->set_interaction_callbacks(std::move(callbacks));

    // Test message
    TestMessage msg("Test warning");
    base->message(libdnf5::base::InteractionCallbacks::MessageLevel::WARNING, msg);
    CPPUNIT_ASSERT_EQUAL(1, callbacks_ptr->message_count);
    CPPUNIT_ASSERT_EQUAL(std::string("Test warning"), callbacks_ptr->last_message);
    CPPUNIT_ASSERT(callbacks_ptr->last_level == libdnf5::base::InteractionCallbacks::MessageLevel::WARNING);
}


void InteractionCallbacksTest::test_confirm() {
    auto base = get_preconfigured_base();

    auto callbacks = std::make_unique<TestInteractionCallbacks>();
    auto * callbacks_ptr = callbacks.get();
    base->set_interaction_callbacks(std::move(callbacks));

    // Test confirm with default_answer=true
    TestMessage msg("Confirm?");
    int32_t result = base->confirm(msg, true);
    CPPUNIT_ASSERT_EQUAL(1, callbacks_ptr->confirm_count);
    CPPUNIT_ASSERT_EQUAL(std::string("Confirm?"), callbacks_ptr->last_message);
    CPPUNIT_ASSERT_EQUAL(libdnf5::base::ANSWER_YES, result);

    // Test confirm with default_answer=false
    result = base->confirm(msg, false);
    CPPUNIT_ASSERT_EQUAL(2, callbacks_ptr->confirm_count);
    CPPUNIT_ASSERT_EQUAL(libdnf5::base::ANSWER_NO, result);
}


void InteractionCallbacksTest::test_choice() {
    auto base = get_preconfigured_base();

    auto callbacks = std::make_unique<TestInteractionCallbacks>();
    auto * callbacks_ptr = callbacks.get();
    base->set_interaction_callbacks(std::move(callbacks));

    // Test choice
    TestMessage opt_a("Option A");
    TestMessage opt_b("Option B");
    TestMessage opt_c("Option C");
    std::vector<libdnf5::Message *> choices = {&opt_a, &opt_b, &opt_c};

    TestMessage msg("Select:");
    int32_t choice_idx = base->choice(msg, choices, 0);
    CPPUNIT_ASSERT_EQUAL(1, callbacks_ptr->choice_count);
    CPPUNIT_ASSERT_EQUAL(std::string("Select:"), callbacks_ptr->last_message);
    CPPUNIT_ASSERT_EQUAL(static_cast<int32_t>(0), choice_idx);
    CPPUNIT_ASSERT_EQUAL(3, callbacks_ptr->choice_options_count);
}


void InteractionCallbacksTest::test_input_text() {
    auto base = get_preconfigured_base();

    auto callbacks = std::make_unique<TestInteractionCallbacks>();
    auto * callbacks_ptr = callbacks.get();
    base->set_interaction_callbacks(std::move(callbacks));

    // Test input_text without validator
    TestMessage msg("Enter:");
    std::string text;
    int32_t result = base->input_text(text, msg, nullptr, nullptr);
    CPPUNIT_ASSERT_EQUAL(1, callbacks_ptr->input_count);
    CPPUNIT_ASSERT_EQUAL(std::string("Enter:"), callbacks_ptr->last_message);
    CPPUNIT_ASSERT_EQUAL(libdnf5::base::ANSWER_YES, result);
    CPPUNIT_ASSERT_EQUAL(std::string("test input"), text);
}


void InteractionCallbacksTest::test_input_text_with_validator() {
    auto base = get_preconfigured_base();

    auto callbacks = std::make_unique<TestInteractionCallbacks>();
    auto * callbacks_ptr = callbacks.get();
    base->set_interaction_callbacks(std::move(callbacks));

    // Test input_text with validator
    NumberValidator validator;
    TestMessage msg("Enter number:");
    std::string text;
    int32_t result = base->input_text(text, msg, nullptr, &validator);
    CPPUNIT_ASSERT_EQUAL(1, callbacks_ptr->input_count);
    CPPUNIT_ASSERT_EQUAL(std::string("Enter number:"), callbacks_ptr->last_message);
    CPPUNIT_ASSERT_EQUAL(libdnf5::base::ANSWER_YES, result);
    CPPUNIT_ASSERT_EQUAL(std::string("42"), text);
}


void InteractionCallbacksTest::test_choice_with_default_option() {
    auto base = get_preconfigured_base();

    // Test default implementation: no custom callbacks, returns default_option
    TestMessage opt_a("Option A");
    TestMessage opt_b("Option B");
    TestMessage opt_c("Option C");
    std::vector<libdnf5::Message *> options = {&opt_a, &opt_b, &opt_c};

    // Test default implementation: always returns ANSWER_DEFAULT regardless of options or default_option
    TestMessage msg("Select:");
    CPPUNIT_ASSERT_EQUAL(libdnf5::base::ANSWER_DEFAULT, base->choice(msg, options, 2));
    CPPUNIT_ASSERT_EQUAL(libdnf5::base::ANSWER_DEFAULT, base->choice(msg, options, 0));
    CPPUNIT_ASSERT_EQUAL(libdnf5::base::ANSWER_DEFAULT, base->choice(msg, options, libdnf5::base::ANSWER_DEFAULT));

    std::vector<libdnf5::Message *> empty;
    CPPUNIT_ASSERT_EQUAL(libdnf5::base::ANSWER_DEFAULT, base->choice(msg, empty, 0));
}


void InteractionCallbacksTest::test_input_text_with_default_text() {
    auto base = get_preconfigured_base();

    // Test default implementation: always returns ANSWER_DEFAULT, out_text is never modified
    TestMessage msg("Enter:");
    std::string text;
    text = "unchanged";
    CPPUNIT_ASSERT_EQUAL(libdnf5::base::ANSWER_DEFAULT, base->input_text(text, msg, "prefilled", nullptr));
    CPPUNIT_ASSERT_EQUAL(std::string("unchanged"), text);

    text = "unchanged";
    CPPUNIT_ASSERT_EQUAL(libdnf5::base::ANSWER_DEFAULT, base->input_text(text, msg, "", nullptr));
    CPPUNIT_ASSERT_EQUAL(std::string("unchanged"), text);

    text = "unchanged";
    CPPUNIT_ASSERT_EQUAL(libdnf5::base::ANSWER_DEFAULT, base->input_text(text, msg, nullptr, nullptr));
    CPPUNIT_ASSERT_EQUAL(std::string("unchanged"), text);
}


void InteractionCallbacksTest::test_progress() {
    auto base = get_preconfigured_base();

    auto callbacks = std::make_unique<TestInteractionCallbacks>();
    auto * cb = callbacks.get();
    base->set_interaction_callbacks(std::move(callbacks));

    TestMessage start_msg("Installing foo");
    int handle = base->progress(0, libdnf5::base::InteractionCallbacks::ProgressState::NEW, &start_msg, 0, 10);
    CPPUNIT_ASSERT(handle >= 0);
    CPPUNIT_ASSERT_EQUAL(1, cb->progress_count);
    CPPUNIT_ASSERT_EQUAL(std::string("Installing foo"), cb->last_progress_msg);

    int ret = base->progress(handle, libdnf5::base::InteractionCallbacks::ProgressState::UPDATE, nullptr, 5, 10);
    CPPUNIT_ASSERT(ret >= 0);
    CPPUNIT_ASSERT_EQUAL(2, cb->progress_count);
    CPPUNIT_ASSERT_EQUAL(static_cast<int64_t>(5), cb->last_count);
    CPPUNIT_ASSERT_EQUAL(static_cast<int64_t>(10), cb->last_total);

    base->progress(handle, libdnf5::base::InteractionCallbacks::ProgressState::END_OK, nullptr, 10, 10);
    CPPUNIT_ASSERT_EQUAL(3, cb->progress_count);
    CPPUNIT_ASSERT(cb->last_progress_state == libdnf5::base::InteractionCallbacks::ProgressState::END_OK);

    // Test default implementation: no callbacks set, returns handle unchanged
    auto base2 = get_preconfigured_base();
    int h = base2->progress(0, libdnf5::base::InteractionCallbacks::ProgressState::NEW, nullptr, 0, 0);
    CPPUNIT_ASSERT_EQUAL(0, h);
    int r = base2->progress(42, libdnf5::base::InteractionCallbacks::ProgressState::UPDATE, nullptr, 1, 1);
    CPPUNIT_ASSERT_EQUAL(42, r);
}


void InteractionCallbacksTest::test_abort_return_value() {
    CPPUNIT_ASSERT_EQUAL(int32_t(-4), libdnf5::base::ANSWER_ABORT);

    // Callbacks that return ANSWER_ABORT
    class AbortCallbacks : public libdnf5::base::InteractionCallbacks {
    public:
        int32_t confirm(const libdnf5::Message &, bool) override { return libdnf5::base::ANSWER_ABORT; }
        int32_t choice(const libdnf5::Message &, const std::vector<libdnf5::Message *> &, int32_t) override {
            return libdnf5::base::ANSWER_ABORT;
        }
        int32_t input_text(
            std::string &, const libdnf5::Message &, const char *, libdnf5::base::TextValidator *) override {
            return libdnf5::base::ANSWER_ABORT;
        }
    };

    auto base = get_preconfigured_base();
    base->set_interaction_callbacks(std::make_unique<AbortCallbacks>());

    TestMessage msg("Test");
    CPPUNIT_ASSERT_EQUAL(libdnf5::base::ANSWER_ABORT, base->confirm(msg, true));

    TestMessage opt("Option A");
    std::vector<libdnf5::Message *> choices = {&opt};
    CPPUNIT_ASSERT_EQUAL(libdnf5::base::ANSWER_ABORT, base->choice(msg, choices, 0));

    std::string text;
    CPPUNIT_ASSERT_EQUAL(libdnf5::base::ANSWER_ABORT, base->input_text(text, msg, nullptr, nullptr));
}
