# Copyright Contributors to the DNF5 project.
# SPDX-License-Identifier: GPL-2.0-or-later

import unittest

import libdnf5.base
import libdnf5.common


# Custom Message implementation for testing
class TestMessage(libdnf5.common.Message):
    def __init__(self, text):
        super().__init__()
        self.text = text

    def format(self, translate, locale=None):
        return self.text


# Custom TextValidatorCallback implementation
class NumberValidator(libdnf5.base.TextValidatorCallback):
    def __init__(self):
        super().__init__()

    def validate(self, input_str):
        """Returns None if valid, or a Message with error description if invalid"""
        if not input_str:
            return TestMessage("Input cannot be empty")
        try:
            int(input_str)
            return None  # Valid
        except ValueError:
            return TestMessage("Input must be a number")


# Custom InteractionCallbacks implementation
class TestInteractionCallbacks(libdnf5.base.InteractionCallbacks):
    def __init__(self):
        super().__init__()
        self.message_count = 0
        self.confirm_count = 0
        self.choice_count = 0
        self.input_count = 0
        self.last_level = None
        self.last_message = None
        self.choice_options_count = 0
        self.last_default_option = 0
        self.last_default_text = None
        self.progress_count = 0
        self.next_handle = 0
        self.last_progress_state = None
        self.last_count = 0
        self.last_total = 0
        self.last_progress_msg = None

    def message(self, level, msg):
        self.message_count += 1
        self.last_level = level
        self.last_message = msg.format(False)

    def confirm(self, msg, default_answer):
        self.confirm_count += 1
        self.last_message = msg.format(False)
        return libdnf5.base.ANSWER_YES if default_answer else libdnf5.base.ANSWER_NO

    def choice(self, msg, choices, default_option):
        self.choice_count += 1
        self.last_message = msg.format(False)
        self.choice_options_count = len(choices)
        self.last_default_option = default_option
        return 0  # Select first option for testing

    def input_text(self, msg, default_text, validator):
        self.input_count += 1
        self.last_message = msg.format(False)
        self.last_default_text = default_text

        if validator:
            test_input = "42"
            error = validator.validate(test_input)
            if error is None:
                return (libdnf5.base.ANSWER_YES, test_input)

        return (libdnf5.base.ANSWER_YES, "test input")

    def progress(self, handle, state, msg, count, total):
        self.progress_count += 1
        self.last_progress_state = state
        self.last_count = count
        self.last_total = total
        self.last_progress_msg = msg.format(False) if msg else None
        if state == libdnf5.base.InteractionCallbacks.ProgressState_NEW:
            self.next_handle += 1
            return self.next_handle
        return handle


class TestInteractionCallbacksClass(unittest.TestCase):
    def test_message(self):
        base = libdnf5.base.Base()

        callbacks = TestInteractionCallbacks()
        base.set_interaction_callbacks(
            libdnf5.base.InteractionCallbacksUniquePtr(callbacks)
        )

        base.message(
            libdnf5.base.InteractionCallbacks.MessageLevel_WARNING,
            TestMessage("Test warning")
        )
        self.assertEqual(1, callbacks.message_count)
        self.assertEqual("Test warning", callbacks.last_message)
        self.assertEqual(
            libdnf5.base.InteractionCallbacks.MessageLevel_WARNING,
            callbacks.last_level
        )

    def test_confirm(self):
        base = libdnf5.base.Base()

        callbacks = TestInteractionCallbacks()
        base.set_interaction_callbacks(
            libdnf5.base.InteractionCallbacksUniquePtr(callbacks)
        )

        # Test confirm with default_answer=True
        result = base.confirm(TestMessage("Confirm?"), True)
        self.assertEqual(1, callbacks.confirm_count)
        self.assertEqual("Confirm?", callbacks.last_message)
        self.assertEqual(libdnf5.base.ANSWER_YES, result)

        # Test confirm with default_answer=False
        result = base.confirm(TestMessage("Confirm?"), False)
        self.assertEqual(2, callbacks.confirm_count)
        self.assertEqual(libdnf5.base.ANSWER_NO, result)

    def test_choice(self):
        base = libdnf5.base.Base()

        callbacks = TestInteractionCallbacks()
        base.set_interaction_callbacks(
            libdnf5.base.InteractionCallbacksUniquePtr(callbacks)
        )

        opt_a = TestMessage("Option A")
        opt_b = TestMessage("Option B")
        opt_c = TestMessage("Option C")
        choices = libdnf5.common.VectorMessagePtr()
        choices.append(opt_a)
        choices.append(opt_b)
        choices.append(opt_c)

        choice_idx = base.choice(TestMessage("Select:"), choices, 0)
        self.assertEqual(1, callbacks.choice_count)
        self.assertEqual("Select:", callbacks.last_message)
        self.assertEqual(0, choice_idx)
        self.assertEqual(3, callbacks.choice_options_count)

    def test_input_text(self):
        base = libdnf5.base.Base()

        callbacks = TestInteractionCallbacks()
        base.set_interaction_callbacks(
            libdnf5.base.InteractionCallbacksUniquePtr(callbacks)
        )

        result, text = base.input_text(TestMessage("Enter:"), None, None)
        self.assertEqual(1, callbacks.input_count)
        self.assertEqual("Enter:", callbacks.last_message)
        self.assertEqual(libdnf5.base.ANSWER_YES, result)
        self.assertEqual("test input", text)

    def test_input_text_with_validator(self):
        base = libdnf5.base.Base()

        callbacks = TestInteractionCallbacks()
        base.set_interaction_callbacks(
            libdnf5.base.InteractionCallbacksUniquePtr(callbacks)
        )

        validator = NumberValidator()
        result, text = base.input_text(TestMessage("Enter number:"), None, validator)
        self.assertEqual(1, callbacks.input_count)
        self.assertEqual("Enter number:", callbacks.last_message)
        self.assertEqual(libdnf5.base.ANSWER_YES, result)
        self.assertEqual("42", text)

    def test_choice_with_default_option(self):
        base = libdnf5.base.Base()

        msg = TestMessage("Select:")
        opt_a = TestMessage("Option A")
        opt_b = TestMessage("Option B")
        opt_c = TestMessage("Option C")
        options = libdnf5.common.VectorMessagePtr()
        options.append(opt_a)
        options.append(opt_b)
        options.append(opt_c)

        # Default implementation always returns ANSWER_DEFAULT regardless of options or default_option
        self.assertEqual(libdnf5.base.ANSWER_DEFAULT, base.choice(msg, options, 2))
        self.assertEqual(libdnf5.base.ANSWER_DEFAULT, base.choice(msg, options, 0))
        self.assertEqual(libdnf5.base.ANSWER_DEFAULT, base.choice(msg, options, libdnf5.base.ANSWER_DEFAULT))

        empty = libdnf5.common.VectorMessagePtr()
        self.assertEqual(libdnf5.base.ANSWER_DEFAULT, base.choice(msg, empty, 0))

    def test_input_text_with_default_text(self):
        base = libdnf5.base.Base()

        msg = TestMessage("Enter:")

        # Default implementation always returns ANSWER_DEFAULT, out_text is never modified
        result, text = base.input_text(msg, "prefilled", None)
        self.assertEqual(libdnf5.base.ANSWER_DEFAULT, result)

        result, text = base.input_text(msg, "", None)
        self.assertEqual(libdnf5.base.ANSWER_DEFAULT, result)

        result, text = base.input_text(msg, None, None)
        self.assertEqual(libdnf5.base.ANSWER_DEFAULT, result)

    def test_progress(self):
        base = libdnf5.base.Base()

        callbacks = TestInteractionCallbacks()
        base.set_interaction_callbacks(
            libdnf5.base.InteractionCallbacksUniquePtr(callbacks)
        )

        ProgressState = libdnf5.base.InteractionCallbacks
        handle = base.progress(
            0, ProgressState.ProgressState_NEW, TestMessage("Installing foo"), 0, 10
        )
        self.assertGreaterEqual(handle, 0)
        self.assertEqual(1, callbacks.progress_count)
        self.assertEqual("Installing foo", callbacks.last_progress_msg)

        ret = base.progress(
            handle, ProgressState.ProgressState_UPDATE, None, 5, 10
        )
        self.assertGreaterEqual(ret, 0)
        self.assertEqual(2, callbacks.progress_count)
        self.assertEqual(5, callbacks.last_count)
        self.assertEqual(10, callbacks.last_total)

        base.progress(
            handle, ProgressState.ProgressState_END_OK, None, 10, 10
        )
        self.assertEqual(3, callbacks.progress_count)
        self.assertEqual(
            ProgressState.ProgressState_END_OK, callbacks.last_progress_state
        )
