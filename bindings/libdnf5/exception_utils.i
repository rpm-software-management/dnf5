#if defined(SWIGPYTHON)
%module(package="libdnf5") exception_utils
#elif defined(SWIGPERL)
%module "libdnf5::exception_utils"
#elif defined(SWIGRUBY)
%module "libdnf5::exception_utils"
#endif

%include <cstring.i>
%include <exception.i>
%include <std_string.i>


%include <shared.i>

%import "advisory.i"
%import "base.i"
%import "common.i"
%import "comps.i"
%import "conf.i"
%import "logger.i"
%import "plugin.i"
%import "repo.i"
%import "rpm.i"
%import "transaction.i"
%import "utils.i"

%{
    #include "libdnf5/base/transaction.hpp"
    #include "libdnf5/common/exception.hpp"
    #include "libdnf5/comps/group/package_type.hpp"
    #include "libdnf5/conf/config_parser.hpp"
    #include "libdnf5/conf/option.hpp"
    #include "libdnf5/conf/option_binds.hpp"
    #include "libdnf5/conf/vars.hpp"
    #include "libdnf5/module/module_errors.hpp"
    #include "libdnf5/module/module_status.hpp"
    #include "libdnf5/repo/file_downloader.hpp"
    #include "libdnf5/repo/package_downloader.hpp"
    #include "libdnf5/repo/repo_cache.hpp"
    #include "libdnf5/repo/repo_errors.hpp"
    #include "libdnf5/rpm/nevra.hpp"
    #include "libdnf5/rpm/rpm_signature.hpp"
    #include "libdnf5/transaction/transaction_item_action.hpp"
    #include "libdnf5/transaction/transaction_item_reason.hpp"
    #include "libdnf5/transaction/transaction_item_state.hpp"
    #include "libdnf5/transaction/transaction.hpp"
%}

%catches(
    libdnf5::base::TransactionError,

    libdnf5::comps::InvalidPackageType,

    libdnf5::module::NoModuleError,
    libdnf5::module::NoStreamError,
    libdnf5::module::EnabledStreamError,
    libdnf5::module::EnableMultipleStreamsError,
    libdnf5::module::ModuleConflictError,
    libdnf5::module::ModuleResolveError,
    libdnf5::module::ModuleError,

    libdnf5::module::InvalidModuleStatus,

    libdnf5::repo::FileDownloadError,
    libdnf5::repo::PackageDownloadError,
    libdnf5::repo::RepoCacheError,

    libdnf5::repo::RepoCacheonlyError,
    libdnf5::repo::RepoDownloadError,
    libdnf5::repo::RepoPgpError,
    libdnf5::repo::RepoRpmError,
    libdnf5::repo::RepoCompsError,
    libdnf5::repo::RepoIdAlreadyExistsError,
    libdnf5::repo::RepoError,

    libdnf5::rpm::NevraIncorrectInputError,
    libdnf5::rpm::SignatureCheckError,
    libdnf5::rpm::KeyImportError,

    libdnf5::transaction::InvalidTransactionItemAction,
    libdnf5::transaction::InvalidTransactionItemReason,
    libdnf5::transaction::InvalidTransactionItemState,
    libdnf5::transaction::InvalidTransactionItemType,
    libdnf5::transaction::InvalidTransactionState,

    libdnf5::InaccessibleConfigError,
    libdnf5::MissingConfigError,
    libdnf5::InvalidConfigError,

    libdnf5::ConfigParserSectionNotFoundError,
    libdnf5::ConfigParserOptionNotFoundError,
    libdnf5::ConfigParserError,

    libdnf5::OptionValueNotSetError,
    libdnf5::OptionValueNotAllowedError,
    libdnf5::OptionInvalidValueError,
    libdnf5::OptionError,

    libdnf5::OptionBindsOptionNotFoundError,
    libdnf5::OptionBindsOptionAlreadyExistsError,
    libdnf5::OptionBindsError,

    libdnf5::ReadOnlyVariableError,

    libdnf5::FileSystemError,
    libdnf5::SystemError,
    libdnf5::RuntimeError,
    libdnf5::Error,
    libdnf5::UserAssertionError,
    libdnf5::AssertionError

    std::out_of_range,
    std::system_error,
    std::runtime_error,
    std::exception
) rethrow_original(const libdnf5::common::ExceptionWrap &);

%inline %{
/// Rethrows the original exception.
void rethrow_original(const libdnf5::common::ExceptionWrap & ex) {
    ex.rethrow_original();
}
%}

%exception rethrow_if_nested {
    try {
        $action
    } catch (const std::exception &) {
        libdnf_exception_wrap_current()
        SWIG_fail;
    }
}

%inline %{
/// If a nested exception is contained, it is thrown in a new ExceptionWrap exception.
void rethrow_if_nested(const libdnf5::common::ExceptionWrap & ex) {
    ex.rethrow_if_nested_original();
}
%}

%exception rethrow_if_nested;
