# Copyright Contributors to the DNF5 project
# SPDX-License-Identifier: LGPL-2.1-or-later

import os

import libdnf5.base

import base_test_case


class CallbackError(Exception):
    pass


class ThrowingCallbacks(libdnf5.rpm.TransactionCallbacks):
    def __init__(self):
        super(ThrowingCallbacks, self).__init__()
        self.reached = False

    def verify_start(self, total=0):
        self.reached = True
        raise CallbackError("exception from a transaction callback")


class TestRpmTransaction(base_test_case.BaseTestCase):
    def test_transaction_callback_exception(self):
        self.addCleanup(os.chdir, os.getcwd())

        self.add_repo_rpm("rpm-repo1")

        goal = libdnf5.base.Goal(self.base)
        goal.add_rpm_install("one")

        transaction = goal.resolve()
        transaction.download()

        callbacks = ThrowingCallbacks()
        transaction.set_callbacks(
            libdnf5.rpm.TransactionCallbacksUniquePtr(callbacks))
        transaction.set_description("install package one")

        threw = False
        try:
            transaction.run()
        except CallbackError:
            threw = True

        self.assertEqual(callbacks.reached, threw)
