# Copyright Contributors to the DNF5 project.
# SPDX-License-Identifier: GPL-2.0-or-later

require 'test/unit'
include Test::Unit::Assertions

require 'libdnf5/base'
require 'libdnf5/common'

require 'base_test_case'


class TestInteractionCallbacks < BaseTestCase
    # Custom Message implementation
    class TestMessage < Libdnf5::Common::Message
        attr_reader :text

        def initialize(text)
            super()
            @text = text
        end

        def format(translate, locale = nil)
            @text
        end
    end

    # Custom TextValidatorCallback implementation
    class NumberValidator < Libdnf5::Base::TextValidatorCallback
        def initialize()
            super()
        end

        def validate(input)
            return TestMessage.new("Input cannot be empty") if input.nil? || input.empty?

            if input =~ /^\d+$/
                nil  # Valid - return nil (nullptr)
            else
                TestMessage.new("Input must be a number")
            end
        end
    end

    # Custom InteractionCallbacks implementation
    class TestCallbacks < Libdnf5::Base::InteractionCallbacks
        attr_accessor :message_count, :confirm_count, :choice_count, :input_count
        attr_accessor :last_level, :last_message, :last_default_option, :last_default_text

        def initialize()
            super()
            @message_count = 0
            @confirm_count = 0
            @choice_count = 0
            @input_count = 0
            @last_level = nil
            @last_message = nil
            @last_default_option = 0
            @last_default_text = nil
            @progress_count = 0
            @next_handle = 0
            @last_progress_state = nil
            @last_count = 0
            @last_total = 0
            @last_progress_msg = nil
        end

        def message(level, msg)
            @message_count += 1
            @last_level = level
            @last_message = msg.format(false)
        end

        def confirm(msg, default_answer)
            @confirm_count += 1
            @last_message = msg.format(false)
            default_answer ? Libdnf5::Base::ANSWER_YES : Libdnf5::Base::ANSWER_NO
        end

        def choice(msg, choices, default_option)
            @choice_count += 1
            @last_message = msg.format(false)
            @last_default_option = default_option
            0  # Select first option for testing
        end

        def input_text(msg, default_text, validator)
            @input_count += 1
            @last_message = msg.format(false)
            @last_default_text = default_text

            if validator
                test_input = "42"
                error = validator.validate(test_input)
                if error.nil?
                    return [Libdnf5::Base::ANSWER_YES, test_input]
                end
            end

            [Libdnf5::Base::ANSWER_YES, "test input"]
        end

        def progress(handle, state, msg, count, total)
            @progress_count += 1
            @last_progress_state = state
            @last_count = count
            @last_total = total
            @last_progress_msg = msg ? msg.format(false) : nil
            if state == Libdnf5::Base::InteractionCallbacks::ProgressState_NEW
                @next_handle += 1
                return @next_handle
            end
            handle
        end
    end

    def test_interaction_callbacks()
        base = Libdnf5::Base::Base.new()

        callbacks = TestCallbacks.new()
        base.set_interaction_callbacks(
            Libdnf5::Base::InteractionCallbacksUniquePtr.new(callbacks)
        )

        # Test message
        base.message(
            Libdnf5::Base::InteractionCallbacks::MessageLevel_WARNING,
            TestMessage.new("Test warning")
        )
        assert_equal(1, callbacks.message_count, "Message callback called")
        assert_equal("Test warning", callbacks.last_message, "Message text correct")

        # Test confirm with default_answer=true
        result = base.confirm(TestMessage.new("Confirm?"), true)
        assert_equal(1, callbacks.confirm_count, "Confirm callback called")
        assert_equal(Libdnf5::Base::ANSWER_YES, result, "Confirm returned YES")

        # Test confirm with default_answer=false
        result = base.confirm(TestMessage.new("Confirm?"), false)
        assert_equal(2, callbacks.confirm_count, "Confirm callback called again")
        assert_equal(Libdnf5::Base::ANSWER_NO, result, "Confirm returned NO")

        # Test choice
        opt_a = TestMessage.new("Option A")
        opt_b = TestMessage.new("Option B")
        choices = Libdnf5::Common::VectorMessagePtr.new()
        choices << opt_a
        choices << opt_b
        choice_idx = base.choice(TestMessage.new("Select:"), choices, 0)
        assert_equal(1, callbacks.choice_count, "Choice callback called")
        assert_equal(0, choice_idx, "Choice returned first option")

        # Test input_text without validator
        result, text = base.input_text(TestMessage.new("Enter:"), nil, nil)
        assert_equal(1, callbacks.input_count, "Input_text callback called")
        assert_equal(Libdnf5::Base::ANSWER_YES, result, "Input returned YES")
        assert_equal("test input", text, "Input text correct")

        # Test input_text with validator
        validator = NumberValidator.new()
        result, text = base.input_text(
            TestMessage.new("Enter number:"),
            nil,
            validator
        )
        assert_equal(2, callbacks.input_count, "Input_text with validator called")
        assert_equal(Libdnf5::Base::ANSWER_YES, result, "Input returned YES")
        assert_equal("42", text, "Validated input correct")
    end

    def test_choice_with_default_option()
        base = Libdnf5::Base::Base.new()

        msg = TestMessage.new("Select:")
        opt_a = TestMessage.new("Option A")
        opt_b = TestMessage.new("Option B")
        opt_c = TestMessage.new("Option C")
        options = Libdnf5::Common::VectorMessagePtr.new()
        options << opt_a
        options << opt_b
        options << opt_c

        # Default implementation always returns ANSWER_DEFAULT regardless of options or default_option
        assert_equal(Libdnf5::Base::ANSWER_DEFAULT, base.choice(msg, options, 2))
        assert_equal(Libdnf5::Base::ANSWER_DEFAULT, base.choice(msg, options, 0))
        assert_equal(Libdnf5::Base::ANSWER_DEFAULT, base.choice(msg, options, Libdnf5::Base::ANSWER_DEFAULT))

        empty = Libdnf5::Common::VectorMessagePtr.new()
        assert_equal(Libdnf5::Base::ANSWER_DEFAULT, base.choice(msg, empty, 0))
    end

    def test_input_text_with_default_text()
        base = Libdnf5::Base::Base.new()

        msg = TestMessage.new("Enter:")

        # Default implementation always returns ANSWER_DEFAULT, out_text is never modified
        result, text = base.input_text(msg, "prefilled", nil)
        assert_equal(Libdnf5::Base::ANSWER_DEFAULT, result)

        result, text = base.input_text(msg, "", nil)
        assert_equal(Libdnf5::Base::ANSWER_DEFAULT, result)

        result, text = base.input_text(msg, nil, nil)
        assert_equal(Libdnf5::Base::ANSWER_DEFAULT, result)
    end

    def test_progress()
        base = Libdnf5::Base::Base.new()

        callbacks = TestCallbacks.new()
        base.set_interaction_callbacks(
            Libdnf5::Base::InteractionCallbacksUniquePtr.new(callbacks)
        )

        ic = Libdnf5::Base::InteractionCallbacks
        handle = base.progress(
            0, ic::ProgressState_NEW, TestMessage.new("Installing foo"), 0, 10
        )
        assert(handle >= 0, "NEW returns valid handle")
        assert_equal(1, callbacks.instance_variable_get(:@progress_count))
        assert_equal("Installing foo",
            callbacks.instance_variable_get(:@last_progress_msg))

        ret = base.progress(handle, ic::ProgressState_UPDATE, nil, 5, 10)
        assert(ret >= 0, "UPDATE returns non-negative")
        assert_equal(2, callbacks.instance_variable_get(:@progress_count))
        assert_equal(5, callbacks.instance_variable_get(:@last_count))
        assert_equal(10, callbacks.instance_variable_get(:@last_total))

        base.progress(handle, ic::ProgressState_END_OK, nil, 10, 10)
        assert_equal(3, callbacks.instance_variable_get(:@progress_count))
        assert_equal(ic::ProgressState_END_OK,
            callbacks.instance_variable_get(:@last_progress_state))
    end

    def test_abort_return_value()
        assert_equal(-4, Libdnf5::Base::ANSWER_ABORT)

        # Test that a script override can return ANSWER_ABORT as a bare integer
        abort_cb = Class.new(Libdnf5::Base::InteractionCallbacks) do
            def input_text(msg, default_text, validator)
                Libdnf5::Base::ANSWER_ABORT
            end
        end.new()

        base = Libdnf5::Base::Base.new()
        base.set_interaction_callbacks(Libdnf5::Base::InteractionCallbacksUniquePtr.new(abort_cb))
        result, text = base.input_text(TestMessage.new("msg"), nil, nil)
        assert_equal(Libdnf5::Base::ANSWER_ABORT, result)
    end
end
