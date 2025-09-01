# Copyright Contributors to the libdnf project.
# SPDX-License-Identifier: GPL-2.0-or-later
#
# This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
#
# Libdnf is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# Libdnf is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with libdnf.  If not, see <https://www.gnu.org/licenses/>.

import unittest

import libdnf5


class TestExceptions(unittest.TestCase):
    # List of all exceptions
    EXCEPTIONS = (
        libdnf5.exception.BaseTransactionError,
        libdnf5.exception.BaseTransactionErrorNested,

        libdnf5.exception.CompsInvalidPackageType,
        libdnf5.exception.CompsInvalidPackageTypeNested,

        #        libdnf5.module.NoModuleError,
        #        libdnf5.module.NoStreamError,
        #        libdnf5.module.EnabledStreamError,
        #        libdnf5.module.EnableMultipleStreamsError,
        #        libdnf5.module.ModuleConflictError,
        #        libdnf5.module.ModuleResolveError,
        #        libdnf5.module.ModuleError,

        #        libdnf5.module.InvalidModuleStatus,

        libdnf5.exception.RepoFileDownloadError,
        libdnf5.exception.RepoFileDownloadErrorNested,
        libdnf5.exception.RepoPackageDownloadError,
        libdnf5.exception.RepoPackageDownloadErrorNested,
        libdnf5.exception.RepoCacheError,
        libdnf5.exception.RepoCacheErrorNested,

        libdnf5.exception.RepoCacheonlyError,
        libdnf5.exception.RepoCacheonlyErrorNested,
        libdnf5.exception.RepoDownloadError,
        libdnf5.exception.RepoDownloadErrorNested,
        libdnf5.exception.RepoPgpError,
        libdnf5.exception.RepoPgpErrorNested,
        libdnf5.exception.RepoRpmError,
        libdnf5.exception.RepoRpmErrorNested,
        libdnf5.exception.RepoCompsError,
        libdnf5.exception.RepoCompsErrorNested,
        libdnf5.exception.RepoIdAlreadyExistsError,
        libdnf5.exception.RepoIdAlreadyExistsErrorNested,
        libdnf5.exception.RepoError,
        libdnf5.exception.RepoErrorNested,

        libdnf5.exception.RpmNevraIncorrectInputError,
        libdnf5.exception.RpmNevraIncorrectInputErrorNested,
        libdnf5.exception.RpmSignatureCheckError,
        libdnf5.exception.RpmSignatureCheckErrorNested,
        libdnf5.exception.RpmKeyImportError,
        libdnf5.exception.RpmKeyImportErrorNested,

        libdnf5.exception.TransactionInvalidTransactionItemAction,
        libdnf5.exception.TransactionInvalidTransactionItemActionNested,
        libdnf5.exception.TransactionInvalidTransactionItemReason,
        libdnf5.exception.TransactionInvalidTransactionItemReasonNested,
        libdnf5.exception.TransactionInvalidTransactionItemState,
        libdnf5.exception.TransactionInvalidTransactionItemStateNested,
        libdnf5.exception.TransactionInvalidTransactionItemType,
        libdnf5.exception.TransactionInvalidTransactionItemTypeNested,
        libdnf5.exception.TransactionInvalidTransactionState,
        libdnf5.exception.TransactionInvalidTransactionStateNested,

        libdnf5.exception.InaccessibleConfigError,
        libdnf5.exception.InaccessibleConfigErrorNested,
        libdnf5.exception.MissingConfigError,
        libdnf5.exception.MissingConfigErrorNested,
        libdnf5.exception.InvalidConfigError,
        libdnf5.exception.InvalidConfigErrorNested,

        libdnf5.exception.ConfigParserSectionNotFoundError,
        libdnf5.exception.ConfigParserSectionNotFoundErrorNested,
        libdnf5.exception.ConfigParserOptionNotFoundError,
        libdnf5.exception.ConfigParserOptionNotFoundErrorNested,
        libdnf5.exception.ConfigParserError,
        libdnf5.exception.ConfigParserErrorNested,

        libdnf5.exception.OptionValueNotSetError,
        libdnf5.exception.OptionValueNotSetErrorNested,
        libdnf5.exception.OptionValueNotAllowedError,
        libdnf5.exception.OptionValueNotAllowedErrorNested,
        libdnf5.exception.OptionInvalidValueError,
        libdnf5.exception.OptionInvalidValueErrorNested,
        libdnf5.exception.OptionError,
        libdnf5.exception.OptionErrorNested,

        libdnf5.exception.OptionBindsOptionNotFoundError,
        libdnf5.exception.OptionBindsOptionNotFoundErrorNested,
        libdnf5.exception.OptionBindsOptionAlreadyExistsError,
        libdnf5.exception.OptionBindsOptionAlreadyExistsErrorNested,
        libdnf5.exception.OptionBindsError,
        libdnf5.exception.OptionBindsErrorNested,

        libdnf5.exception.ReadOnlyVariableError,
        libdnf5.exception.ReadOnlyVariableErrorNested,

        libdnf5.exception.FileSystemError,
        libdnf5.exception.FileSystemErrorNested,
        libdnf5.exception.SystemError,
        libdnf5.exception.SystemErrorNested,
        libdnf5.exception.RuntimeError,
        libdnf5.exception.RuntimeErrorNested,
        libdnf5.exception.Error,
        libdnf5.exception.ErrorNested,
        libdnf5.exception.UserAssertionError,
        libdnf5.exception.UserAssertionErrorNested,
        libdnf5.exception.AssertionError,
        libdnf5.exception.AssertionErrorNested,

        libdnf5.exception.NonLibdnf5Exception,
        libdnf5.exception.NonLibdnf5ExceptionNested
    )

    def test_all_exceptions_without_ctor(self):
        # Check that exceptions do not have a defined constructor.
        for ex in self.EXCEPTIONS:
            with self.assertRaises(AttributeError, msg=str(ex)) as e:
                raise ex()
            self.assertEqual(str(e.exception),
                             'No constructor defined', str(ex))
