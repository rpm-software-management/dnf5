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


#ifndef LIBDNF5_RPM_TRANSACTION_HPP
#define LIBDNF5_RPM_TRANSACTION_HPP

#include "rpm_log_guard.hpp"

#include "libdnf5/base/base_weak.hpp"
#include "libdnf5/base/transaction_package.hpp"
#include "libdnf5/common/exception.hpp"
#include "libdnf5/rpm/package.hpp"
#include "libdnf5/rpm/transaction_callbacks.hpp"

#include <rpm/header.h>
#include <rpm/rpmprob.h>
#include <rpm/rpmps.h>
#include <rpm/rpmts.h>

#include <memory>

// Required for building with fmt >= 10
// See: https://github.com/fmtlib/fmt/blob/10.0.0/ChangeLog.rst?plain=1#L68
inline uint32_t format_as(rpmCallbackType type) {
    return static_cast<uint32_t>(type);
}


namespace libdnf5::rpm {

class RpmProblemSet;


/// Class represents one item in transaction set.
using TransactionItem = base::TransactionPackage;


/// Class for access RPM header
class RpmHeader {
public:
    RpmHeader(const RpmHeader & src) : header(headerLink(src.header)) {}

    RpmHeader(RpmHeader && src) : header(src.header) { src.header = nullptr; }

    ~RpmHeader() { headerFree(header); }

    RpmHeader & operator=(const RpmHeader & src);
    RpmHeader & operator=(RpmHeader && src);

    std::string get_name() const { return headerGetString(header, RPMTAG_NAME); }

    uint64_t get_epoch() const noexcept { return headerGetNumber(header, RPMTAG_EPOCH); }

    std::string get_version() const { return headerGetString(header, RPMTAG_VERSION); }

    std::string get_release() const { return headerGetString(header, RPMTAG_RELEASE); }

    std::string get_arch() const { return headerGetString(header, RPMTAG_ARCH); }

    std::string get_evr() const;

    std::string get_nevra() const;

    std::string get_full_nevra() const;

    unsigned int get_rpmdbid() const noexcept { return headerGetInstance(header); }

private:
    friend class Transaction;
    explicit RpmHeader(Header hdr) : header(headerLink(hdr)) {}
    Header header;
};


class RpmProblem {
public:
    bool operator==(RpmProblem & other) const noexcept { return rpmProblemCompare(problem, other.problem) == 0; }

    /// Return package NEVR
    std::string get_pkg_nevr() const { return rpmProblemGetPkgNEVR(problem); }

    /// Return related (e.g. through a dependency) package NEVR
    std::string get_alt_nevr() const { return rpmProblemGetAltNEVR(problem); }

    /// Return type of problem (dependency, diskpace etc)
    rpmProblemType get_type() const { return rpmProblemGetType(problem); }

    /// Return pointer to transaction item associated to the problem or nullptr.
    const TransactionItem * get_transaction_item() const noexcept {
        return static_cast<const TransactionItem *>(rpmProblemGetKey(problem));
    }

    /// Return a generic data string from a problem
    std::string get_generic_string() const { return rpmProblemGetStr(problem); }

    /// Return disk requirement (needed disk space / number of inodes)
    /// depending on problem type. On problem types other than DISKSPACE
    /// and DISKNODES return value is undefined.
    uint64_t get_disk_need() const noexcept { return rpmProblemGetDiskNeed(problem); }

    /// Return formatted string representation of a problem.
    std::string to_string() const { return rpmProblemString(problem); }

private:
    friend RpmProblemSet;
    explicit RpmProblem(rpmProblem problem) : problem(problem) {}
    rpmProblem problem;
};


class RpmProblemSet {
public:
    class Iterator {
    public:
        // iterator traits
        using difference_type = int;
        using value_type = RpmProblem;
        using pointer = const RpmProblem *;
        using reference = const RpmProblem &;
        using iterator_category = std::forward_iterator_tag;

        Iterator() = default;
        Iterator(const Iterator & other) = delete;
        ~Iterator() { rpmpsFreeIterator(iter); }

        Iterator & operator=(const Iterator & other) = delete;

        Iterator & operator++();

        // TODO(jrohel): postfix operator ++, Bad iterator support in the librpm
        // iterator operator++(int);

        bool operator!=(Iterator & other) const { return rpmpsGetProblem(iter) != rpmpsGetProblem(other.iter); }

        RpmProblem operator*() { return RpmProblem(rpmpsGetProblem(iter)); }

    private:
        friend RpmProblemSet;
        explicit Iterator(rpmps problem_set) : iter(rpmpsInitIterator(problem_set)) { rpmpsiNext(iter); }

        void free() {
            rpmpsFreeIterator(iter);
            iter = nullptr;
        }

        rpmpsi iter{nullptr};
    };

    Iterator begin() { return Iterator(problem_set); }

    Iterator end() { return Iterator(); }

    RpmProblemSet(const RpmProblemSet & lhs) : problem_set(rpmpsLink(lhs.problem_set)) {}

    ~RpmProblemSet() { rpmpsFree(problem_set); }

    /// Return number of problems in set.
    /// @return  number of problems
    int size() noexcept { return rpmpsNumProblems(problem_set); }

    bool empty() noexcept { return size() == 0; }

private:
    friend class Transaction;
    explicit RpmProblemSet(rpmps problem_set) : problem_set(problem_set) {}
    rpmps problem_set;
};


class TransactionError : public Error {
public:
    using Error::Error;
    const char * get_domain_name() const noexcept override { return "libdnf5::rpm"; }
    const char * get_name() const noexcept override { return "TransactionError"; }
};


class Transaction {
public:
    explicit Transaction(Base & base);
    explicit Transaction(const BaseWeakPtr & base);
    Transaction(const Transaction &) = delete;
    Transaction(Transaction &&) = delete;
    ~Transaction();

    Transaction & operator=(const Transaction &) = delete;
    Transaction & operator=(Transaction &&) = delete;

    /// Set transaction root directory, i.e. path to chroot(2).
    /// @param root_dir  new transaction root directory (or NULL)
    /// @return  0 on success, -1 on error (invalid root directory)
    void set_root_dir(const char * root_dir);

    /// Get transaction root directory, i.e. path to chroot(2).
    /// @return  transaction root directory
    const char * get_root_dir() const { return rpmtsRootDir(ts); }

    /// Retrieve color bits of transaction set.
    /// @return  color bits
    rpm_color_t get_color() const { return rpmtsColor(ts); }

    /// Set color bits of transaction set.
    /// @param color  new color bits
    /// @return  previous color bits
    rpm_color_t set_color(rpm_color_t color) { return rpmtsSetColor(ts, color); }

    /// Retrieve preferred file color
    /// @return  color bits
    rpm_color_t get_pref_color() const { return rpmtsPrefColor(ts); }

    /// Set preferred file color
    /// @param color  new color bits
    /// @return  previous color bits
    rpm_color_t set_pref_color(rpm_color_t color) { return rpmtsSetPrefColor(ts, color); }

    /// Get transaction flags, i.e. bits that control rpmtsRun().
    /// @return  transaction flags
    rpmtransFlags get_flags() const { return rpmtsFlags(ts); }

    /// Set transaction flags, i.e. bits that control rpmtsRun().
    /// @param flags  new transaction flags
    /// @return  previous transaction flags
    rpmtransFlags set_flags(rpmtransFlags flags) { return rpmtsSetFlags(ts, flags); }

    /// Get verify signatures flag(s).
    /// @return  verify signatures flags
    rpmVSFlags get_signature_verify_flags() const { return rpmtsVSFlags(ts); }

    /// Set verify signatures flag(s).
    /// @param verify_flags  new verify signatures flags
    /// @return         previous value
    rpmVSFlags set_signature_verify_flags(rpmVSFlags verify_flags) { return rpmtsSetVSFlags(ts, verify_flags); }

    /// Get package verify flag(s).
    /// @return  verify signatures flags
    rpmVSFlags get_pkg_verify_flags() const { return rpmtsVfyFlags(ts); }

    /// Set package verify flag(s).
    /// @param verify_flags  new package verify flags
    /// @return  old package verify flags
    rpmVSFlags set_pkg_verify_flags(rpmVSFlags verify_flags) { return rpmtsSetVfyFlags(ts, verify_flags); }

    /// Get enforced package verify level
    /// @return  package verify level
    int get_pkg_verify_level() const { return rpmtsVfyLevel(ts); }

    /// Set enforced package verify level
    /// @param verify_level  new package verify level
    /// @return  old package verify level
    int set_pkg_verify_level(int verify_level) { return rpmtsSetVfyLevel(ts, verify_level); }

    /// Retrieve rpm database cookie.
    /// Useful for eg. determining cache validity.
    /// @return rpm database cookie
    std::string get_db_cookie() const;

    /// Get transaction id, i.e. transaction time stamp.
    /// @return  transaction id
    rpm_tid_t get_id() const { return rpmtsGetTid(ts); }

    /// Set transaction callbacks.
    /// @param callbacks a `unique_ptr` containing the callbacks object
    void set_callbacks(std::unique_ptr<TransactionCallbacks> && callbacks) {
        callbacks_holder.callbacks = std::move(callbacks);
    }

    /// Fill the RPM transaction from base::Transaction.
    /// @param transcation The base::Transaction object.
    void fill(const base::Transaction & transaction);

    /// Perform a dependency check on the transaction set.
    /// After headers have been added to a transaction set,
    /// a dependency check can be performed to make sure that all package dependencies are satisfied.
    /// Any found problems can be examined by retrieving the problem set with rpmtsProblems().
    /// @return  true on dependencies are ok
    bool check() { return rpmtsCheck(ts) == 0; }

    /// Process all package elements in a transaction set.
    /// Before calling run() be sure to have:
    ///
    ///    - setup the rpm root dir via set_root_dir().
    ///    - setup the rpm notify callback via register_cb().
    ///    - setup the rpm transaction flags via set_flags().
    ///
    /// Additionally, though not required you may want to:
    ///
    ///    - setup the rpm verify signature flags via set_signature_verify_flags().
    ///
    /// @return		0 on success, -1 on error, >0 with newProbs set
    int run();

    /// Get current transaction set problems.
    RpmProblemSet get_problems() { return RpmProblemSet(rpmtsProblems(ts)); }

    /// Set transaction script file descriptor, i.e. stdout/stderr on scriptlet execution.
    /// The file descriptor is copied using dup().
    /// @param fd  new script file descriptor (or -1)
    void set_script_out_fd(int fd);

    /// Set transaction script file, i.e. stdout/stderr on scriptlet execution.
    /// The set_script_fd() can be used to pass file handle.
    /// @param file_path  new file path
    void set_script_out_file(const std::string & file_path);

    /// @return A `Base` object to which the transaction belongs.
    /// @since 5.0
    BaseWeakPtr get_base() const;

private:
    struct CallbacksHolder {
        std::unique_ptr<TransactionCallbacks> callbacks;
        Transaction * transaction;
    };

    BaseWeakPtr base;
    rpmts ts;
    FD_t script_fd{nullptr};
    CallbacksHolder callbacks_holder{nullptr, this};
    FD_t fd_in_cb{nullptr};  // file descriptor used by transaction in callback (install/reinstall package)

    TransactionItem * last_added_item{nullptr};  // item added by last install/reinstall/erase/...
    bool last_item_added_ts_element{false};      // Did the last item add the element ts?

    std::map<unsigned int, rpmte> implicit_ts_elements;  // elements added to the librpm transaction by librpm itself
    bool downgrade_requested{false};
    std::vector<TransactionItem> transaction_items;

    /// The outputs of the last executed scriptlet is stored here
    std::string last_script_output;
    std::mutex last_script_output_mutex;

    RpmLogGuard rpm_log_guard;


    /// Return header from package.
    /// @param path  file path
    /// @retval hdrp  header (NULL on failure)
    /// @return  package header
    Header read_pkg_header(const std::string & file_path) const;

    /// Get header of package at offset in the rpmdbi database
    Header get_header(unsigned int rec_offset);

    /// Add package to be installed to transaction set.
    /// The transaction set is checked for duplicate package names.
    /// If found, the package with the "newest" EVR will be replaced.
    /// @param item  item to be installed
    void install(TransactionItem & item) {
        install_up_down(item, libdnf5::transaction::TransactionItemAction::INSTALL);
    }

    /// Add package to be upgraded to transaction set.
    /// The transaction set is checked for duplicate package names.
    /// If found, the package with the "newest" EVR will be replaced.
    /// @param item  item to be upgraded
    void upgrade(TransactionItem & item) {
        install_up_down(item, libdnf5::transaction::TransactionItemAction::UPGRADE);
    }

    /// Add package to be upgraded to transaction set.
    /// The transaction set is checked for duplicate package names.
    /// If found, the package with the "newest" EVR will be replaced.
    /// @param item  item to be upgraded
    void downgrade(TransactionItem & item) {
        install_up_down(item, libdnf5::transaction::TransactionItemAction::DOWNGRADE);
    }

    /// Add package to be reinstalled to transaction set.
    /// @param item  item to be reinstalled
    void reinstall(TransactionItem & item);

    /// Add package to be erased to transaction set.
    /// @param item  item to be erased
    void erase(TransactionItem & item);

    /// Add package to be installed to transaction set.
    /// The transaction set is checked for duplicate package names.
    /// If found, the package with the "newest" EVR will be replaced.
    /// @param item  item to be erased
    /// @param action  one of TransactionItemAction::UPGRADE,
    ///     TransactionItemAction::DOWNGRADE, TransactionItemAction::INSTALL
    void install_up_down(TransactionItem & item, libdnf5::transaction::TransactionItemAction action);

    static Nevra trans_element_to_nevra(rpmte te);

    /// Function triggered by rpmtsNotifyChange()
    ///
    /// On explicit install/erase add events, "other" is NULL, on implicit
    /// add events (erasures due to obsolete/upgrade/reinstall, replaced by newer)
    /// it points to the replacing package.
    ///
    /// @param event  Change event (see rpmtsEvent enum)
    /// @param te  Transaction element
    /// @param other  Related transaction element (or NULL)
    /// @param data  Application private data from rpmtsSetChangeCallback()
    static int ts_change_callback(int event, rpmte te, rpmte other, void * data);

    static TransactionCallbacks::ScriptType rpm_tag_to_script_type(rpmTag_e tag) noexcept;

    /// Function triggered by rpmtsNotify()
    ///
    /// @param te  related transaction element or NULL
    /// @param what  kind of notification
    /// @param amount  number of bytes/packages already processed or tag of the scriptlet involved
    ///                or 0 or some other number
    /// @param total  total number of bytes/packages to be processed or return code of the scriptlet or 0
    /// @param key  result of rpmteKey() of related rpmte or 0
    /// @param data  user data as passed to rpmtsSetNotifyCallback()
    static void * ts_callback(
        const void * te,
        const rpmCallbackType what,
        const rpm_loff_t amount,
        const rpm_loff_t total,
        [[maybe_unused]] const void * pkg_key,
        rpmCallbackData data);

    /// Reads the output of scriptlets from the file descriptor and processes them.
    static void process_scriptlets_output(int fd, Transaction * transaction);
};

}  // namespace libdnf5::rpm

#endif  // LIBDNF5_RPM_TRANSACTION_HPP
