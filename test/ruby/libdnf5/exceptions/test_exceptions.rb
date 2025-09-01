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

require 'test/unit'
include Test::Unit::Assertions

require 'libdnf5/exception'

class TestExceptions < Test::Unit::TestCase
    # List of all exceptions
    EXCEPTIONS = [
        Libdnf5::Exception::BaseTransactionError,
        Libdnf5::Exception::BaseTransactionErrorNested,

        Libdnf5::Exception::CompsInvalidPackageType,
        Libdnf5::Exception::CompsInvalidPackageTypeNested,

        # Libdnf5::Exception::NoModuleError,
        # Libdnf5::Exception::NoStreamError,
        # Libdnf5::Exception::EnabledStreamError,
        # Libdnf5::Exception::EnableMultipleStreamsError,
        # Libdnf5::Exception::ModuleConflictError,
        # Libdnf5::Exception::ModuleResolveError,
        # Libdnf5::Exception::ModuleError,

        # Libdnf5::Exception::InvalidModuleStatus,

        Libdnf5::Exception::RepoFileDownloadError,
        Libdnf5::Exception::RepoFileDownloadErrorNested,
        Libdnf5::Exception::RepoPackageDownloadError,
        Libdnf5::Exception::RepoPackageDownloadErrorNested,
        Libdnf5::Exception::RepoCacheError,
        Libdnf5::Exception::RepoCacheErrorNested,

        Libdnf5::Exception::RepoCacheonlyError,
        Libdnf5::Exception::RepoCacheonlyErrorNested,
        Libdnf5::Exception::RepoDownloadError,
        Libdnf5::Exception::RepoDownloadErrorNested,
        Libdnf5::Exception::RepoPgpError,
        Libdnf5::Exception::RepoPgpErrorNested,
        Libdnf5::Exception::RepoRpmError,
        Libdnf5::Exception::RepoRpmErrorNested,
        Libdnf5::Exception::RepoCompsError,
        Libdnf5::Exception::RepoCompsErrorNested,
        Libdnf5::Exception::RepoIdAlreadyExistsError,
        Libdnf5::Exception::RepoIdAlreadyExistsErrorNested,
        Libdnf5::Exception::RepoError,
        Libdnf5::Exception::RepoErrorNested,

        Libdnf5::Exception::RpmNevraIncorrectInputError,
        Libdnf5::Exception::RpmNevraIncorrectInputErrorNested,
        Libdnf5::Exception::RpmSignatureCheckError,
        Libdnf5::Exception::RpmSignatureCheckErrorNested,
        Libdnf5::Exception::RpmKeyImportError,
        Libdnf5::Exception::RpmKeyImportErrorNested,

        Libdnf5::Exception::TransactionInvalidTransactionItemAction,
        Libdnf5::Exception::TransactionInvalidTransactionItemActionNested,
        Libdnf5::Exception::TransactionInvalidTransactionItemReason,
        Libdnf5::Exception::TransactionInvalidTransactionItemReasonNested,
        Libdnf5::Exception::TransactionInvalidTransactionItemState,
        Libdnf5::Exception::TransactionInvalidTransactionItemStateNested,
        Libdnf5::Exception::TransactionInvalidTransactionItemType,
        Libdnf5::Exception::TransactionInvalidTransactionItemTypeNested,
        Libdnf5::Exception::TransactionInvalidTransactionState,
        Libdnf5::Exception::TransactionInvalidTransactionStateNested,

        Libdnf5::Exception::InaccessibleConfigError,
        Libdnf5::Exception::InaccessibleConfigErrorNested,
        Libdnf5::Exception::MissingConfigError,
        Libdnf5::Exception::MissingConfigErrorNested,
        Libdnf5::Exception::InvalidConfigError,
        Libdnf5::Exception::InvalidConfigErrorNested,

        Libdnf5::Exception::ConfigParserSectionNotFoundError,
        Libdnf5::Exception::ConfigParserSectionNotFoundErrorNested,
        Libdnf5::Exception::ConfigParserOptionNotFoundError,
        Libdnf5::Exception::ConfigParserOptionNotFoundErrorNested,
        Libdnf5::Exception::ConfigParserError,
        Libdnf5::Exception::ConfigParserErrorNested,

        Libdnf5::Exception::OptionValueNotSetError,
        Libdnf5::Exception::OptionValueNotSetErrorNested,
        Libdnf5::Exception::OptionValueNotAllowedError,
        Libdnf5::Exception::OptionValueNotAllowedErrorNested,
        Libdnf5::Exception::OptionInvalidValueError,
        Libdnf5::Exception::OptionInvalidValueErrorNested,
        Libdnf5::Exception::OptionError,
        Libdnf5::Exception::OptionErrorNested,

        Libdnf5::Exception::OptionBindsOptionNotFoundError,
        Libdnf5::Exception::OptionBindsOptionNotFoundErrorNested,
        Libdnf5::Exception::OptionBindsOptionAlreadyExistsError,
        Libdnf5::Exception::OptionBindsOptionAlreadyExistsErrorNested,
        Libdnf5::Exception::OptionBindsError,
        Libdnf5::Exception::OptionBindsErrorNested,

        Libdnf5::Exception::ReadOnlyVariableError,
        Libdnf5::Exception::ReadOnlyVariableErrorNested,

        Libdnf5::Exception::FileSystemError,
        Libdnf5::Exception::FileSystemErrorNested,
        Libdnf5::Exception::SystemError,
        Libdnf5::Exception::SystemErrorNested,
        Libdnf5::Exception::RuntimeError,
        Libdnf5::Exception::RuntimeErrorNested,
        Libdnf5::Exception::Error,
        Libdnf5::Exception::ErrorNested,
        Libdnf5::Exception::UserAssertionError,
        Libdnf5::Exception::UserAssertionErrorNested,
        Libdnf5::Exception::AssertionError,
        Libdnf5::Exception::AssertionErrorNested,

        Libdnf5::Exception::NonLibdnf5Exception,
        Libdnf5::Exception::NonLibdnf5ExceptionNested
    ]

    def test_all_exceptions_without_ctor()
        # Check that exceptions do not have a defined constructor.
        for ex in EXCEPTIONS do
            exception = assert_raises(TypeError, ex.to_s()) do
                ex.new()
            end
            assert_match(/allocator undefined/, exception.message, ex.to_s())
        end
    end
end
