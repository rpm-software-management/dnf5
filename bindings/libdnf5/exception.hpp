#ifndef _BINDINGS_LIBDNF5_EXCEPTINON_HPP_
#define _BINDINGS_LIBDNF5_EXCEPTINON_HPP_

#include "libdnf5/base/transaction_errors.hpp"
#include "libdnf5/common/exception.hpp"
#include "libdnf5/comps/group/package_errors.hpp"
#include "libdnf5/conf/config_parser_errors.hpp"
#include "libdnf5/conf/option_binds_errors.hpp"
#include "libdnf5/conf/option_errors.hpp"
#include "libdnf5/conf/option_path_errors.hpp"
#include "libdnf5/conf/vars_errors.hpp"
//#include "libdnf5/module/module_errors.hpp"
#include "libdnf5/repo/file_downloader_errors.hpp"
#include "libdnf5/repo/package_downloader_errors.hpp"
#include "libdnf5/repo/repo_cache_errors.hpp"
#include "libdnf5/repo/repo_errors.hpp"
#include "libdnf5/rpm/nevra_errors.hpp"
#include "libdnf5/rpm/rpm_signature_errors.hpp"
#include "libdnf5/transaction/transaction_errors.hpp"
#include "libdnf5/transaction/transaction_item_errors.hpp"
#include "libdnf5/utils/bgettext/bgettext-mark.h"


/// Class for wrapping exceptions not defined in libdnf5. For example, std::* exceptions.
class NonLibdnf5Exception : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};


#if defined(SWIGPYTHON)

#define swig_exception(e, Type)                     \
    auto * ex = new Type(e);                        \
    auto ex_swig_type = SWIG_TypeQuery(#Type " *"); \
    SWIG_Python_Raise(SWIG_NewPointerObj(ex, ex_swig_type, SWIG_POINTER_OWN), #Type, ex_swig_type);

#elif defined(SWIGPERL)

#define swig_exception(e, Type)                     \
    auto * ex = new Type(e);                        \
    auto ex_swig_type = SWIG_TypeQuery(#Type " *"); \
    sv_setsv(get_sv("@", GV_ADD), SWIG_NewPointerObj(ex, ex_swig_type, SWIG_POINTER_OWN));

#elif defined(SWIGRUBY)

#define swig_exception(e, Type)                     \
    auto * ex = new Type(e);                        \
    auto ex_swig_type = SWIG_TypeQuery(#Type " *"); \
    rb_exc_raise(SWIG_Ruby_ExceptionType(ex_swig_type, SWIG_NewPointerObj(ex, ex_swig_type, SWIG_POINTER_OWN)));

#endif

#define swig_if_exception(e, Type)                                                  \
    if (auto * oe = dynamic_cast<const Type *>(&e)) {                               \
        if (auto * ne = dynamic_cast<const libdnf5::NestedException<Type> *>(&e)) { \
            swig_exception(*ne, libdnf5::NestedException<Type>);                    \
        } else {                                                                    \
            swig_exception(*oe, Type);                                              \
        }                                                                           \
        break;                                                                      \
    }

static void create_swig_exception(const std::exception & e) {
    do {
        swig_if_exception(e, libdnf5::base::TransactionError);

        swig_if_exception(e, libdnf5::comps::InvalidPackageType);

        swig_if_exception(e, libdnf5::repo::FileDownloadError);
        swig_if_exception(e, libdnf5::repo::PackageDownloadError);
        swig_if_exception(e, libdnf5::repo::RepoCacheError);

        swig_if_exception(e, libdnf5::repo::RepoCacheonlyError);
        swig_if_exception(e, libdnf5::repo::RepoDownloadError);
        swig_if_exception(e, libdnf5::repo::RepoPgpError);
        swig_if_exception(e, libdnf5::repo::RepoRpmError);
        swig_if_exception(e, libdnf5::repo::RepoCompsError);
        swig_if_exception(e, libdnf5::repo::RepoIdAlreadyExistsError);
        swig_if_exception(e, libdnf5::repo::RepoError);

        swig_if_exception(e, libdnf5::rpm::NevraIncorrectInputError);
        swig_if_exception(e, libdnf5::rpm::SignatureCheckError);
        swig_if_exception(e, libdnf5::rpm::KeyImportError);

        swig_if_exception(e, libdnf5::transaction::InvalidTransactionItemAction);
        swig_if_exception(e, libdnf5::transaction::InvalidTransactionItemReason);
        swig_if_exception(e, libdnf5::transaction::InvalidTransactionItemState);
        swig_if_exception(e, libdnf5::transaction::InvalidTransactionItemType);
        swig_if_exception(e, libdnf5::transaction::InvalidTransactionState);

        swig_if_exception(e, libdnf5::InaccessibleConfigError);
        swig_if_exception(e, libdnf5::MissingConfigError);
        swig_if_exception(e, libdnf5::InvalidConfigError);

        swig_if_exception(e, libdnf5::ConfigParserSectionNotFoundError);
        swig_if_exception(e, libdnf5::ConfigParserOptionNotFoundError);
        swig_if_exception(e, libdnf5::ConfigParserError);

        swig_if_exception(e, libdnf5::OptionValueNotSetError);
        swig_if_exception(e, libdnf5::OptionValueNotAllowedError);
        swig_if_exception(e, libdnf5::OptionInvalidValueError);
        swig_if_exception(e, libdnf5::OptionError);

        swig_if_exception(e, libdnf5::OptionBindsOptionNotFoundError);
        swig_if_exception(e, libdnf5::OptionBindsOptionAlreadyExistsError);
        swig_if_exception(e, libdnf5::OptionBindsError);

        swig_if_exception(e, libdnf5::ReadOnlyVariableError);

        swig_if_exception(e, libdnf5::FileSystemError);
        swig_if_exception(e, libdnf5::SystemError);
        swig_if_exception(e, libdnf5::RuntimeError);
        swig_if_exception(e, libdnf5::Error);
        swig_if_exception(e, libdnf5::UserAssertionError);
        swig_if_exception(e, libdnf5::AssertionError);

        // We caught an exception not defined in libdnf5. We transform it into a libdnf5::NonLibdnf5Exception.
        if (auto * oe = dynamic_cast<const std::exception *>(&e)) {
            try {
                std::rethrow_if_nested(e);
            } catch (const std::exception &) {
                swig_exception(
                    libdnf5::NestedException<NonLibdnf5Exception>(NonLibdnf5Exception(oe->what())),
                    libdnf5::NestedException<NonLibdnf5Exception>);
                break;
            }
            swig_exception(NonLibdnf5Exception(oe->what()), NonLibdnf5Exception);
            break;
        }
    } while (false);
}

#undef swig_if_exception
#undef swig_exception

#endif
