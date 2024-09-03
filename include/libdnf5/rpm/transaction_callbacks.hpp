/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2.1 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/


#ifndef LIBDNF5_RPM_TRANSACTION_CALLBACKS_HPP
#define LIBDNF5_RPM_TRANSACTION_CALLBACKS_HPP

#include "libdnf5/defs.h"
#include "libdnf5/rpm/nevra.hpp"

#include <cstdint>

namespace libdnf5::base {

class TransactionPackage;

}  // namespace libdnf5::base

namespace libdnf5::rpm {


/// Class represents one item in transaction set.
using TransactionItem = base::TransactionPackage;


/// The base class for Transaction callbacks.
/// User implements Transaction callbacks by inheriting this class and overriding its methods.
///
/// Typical order in which the transaction callbacks are called is:
///
/// - before_begin
/// - verification phase: verify_start, verify_progress, verify_stop
/// - script_start, script_stop, script_error for pre transaction scriplets
/// - preparation phase: transaction_start, transaction_progress, transaction_stop
/// - install packages: elem_progress, install_start, install_progress, install_stop, with their scriptlets
/// - remove packages: elem_progress, uninstall_start, uninstall_progress, uninstall_stop, with their scriptlets
/// - script_start, script_stop, script_error for post transaction scriplets
/// - after_complete
///
class LIBDNF_API TransactionCallbacks {
public:
    /// Scriptlet type
    // TODO(jrohel): Are all scriptlets types present and correct?
    enum class ScriptType {
        UNKNOWN,
        PRE_INSTALL,            // "%pre"
        POST_INSTALL,           // "%post"
        PRE_UNINSTALL,          // "%preun"
        POST_UNINSTALL,         // "%postun"
        PRE_TRANSACTION,        // "%pretrans"
        POST_TRANSACTION,       // "%posttrans"
        TRIGGER_PRE_INSTALL,    // "%triggerprein"
        TRIGGER_INSTALL,        // "%triggerin"
        TRIGGER_UNINSTALL,      // "%triggerun"
        TRIGGER_POST_UNINSTALL  // "%triggerpostun"
    };

    /// @param type  scriptlet type
    /// @return  string representation of the scriptlet type
    static const char * script_type_to_string(ScriptType type) noexcept;

    explicit TransactionCallbacks();
    TransactionCallbacks(const TransactionCallbacks &) = delete;
    TransactionCallbacks(TransactionCallbacks &&) = delete;
    virtual ~TransactionCallbacks();

    TransactionCallbacks & operator=(const TransactionCallbacks &) = delete;
    TransactionCallbacks & operator=(TransactionCallbacks &&) = delete;

    /// Called right before the rpm transaction is run
    /// @param total Number of elements in the rpm transaction
    virtual void before_begin(uint64_t total);

    /// Called after the transaction run finished
    /// @param success Whether the rpm transaction was completed successfully
    virtual void after_complete(bool success);

    /// Report the package installation progress periodically.
    ///
    /// @param item The TransactionPackage class instance for the package currently being installed
    /// @param amount The portion of the package already installed
    /// @param total The disk space used by the package after installation
    virtual void install_progress(const TransactionItem & item, uint64_t amount, uint64_t total);

    /// Installation of a package has started
    ///
    /// @param item The TransactionPackage class instance for the package currently being installed
    /// @param total The disk space used by the package after installation
    virtual void install_start(const TransactionItem & item, uint64_t total);

    /// Installation of a package finished
    ///
    /// @param item The TransactionPackage class instance for the package currently being installed
    /// @param amount The portion of the package that has been installed
    /// @param total The disk space used by the package after installation
    virtual void install_stop(const TransactionItem & item, uint64_t amount, uint64_t total);

    /// Preparation of a package has started.
    ///
    /// @param amount Index of the package currently being prepared. Items are indexed starting from 0.
    /// @param total The total number of packages in the transaction
    virtual void transaction_progress(uint64_t amount, uint64_t total);

    /// Preparation phase has started.
    ///
    /// @param total The total number of packages in the transaction
    virtual void transaction_start(uint64_t total);

    /// Preparation phase finished.
    ///
    /// @param total The total number of packages in the transaction
    virtual void transaction_stop(uint64_t total);

    /// Report the package removal progress periodically.
    ///
    /// @param item The TransactionPackage class instance for the package currently being removed
    /// @param amount The portion of the package already uninstalled
    /// @param total The disk space freed by the package after removal
    virtual void uninstall_progress(const TransactionItem & item, uint64_t amount, uint64_t total);

    /// Removal of a package has started
    ///
    /// @param item The TransactionPackage class instance for the package currently being removed
    /// @param total The disk space freed by the package after removal
    virtual void uninstall_start(const TransactionItem & item, uint64_t total);

    /// Removal of a package finished
    ///
    /// @param item The TransactionPackage class instance for the package currently being removed
    /// @param amount The portion of the package already uninstalled
    /// @param total The disk space freed by the package after removal
    virtual void uninstall_stop(const TransactionItem & item, uint64_t amount, uint64_t total);

    /// Unpacking of the package failed.
    ///
    /// @param item The TransactionPackage class instance representing the package that failed to unpack
    virtual void unpack_error(const TransactionItem & item);

    /// cpio error during the package installation. Currently not used by librpm.
    ///
    /// @param item The TransactionPackage class instance representing the package that caused the error
    virtual void cpio_error(const TransactionItem & item);

    /// Execution of the rpm scriptlet finished with error
    ///
    /// @param item The TransactionPackage class instance for the package that owns the executed or triggered scriptlet. It can be `nullptr` if the scriptlet owner is not part of the transaction.
    /// @param nevra Nevra of the package that owns the executed or triggered scriptlet.
    /// @param type Type of the scriptlet
    /// @param return_code The return code of the scriptlet execution
    virtual void script_error(const TransactionItem * item, Nevra nevra, ScriptType type, uint64_t return_code);

    /// Execution of the rpm scriptlet has started
    ///
    /// @param item The TransactionPackage class instance for the package that owns the executed or triggered scriptlet. It can be `nullptr` if the scriptlet owner is not part of the transaction (e.g., a package installation triggered an update of the man database, owned by man-db package).
    /// @param nevra Nevra of the package that owns the executed or triggered scriptlet.
    /// @param type Type of the scriptlet
    virtual void script_start(const TransactionItem * item, Nevra nevra, ScriptType type);

    /// Execution of the rpm scriptlet finished without critical error
    ///
    /// @param item The TransactionPackage class instance for the package that owns the executed or triggered scriptlet. It can be `nullptr` if the scriptlet owner is not part of the transaction.
    /// @param nevra Nevra of the package that owns the executed or triggered scriptlet.
    /// @param type Type of the scriptlet
    /// @param return_code The return code of the scriptlet execution
    virtual void script_stop(const TransactionItem * item, Nevra nevra, ScriptType type, uint64_t return_code);

    /// The installation/removal process for the item has started
    ///
    /// @param amount Index of the package currently being processed. Items are indexed starting from 0.
    /// @param total The total number of packages in the transaction
    virtual void elem_progress(const TransactionItem & item, uint64_t amount, uint64_t total);

    /// Verification of a package files has started.
    ///
    /// @param amount Index of the package currently being verified. Items are indexed starting from 0.
    /// @param total The total number of packages to verify
    virtual void verify_progress(uint64_t amount, uint64_t total);

    /// Packages files verification phase has started. In this phase the signature of packages are verified.
    ///
    /// @param total The total number of packages to verify
    virtual void verify_start(uint64_t total);

    /// Packages files verification phase finished.
    ///
    /// @param total The total number of packages to verify
    virtual void verify_stop(uint64_t total);
};


}  // namespace libdnf5::rpm

#endif  // LIBDNF5_RPM_TRANSACTION_CALLBACKS_HPP
