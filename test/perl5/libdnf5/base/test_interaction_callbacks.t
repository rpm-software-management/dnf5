#!/usr/bin/perl
# Copyright Contributors to the DNF5 project.
# SPDX-License-Identifier: GPL-2.0-or-later

use strict;
use warnings;

use Test::More;

use FindBin;
use lib "$FindBin::Bin/..";
use BaseTestCase;

# Custom Message implementation
{
    package TestMessage;
    use base qw(libdnf5::common::Message);

    sub new {
        my ($class, $text) = @_;
        my $self = $class->SUPER::new();
        $self->{text} = $text;
        return bless($self, $class);
    }

    sub format {
        my ($self, $translate, $locale) = @_;
        return $self->{text};
    }
}

# Custom TextValidatorCallback implementation
{
    package NumberValidator;
    use base qw(libdnf5::base::TextValidatorCallback);

    sub new {
        my $class = shift;
        my $self = $class->SUPER::new(@_);
        return bless($self, $class);
    }

    sub validate {
        my ($self, $input) = @_;
        return TestMessage->new("Input cannot be empty") if !defined($input) || $input eq '';

        if ($input =~ /^\d+$/) {
            return undef;  # Valid - return undef (nullptr)
        } else {
            return TestMessage->new("Input must be a number");
        }
    }
}

# InteractionCallbacks that returns ANSWER_ABORT from input_text
{
    package AbortInputCallbacks;
    use base qw(libdnf5::base::InteractionCallbacks);

    sub new {
        my $class = shift;
        return bless($class->SUPER::new(@_), $class);
    }

    sub input_text { return $libdnf5::base::ANSWER_ABORT; }
}

# Custom InteractionCallbacks implementation
{
    package TestInteractionCallbacks;
    use base qw(libdnf5::base::InteractionCallbacks);

    sub new {
        my $class = shift;
        my $self = $class->SUPER::new(@_);
        $self->{message_count} = 0;
        $self->{confirm_count} = 0;
        $self->{progress_count} = 0;
        $self->{next_handle} = 0;
        $self->{last_progress_state} = undef;
        $self->{last_count} = 0;
        $self->{last_total} = 0;
        $self->{last_progress_msg} = undef;
        $self->{choice_count} = 0;
        $self->{input_count} = 0;
        $self->{last_level} = undef;
        $self->{last_message} = undef;
        return bless($self, $class);
    }

    sub message {
        my ($self, $level, $msg) = @_;
        $self->{message_count}++;
        $self->{last_level} = $level;
        $self->{last_message} = $msg->format(0);
    }

    sub confirm {
        my ($self, $msg, $default_answer) = @_;
        $self->{confirm_count}++;
        $self->{last_message} = $msg->format(0);
        return $default_answer ? $libdnf5::base::ANSWER_YES : $libdnf5::base::ANSWER_NO;
    }

    sub choice {
        my ($self, $msg, $choices, $default_option) = @_;
        $self->{choice_count}++;
        $self->{last_message} = $msg->format(0);
        $self->{last_default_option} = $default_option;
        return 0;  # Select first option for testing
    }

    sub input_text {
        my ($self, $msg, $default_text, $validator) = @_;
        $self->{input_count}++;
        $self->{last_message} = $msg->format(0);
        $self->{last_default_text} = $default_text;

        if (defined $validator) {
            my $test_input = "42";
            my $error = $validator->validate($test_input);
            if (!defined $error) {
                return [$libdnf5::base::ANSWER_YES, $test_input];
            }
        }

        return [$libdnf5::base::ANSWER_YES, "test input"];
    }

    sub progress {
        my ($self, $handle, $state, $msg, $count, $total) = @_;
        $self->{progress_count}++;
        $self->{last_progress_state} = $state;
        $self->{last_count} = $count;
        $self->{last_total} = $total;
        $self->{last_progress_msg} = defined $msg ? $msg->format(0) : undef;
        if ($state == $libdnf5::base::InteractionCallbacks::ProgressState_NEW) {
            $self->{next_handle}++;
            return $self->{next_handle};
        }
        return $handle;
    }
}

subtest 'interaction_callbacks_test' => sub {
    my $base = libdnf5::base::Base->new();

    my $callbacks = TestInteractionCallbacks->new();
    $base->set_interaction_callbacks(
        libdnf5::base::InteractionCallbacksUniquePtr->new($callbacks)
    );

    # Test message
    $base->message(
        $libdnf5::base::InteractionCallbacks::MessageLevel_WARNING,
        TestMessage->new("Test warning")
    );
    is($callbacks->{message_count}, 1, "Message callback called");
    is($callbacks->{last_message}, "Test warning", "Message text correct");

    # Test confirm with default_answer=true
    my $result = $base->confirm(TestMessage->new("Confirm?"), 1);
    is($callbacks->{confirm_count}, 1, "Confirm callback called");
    is($result, $libdnf5::base::ANSWER_YES, "Confirm returned YES");

    # Test confirm with default_answer=false
    $result = $base->confirm(TestMessage->new("Confirm?"), 0);
    is($callbacks->{confirm_count}, 2, "Confirm callback called again");
    is($result, $libdnf5::base::ANSWER_NO, "Confirm returned NO");

    # Test choice
    my $opt_a = TestMessage->new("Option A");
    my $opt_b = TestMessage->new("Option B");
    my $choices = libdnf5::common::VectorMessagePtr->new();
    $choices->push($opt_a);
    $choices->push($opt_b);
    my $choice_idx = $base->choice(TestMessage->new("Select:"), $choices, 0);
    is($callbacks->{choice_count}, 1, "Choice callback called");
    is($choice_idx, 0, "Choice returned first option");

    # Test input_text without validator
    my ($ok, $text) = $base->input_text(TestMessage->new("Enter:"), undef, undef);
    is($callbacks->{input_count}, 1, "Input_text callback called");
    is($ok, $libdnf5::base::ANSWER_YES, "Input returned YES");
    is($text, "test input", "Input text correct");

    # Test input_text with validator
    my $validator = NumberValidator->new();
    ($ok, $text) = $base->input_text(
        TestMessage->new("Enter number:"),
        undef,
        $validator
    );
    is($callbacks->{input_count}, 2, "Input_text with validator called");
    is($ok, $libdnf5::base::ANSWER_YES, "Input returned YES");
    is($text, "42", "Validated input correct");
};

subtest 'choice_with_default_option' => sub {
    my $base = libdnf5::base::Base->new();

    my $msg = TestMessage->new("Select:");
    my $opt_a = TestMessage->new("Option A");
    my $opt_b = TestMessage->new("Option B");
    my $opt_c = TestMessage->new("Option C");
    my $options = libdnf5::common::VectorMessagePtr->new();
    $options->push($opt_a);
    $options->push($opt_b);
    $options->push($opt_c);

    # Default implementation always returns ANSWER_DEFAULT regardless of options or default_option
    is($base->choice($msg, $options, 2), $libdnf5::base::ANSWER_DEFAULT, "Default choice 2 returns DEFAULT");
    is($base->choice($msg, $options, 0), $libdnf5::base::ANSWER_DEFAULT, "Default choice 0 returns DEFAULT");
    is($base->choice($msg, $options, $libdnf5::base::ANSWER_DEFAULT), $libdnf5::base::ANSWER_DEFAULT, "DEFAULT returns DEFAULT");

    my $empty = libdnf5::common::VectorMessagePtr->new();
    is($base->choice($msg, $empty, 0), $libdnf5::base::ANSWER_DEFAULT, "Empty options returns DEFAULT");
};

subtest 'input_text_with_default_text' => sub {
    my $base = libdnf5::base::Base->new();

    my $msg = TestMessage->new("Enter:");

    # Default implementation always returns ANSWER_DEFAULT, out_text is never modified
    my ($ok, $text) = $base->input_text($msg, "prefilled", undef);
    is($ok, $libdnf5::base::ANSWER_DEFAULT, "prefilled returns DEFAULT");

    ($ok, $text) = $base->input_text($msg, "", undef);
    is($ok, $libdnf5::base::ANSWER_DEFAULT, "empty default returns DEFAULT");

    ($ok, $text) = $base->input_text($msg, undef, undef);
    is($ok, $libdnf5::base::ANSWER_DEFAULT, "no default returns DEFAULT");
};

subtest 'progress' => sub {
    my $base = libdnf5::base::Base->new();

    my $callbacks = TestInteractionCallbacks->new();
    $base->set_interaction_callbacks(
        libdnf5::base::InteractionCallbacksUniquePtr->new($callbacks)
    );

    my $NEW = $libdnf5::base::InteractionCallbacks::ProgressState_NEW;
    my $UPDATE = $libdnf5::base::InteractionCallbacks::ProgressState_UPDATE;
    my $END_OK = $libdnf5::base::InteractionCallbacks::ProgressState_END_OK;

    my $handle = $base->progress(0, $NEW, TestMessage->new("Installing foo"), 0, 10);
    ok($handle >= 0, "NEW returns valid handle");
    is($callbacks->{progress_count}, 1, "Progress count is 1");
    is($callbacks->{last_progress_msg}, "Installing foo", "Message passed");

    my $ret = $base->progress($handle, $UPDATE, undef, 5, 10);
    ok($ret >= 0, "UPDATE returns non-negative");
    is($callbacks->{progress_count}, 2, "Progress count is 2");
    is($callbacks->{last_count}, 5, "Count is 5");
    is($callbacks->{last_total}, 10, "Total is 10");

    $base->progress($handle, $END_OK, undef, 10, 10);
    is($callbacks->{progress_count}, 3, "Progress count is 3");
    is($callbacks->{last_progress_state}, $END_OK, "State is END_OK");
};

subtest 'abort_return_value' => sub {
    is($libdnf5::base::ANSWER_ABORT, -4, "ABORT constant has value -4");

    # Test that a script override can return ANSWER_ABORT as a bare integer
    my $base = libdnf5::base::Base->new();
    my $cb = AbortInputCallbacks->new();
    $base->set_interaction_callbacks(libdnf5::base::InteractionCallbacksUniquePtr->new($cb));
    my ($result, $text) = $base->input_text(TestMessage->new("msg"), undef, undef);
    is($result, $libdnf5::base::ANSWER_ABORT, "Script override returning ANSWER_ABORT works");
};

done_testing();
