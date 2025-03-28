#if defined(SWIGPYTHON)
%module(package="libdnf5") exception
#elif defined(SWIGPERL)
%module "libdnf5::exception"
#elif defined(SWIGRUBY)
%module "libdnf5/exception"
#endif

%include <exception.i>
%include <cstring.i>
%include <std_string.i>

%include "shared.i"

%{
    #include "bindings/libdnf5/exception.hpp"
%}

%typemap(throws, noblock=1) libdnf5::AssertionError, libdnf5::UserAssertionError, libdnf5::Error, NonLibdnf5Exception, std::runtime_error {
    create_swig_exception($1);
    SWIG_fail;
}

// Deletes any previously defined general purpose exception handler
%exception;

// Set default exception handler
%catches(libdnf5::UserAssertionError, std::runtime_error, std::out_of_range);

// Parent exception classes must be listed and used at least once in `%catches` for SWIG to consider them
// as classes implementing the exception.
// In general, we want to use `%catches` with as few exceptions listed as possible. Therefore,
// this `%catches` is defined for only one method.
%catches(
    libdnf5::AssertionError,
    libdnf5::UserAssertionError,
    libdnf5::Error,
    NonLibdnf5Exception,
    std::runtime_error,
    std::out_of_range) rethrow_if_nested;

// Optimize -> No need exception handler from `noexcept` methods
%catches() what;
%catches() get_name;
%catches() get_domain_name;
%catches() get_error_code;

%ignore NonLibdnf5Exception::NonLibdnf5Exception;
%ignore create_swig_exception;
%include "bindings/libdnf5/exception.hpp"

// Ignore macros
%ignore LIBDNF_LOCATION;
%ignore LIBDNF_ASSERTION_MACROS;
%ignore libdnf_throw_assertion;
%ignore libdnf_assert;
%ignore libdnf_user_assert;

%ignore libdnf5::SourceLocation;
%ignore libdnf5::Error::Error;
%ignore libdnf5::AssertionError::AssertionError;
%ignore libdnf5::UserAssertionError::UserAssertionError;
%ignore libdnf5::FileSystemError::FileSystemError;
%ignore libdnf5::SystemError::SystemError;
%ignore libdnf5::NestedException::NestedException;
%ignore libdnf5::format;
%include "libdnf5/common/exception.hpp"

%ignore libdnf5::base::TransactionError::TransactionError;
%rename(BaseTransactionError) libdnf5::base::TransactionError;
%include "libdnf5/base/transaction_errors.hpp"

%ignore libdnf5::comps::InvalidPackageType::InvalidPackageType;
%rename(CompsInvalidPackageType) libdnf5::comps::InvalidPackageType;
%include "libdnf5/comps/group/package_errors.hpp"

%ignore libdnf5::InaccessibleConfigError::InaccessibleConfigError;
%ignore libdnf5::MissingConfigError::MissingConfigError;
%ignore libdnf5::InvalidConfigError::InvalidConfigError;
%ignore libdnf5::ConfigParserError::ConfigParserError;
%ignore libdnf5::ConfigParserSectionNotFoundError::ConfigParserSectionNotFoundError;
%ignore libdnf5::ConfigParserOptionNotFoundError::ConfigParserOptionNotFoundError;
%include "libdnf5/conf/config_parser_errors.hpp"

%ignore libdnf5::OptionError::OptionError;
%ignore libdnf5::OptionInvalidValueError::OptionInvalidValueError;
%ignore libdnf5::OptionValueNotAllowedError::OptionValueNotAllowedError;
%ignore libdnf5::OptionValueNotSetError::OptionValueNotSetError;
%include "libdnf5/conf/option_errors.hpp"

%ignore libdnf5::OptionPathNotFoundError::OptionPathNotFoundError;
%include "libdnf5/conf/option_path_errors.hpp"

%ignore libdnf5::OptionBindsError::OptionBindsError;
%ignore libdnf5::OptionBindsOptionNotFoundError::OptionBindsOptionNotFoundError;
%ignore libdnf5::OptionBindsOptionAlreadyExistsError::OptionBindsOptionAlreadyExistsError;
%include "libdnf5/conf/option_binds_errors.hpp"

%ignore libdnf5::ReadOnlyVariableError::ReadOnlyVariableError;
%include "libdnf5/conf/vars_errors.hpp"

//%include "libdnf5/module/module_errors.hpp"

%ignore libdnf5::repo::FileDownloadError::FileDownloadError;
%rename(RepoFileDownloadError) libdnf5::repo::FileDownloadError;
%include "libdnf5/repo/file_downloader_errors.hpp"

%ignore libdnf5::repo::PackageDownloadError::PackageDownloadError;
%rename(RepoPackageDownloadError) libdnf5::repo::PackageDownloadError;
%include "libdnf5/repo/package_downloader_errors.hpp"

%ignore libdnf5::repo::RepoCacheError::RepoCacheError;
%include "libdnf5/repo/repo_cache_errors.hpp"

%ignore libdnf5::repo::RepoCacheonlyError::RepoCacheonlyError;
%ignore libdnf5::repo::RepoDownloadError::RepoDownloadError;
%ignore libdnf5::repo::RepoPgpError::RepoPgpError;
%ignore libdnf5::repo::RepoRpmError::RepoRpmError;
%ignore libdnf5::repo::RepoCompsError::RepoCompsError;
%ignore libdnf5::repo::RepoIdAlreadyExistsError::RepoIdAlreadyExistsError;
%ignore libdnf5::repo::RepoError::RepoError;
%include "libdnf5/repo/repo_errors.hpp"

%ignore libdnf5::rpm::NevraIncorrectInputError::NevraIncorrectInputError;
%rename(RpmNevraIncorrectInputError) libdnf5::rpm::NevraIncorrectInputError;
%include "libdnf5/rpm/nevra_errors.hpp"

%ignore libdnf5::rpm::KeyImportError::KeyImportError;
%ignore libdnf5::rpm::SignatureCheckError::SignatureCheckError;
%rename(RpmKeyImportError) libdnf5::rpm::KeyImportError;
%rename(RpmSignatureCheckError) libdnf5::rpm::SignatureCheckError;
%include "libdnf5/rpm/rpm_signature_errors.hpp"

%ignore libdnf5::transaction::InvalidTransactionState::InvalidTransactionState;
%rename(TransactionInvalidTransactionState) libdnf5::transaction::InvalidTransactionState;
%include "libdnf5/transaction/transaction_errors.hpp"

%ignore libdnf5::transaction::InvalidTransactionItemAction::InvalidTransactionItemAction;
%ignore libdnf5::transaction::InvalidTransactionItemReason::InvalidTransactionItemReason;
%ignore libdnf5::transaction::InvalidTransactionItemState::InvalidTransactionItemState;
%ignore libdnf5::transaction::InvalidTransactionItemType::InvalidTransactionItemType;
%rename(TransactionInvalidTransactionItemAction) libdnf5::transaction::InvalidTransactionItemAction;
%rename(TransactionInvalidTransactionItemReason) libdnf5::transaction::InvalidTransactionItemReason;
%rename(TransactionInvalidTransactionItemState) libdnf5::transaction::InvalidTransactionItemState;
%rename(TransactionInvalidTransactionItemType) libdnf5::transaction::InvalidTransactionItemType;
%include "libdnf5/transaction/transaction_item_errors.hpp"

%include "libdnf5/utils/bgettext/bgettext-mark.h"

%extend libdnf5::AssertionError {
    std::string format(libdnf5::FormatDetailLevel detail) const {
        return libdnf5::format(*$self, detail);
    }

    void rethrow_if_nested() const {
        std::rethrow_if_nested(*$self);
    }
}

%extend libdnf5::UserAssertionError {
    std::string format(libdnf5::FormatDetailLevel detail) const {
        return libdnf5::format(*$self, detail);
    }

    void rethrow_if_nested() const {
        std::rethrow_if_nested(*$self);
    }
}

%extend libdnf5::Error {
    std::string format(libdnf5::FormatDetailLevel detail) const {
        return libdnf5::format(*$self, detail);
    }

    void rethrow_if_nested() const {
        std::rethrow_if_nested(*$self);
    }
}

%extend NonLibdnf5Exception {
    std::string format(libdnf5::FormatDetailLevel detail) const {
        return libdnf5::format(*$self, detail);
    }

    void rethrow_if_nested() const {
        std::rethrow_if_nested(*$self);
    }

    const char * what() const noexcept {
        return $self->what();
    }
}

#if defined(SWIGPYTHON)

%catches() __str__;

%extend libdnf5::AssertionError {
    std::string __str__() const {
        return $self->what();
    }
}

%extend libdnf5::UserAssertionError {
    std::string __str__() const {
        return $self->what();
    }
}

%extend libdnf5::Error {
    std::string __str__() const {
        return $self->what();
    }
}

%extend NonLibdnf5Exception {
    std::string __str__() const {
        return $self->what();
    }
}

#elif defined(SWIGRUBY)

%catches() to_s;

%extend libdnf5::AssertionError {
    std::string to_s() const {
        return $self->what();
    }
}

%extend libdnf5::UserAssertionError {
    std::string to_s() const {
        return $self->what();
    }
}

%extend libdnf5::Error {
    std::string to_s() const {
        return $self->what();
    }
}

%extend NonLibdnf5Exception {
    std::string to_s() const {
        return $self->what();
    }
}

#endif


%template(AssertionErrorNested) libdnf5::NestedException<libdnf5::AssertionError>;
%template(UserAssertionErrorNested) libdnf5::NestedException<libdnf5::UserAssertionError>;
%template(ErrorNested) libdnf5::NestedException<libdnf5::Error>;
%template(SystemErrorNested) libdnf5::NestedException<libdnf5::SystemError>;
%template(FileSystemErrorNested) libdnf5::NestedException<libdnf5::FileSystemError>;
%template(RuntimeErrorNested) libdnf5::NestedException<libdnf5::RuntimeError>;

%template(BaseTransactionErrorNested) libdnf5::NestedException<libdnf5::base::TransactionError>;

%template(CompsInvalidPackageTypeNested) libdnf5::NestedException<libdnf5::comps::InvalidPackageType>;

/* TODO: Modularity does not yet have a swig binding defined.
%template(ModuleNoModuleErrorNested) libdnf5::NestedException<libdnf5::module::NoModuleError>;
%template(ModuleNoStreamErrorNested) libdnf5::NestedException<libdnf5::module::NoStreamError>;
%template(ModuleEnableStreamErrorNested) libdnf5::NestedException<libdnf5::module::EnabledStreamError>;
%template(ModuleEnableMultipleStreamErrorNested) libdnf5::NestedException<libdnf5::module::EnableMultipleStreamsError>;
%template(ModuleModuleConflictErrorNested) libdnf5::NestedException<libdnf5::module::ModuleConflictError>;
%template(ModuleModuleResolveErrorNested) libdnf5::NestedException<libdnf5::module::ModuleResolveError>;
%template(ModuleModuleErrorNested) libdnf5::NestedException<libdnf5::module::ModuleError>;

%template(ModuleInvalidModuleStatusNested) libdnf5::NestedException<libdnf5::module::InvalidModuleStatus>;
*/

%template(RepoFileDownloadErrorNested) libdnf5::NestedException<libdnf5::repo::FileDownloadError>;
%template(RepoPackageDownloadErrorNested) libdnf5::NestedException<libdnf5::repo::PackageDownloadError>;
%template(RepoCacheErrorNested) libdnf5::NestedException<libdnf5::repo::RepoCacheError>;

%template(RepoCacheonlyErrorNested) libdnf5::NestedException<libdnf5::repo::RepoCacheonlyError>;
%template(RepoDownloadErrorNested) libdnf5::NestedException<libdnf5::repo::RepoDownloadError>;
%template(RepoPgpErrorNested) libdnf5::NestedException<libdnf5::repo::RepoPgpError>;
%template(RepoRpmErrorNested) libdnf5::NestedException<libdnf5::repo::RepoRpmError>;
%template(RepoCompsErrorNested) libdnf5::NestedException<libdnf5::repo::RepoCompsError>;
%template(RepoIdAlreadyExistsErrorNested) libdnf5::NestedException<libdnf5::repo::RepoIdAlreadyExistsError>;
%template(RepoErrorNested) libdnf5::NestedException<libdnf5::repo::RepoError>;

%template(RpmNevraIncorrectInputErrorNested) libdnf5::NestedException<libdnf5::rpm::NevraIncorrectInputError>;
%template(RpmSignatureCheckErrorNested) libdnf5::NestedException<libdnf5::rpm::SignatureCheckError>;
%template(RpmKeyImportErrorNested) libdnf5::NestedException<libdnf5::rpm::KeyImportError>;

%template(TransactionInvalidTransactionItemActionNested) libdnf5::NestedException<libdnf5::transaction::InvalidTransactionItemAction>;
%template(TransactionInvalidTransactionItemReasonNested) libdnf5::NestedException<libdnf5::transaction::InvalidTransactionItemReason>;
%template(TransactionInvalidTransactionItemStateNested) libdnf5::NestedException<libdnf5::transaction::InvalidTransactionItemState>;
%template(TransactionInvalidTransactionItemTypeNested) libdnf5::NestedException<libdnf5::transaction::InvalidTransactionItemType>;
%template(TransactionInvalidTransactionStateNested) libdnf5::NestedException<libdnf5::transaction::InvalidTransactionState>;

%template(InaccessibleConfigErrorNested) libdnf5::NestedException<libdnf5::InaccessibleConfigError>;
%template(MissingConfigErrorNested) libdnf5::NestedException<libdnf5::MissingConfigError>;
%template(InvalidConfigErrorNested) libdnf5::NestedException<libdnf5::InvalidConfigError>;

%template(ConfigParserSectionNotFoundErrorNested) libdnf5::NestedException<libdnf5::ConfigParserSectionNotFoundError>;
%template(ConfigParserOptionNotFoundErrorNested) libdnf5::NestedException<libdnf5::ConfigParserOptionNotFoundError>;
%template(ConfigParserErrorNested) libdnf5::NestedException<libdnf5::ConfigParserError>;

%template(OptionValueNotSetErrorNested) libdnf5::NestedException<libdnf5::OptionValueNotSetError>;
%template(OptionValueNotAllowedErrorNested) libdnf5::NestedException<libdnf5::OptionValueNotAllowedError>;
%template(OptionInvalidValueErrorNested) libdnf5::NestedException<libdnf5::OptionInvalidValueError>;
%template(OptionErrorNested) libdnf5::NestedException<libdnf5::OptionError>;

%template(OptionBindsOptionNotFoundErrorNested) libdnf5::NestedException<libdnf5::OptionBindsOptionNotFoundError>;
%template(OptionBindsOptionAlreadyExistsErrorNested) libdnf5::NestedException<libdnf5::OptionBindsOptionAlreadyExistsError>;
%template(OptionBindsErrorNested) libdnf5::NestedException<libdnf5::OptionBindsError>;

%template(ReadOnlyVariableErrorNested) libdnf5::NestedException<libdnf5::ReadOnlyVariableError>;

%template(NonLibdnf5ExceptionNested) libdnf5::NestedException<NonLibdnf5Exception>;

// Deletes any previously defined catches
%catches();
