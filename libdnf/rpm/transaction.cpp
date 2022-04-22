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


#include "libdnf/base/transaction.hpp"

#include "package_set_impl.hpp"
#include "rpm_log_guard.hpp"
#include "utils/bgettext/bgettext-lib.h"

#include "libdnf/common/exception.hpp"
#include "libdnf/rpm/transaction.hpp"
#include "libdnf/transaction/transaction_item_action.hpp"
#include "libdnf/utils/to_underlying.hpp"

#include <fcntl.h>
#include <fmt/format.h>
#include <rpm/rpmbuild.h>
#include <rpm/rpmdb.h>
#include <rpm/rpmlib.h>
#include <rpm/rpmpgp.h>
#include <rpm/rpmtag.h>
#include <rpm/rpmts.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <filesystem>
#include <map>
#include <type_traits>

namespace libdnf::rpm {

RpmHeader::RpmHeader(void * hdr) : header(headerLink(static_cast<Header>(hdr))) {}

RpmHeader::RpmHeader(const RpmHeader & src) : header(headerLink(static_cast<Header>(src.header))) {}

RpmHeader::RpmHeader(RpmHeader && src) : header(src.header) {
    src.header = nullptr;
}

RpmHeader::~RpmHeader() {
    headerFree(static_cast<Header>(header));
}

RpmHeader & RpmHeader::operator=(const RpmHeader & src) {
    if (&src != this) {
        headerFree(static_cast<Header>(header));
        header = headerLink(static_cast<Header>(src.header));
    }
    return *this;
}

RpmHeader & RpmHeader::operator=(RpmHeader && src) {
    if (&src != this) {
        headerFree(static_cast<Header>(header));
        header = src.header;
        src.header = nullptr;
    }
    return *this;
}

std::string RpmHeader::get_name() const {
    return headerGetString(static_cast<Header>(header), RPMTAG_NAME);
}

uint64_t RpmHeader::get_epoch() const noexcept {
    return headerGetNumber(static_cast<Header>(header), RPMTAG_EPOCH);
}

std::string RpmHeader::get_version() const {
    return headerGetString(static_cast<Header>(header), RPMTAG_VERSION);
}

std::string RpmHeader::get_release() const {
    return headerGetString(static_cast<Header>(header), RPMTAG_RELEASE);
}

std::string RpmHeader::get_arch() const {
    return headerGetString(static_cast<Header>(header), RPMTAG_ARCH);
}

std::string RpmHeader::get_nevra() const {
    auto * tmp = headerGetAsString(static_cast<Header>(header), RPMTAG_NEVRA);
    std::string ret = tmp;
    rfree(tmp);
    return ret;
}

std::string RpmHeader::get_full_nevra() const {
    std::stringstream out;
    out << get_name() << '-' << get_epoch() << ':' << get_version() << '-' << get_release() << '.' << get_arch();
    return out.str();
}

unsigned int RpmHeader::get_rpmdbid() const noexcept {
    return headerGetInstance(static_cast<Header>(header));
}

class RpmProblem::Impl {
public:
    explicit Impl(rpmProblem problem) : problem(problem) {}

private:
    friend RpmProblem;
    rpmProblem problem;
};


RpmProblem::RpmProblem(std::unique_ptr<Impl> && p_impl) : p_impl(std::move(p_impl)) {}

RpmProblem::~RpmProblem() = default;

bool RpmProblem::operator==(RpmProblem & other) const noexcept {
    return rpmProblemCompare(p_impl->problem, other.p_impl->problem) == 0;
}

bool RpmProblem::operator!=(RpmProblem & other) const noexcept {
    return rpmProblemCompare(p_impl->problem, other.p_impl->problem) != 0;
}

std::string RpmProblem::get_pkg_nevr() const {
    return rpmProblemGetPkgNEVR(p_impl->problem);
}

std::string RpmProblem::get_alt_nevr() const {
    return rpmProblemGetAltNEVR(p_impl->problem);
}

RpmProblem::Type RpmProblem::get_type() const {
    return static_cast<Type>(rpmProblemGetType(p_impl->problem));
}

const TransactionItem * RpmProblem::get_transaction_item() const {
    return static_cast<const TransactionItem *>(rpmProblemGetKey(p_impl->problem));
}

std::string RpmProblem::get_generic_string() const {
    return rpmProblemGetStr(p_impl->problem);
}

uint64_t RpmProblem::get_disk_need() const {
    return rpmProblemGetDiskNeed(p_impl->problem);
}

std::string RpmProblem::to_string() const {
    return rpmProblemString(p_impl->problem);
}


class RpmProblemSet::Impl {
public:
    explicit Impl(rpmps problem_set) : problem_set(problem_set) {}

    ~Impl() { rpmpsFree(problem_set); }

    Impl(const Impl &) = delete;
    Impl(Impl &&) = delete;

    Impl & operator=(const Impl &) = delete;
    Impl & operator=(Impl &&) = delete;

private:
    friend RpmProblemSet;
    rpmps problem_set;
};

RpmProblemSet::RpmProblemSet(std::unique_ptr<Impl> && p_impl) : p_impl(std::move(p_impl)) {}

RpmProblemSet::~RpmProblemSet() = default;

class RpmProblemSet::Iterator::Impl {
public:
    Impl() { iter = nullptr; }
    explicit Impl(rpmps problem_set) { iter = rpmpsInitIterator(problem_set); }

    Impl(const Impl &) = delete;
    Impl(Impl &&) = delete;

    Impl & operator=(const Impl &) = delete;
    Impl & operator=(Impl &&) = delete;

    // TODO(jrohel): Missing support in librpm.
    // explicit Impl(rpmpsi iter) : iter(iter) {}

    ~Impl() { rpmpsFreeIterator(iter); }

    Impl & next() {
        if (!rpmpsiNext(iter)) {
            free();
        }
        return *this;
    }

    RpmProblem operator*() const {
        auto * problem = rpmpsGetProblem(iter);
        auto rpm_problem_impl = std::make_unique<RpmProblem::Impl>(problem);
        return RpmProblem(std::move(rpm_problem_impl));
    }

    void free() {
        rpmpsFreeIterator(iter);
        iter = nullptr;
    }

private:
    friend RpmProblemSet;
    rpmpsi iter;
};

RpmProblemSet::Iterator::Iterator() = default;

RpmProblemSet::Iterator::Iterator(std::unique_ptr<Impl> && p_impl) : p_impl(std::move(p_impl)) {}

RpmProblemSet::Iterator::~Iterator() = default;

RpmProblemSet::Iterator RpmProblemSet::begin() {
    auto iter_impl = std::make_unique<Iterator::Impl>(p_impl->problem_set);
    return Iterator(std::move(iter_impl));
}

RpmProblemSet::Iterator RpmProblemSet::end() {
    return Iterator();
}

RpmProblemSet::Iterator & RpmProblemSet::Iterator::operator++() {
    p_impl->next();
    return *this;
};

bool RpmProblemSet::Iterator::operator==(Iterator & other) const {
    return rpmpsGetProblem(p_impl->iter) == rpmpsGetProblem(other.p_impl->iter);
}

RpmProblem RpmProblemSet::Iterator::operator*() {
    return **p_impl;
}

int RpmProblemSet::size() noexcept {
    return rpmpsNumProblems(p_impl->problem_set);
}

class Transaction::Impl {
public:
    explicit Impl(const BaseWeakPtr & base);
    Impl(const BaseWeakPtr & base, rpmVSFlags vsflags);
    Impl(const Impl &) = delete;
    Impl(Impl &&) = delete;
    ~Impl();

    Impl & operator=(const Impl &) = delete;
    Impl & operator=(Impl &&) = delete;

    /// Set transaction script file handle, i.e. stdout/stderr on scriptlet execution
    /// @param script_fd  new script file handle (or NULL)
    void set_script_fd(FD_t script_fd) noexcept {
        rpmtsSetScriptFd(ts, script_fd);
        if (this->script_fd) {
            Fclose(this->script_fd);
        }
        this->script_fd = script_fd;
    }

    /// Set transaction root directory, i.e. path to chroot(2).
    /// @param root_dir  new transaction root directory (or NULL)
    /// @return  0 on success, -1 on error (invalid root directory)
    void set_root_dir(const char * root_dir) {
        auto rc = rpmtsSetRootDir(ts, root_dir);
        if (rc != 0) {
            //TODO(jrohel): Why? Librpm does not provide this information.
            throw TransactionError(M_("Cannot set root directory \"{}\""), std::string(root_dir));
        }
    }

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
    std::string get_db_cookie() const {
        std::string rpmdb_cookie;

        // Open rpm database if it is not already open
        if (!rpmtsGetRdb(ts)) {
            auto rc = rpmtsOpenDB(ts, rpmtsGetDBMode(ts));
            if (rc != 0) {
                throw TransactionError(M_("Error %i opening rpm database"), rc);
            }
        }

        std::unique_ptr<char, decltype(free) *> rpmdb_cookie_uptr{rpmdbCookie(rpmtsGetRdb(ts)), free};
        if (rpmdb_cookie_uptr) {
            rpmdb_cookie = rpmdb_cookie_uptr.get();
        }
        if (rpmdb_cookie.empty()) {
            throw TransactionError(M_("The rpmdbCookie() function did not return cookie of rpm database."));
        }

        return rpmdb_cookie;
    }

    /// Get transaction id, i.e. transaction time stamp.
    /// @return  transaction id
    rpm_tid_t get_id() const { return rpmtsGetTid(ts); }

    /// Return header from package.
    /// @param path  file path
    /// @retval hdrp  header (NULL on failure)
    /// @return  package header
    Header read_pkg_header(const std::string & file_path) const {
        FD_t fd = Fopen(file_path.c_str(), "r.ufdio");

        if (!fd) {
            throw TransactionError(
                M_("Failed to read package header, cannot open file \"{}\": {}"),
                file_path,
                std::string(Fstrerror(fd)));
        }

        const char * descr = file_path.c_str();
        Header h{};  // Initialization of h is not needed. It is output argument of rpmReadPackageFile().
        rpmRC rpmrc = rpmReadPackageFile(ts, fd, descr, &h);
        Fclose(fd);

        switch (rpmrc) {
            case RPMRC_NOTTRUSTED:
            case RPMRC_NOKEY:
            case RPMRC_OK:
                break;
            case RPMRC_NOTFOUND:
            case RPMRC_FAIL:
            default:
                h = headerFree(h);
                throw TransactionError(M_("Failed to read package header from file \"{}\""), file_path);
                break;
        }

        return h;
    }

    /// Add package to be installed to transaction set.
    /// The transaction set is checked for duplicate package names.
    /// If found, the package with the "newest" EVR will be replaced.
    /// @param item  item to be installed
    void install(TransactionItem & item) { install_up_down(item, libdnf::transaction::TransactionItemAction::INSTALL); }

    /// Add package to be upgraded to transaction set.
    /// The transaction set is checked for duplicate package names.
    /// If found, the package with the "newest" EVR will be replaced.
    /// @param item  item to be upgraded
    void upgrade(TransactionItem & item) { install_up_down(item, libdnf::transaction::TransactionItemAction::UPGRADE); }

    /// Add package to be upgraded to transaction set.
    /// The transaction set is checked for duplicate package names.
    /// If found, the package with the "newest" EVR will be replaced.
    /// @param item  item to be upgraded
    void downgrade(TransactionItem & item) {
        install_up_down(item, libdnf::transaction::TransactionItemAction::DOWNGRADE);
    }

    /// Add package to be reinstalled to transaction set.
    /// @param item  item to be reinstalled
    void reinstall(TransactionItem & item) {
        auto file_path = item.get_package().get_package_path();
        auto * header = read_pkg_header(file_path);
        last_added_item = &item;
        last_item_added_ts_element = false;
        auto rc = rpmtsAddReinstallElement(ts, header, &item);
        headerFree(header);
        if (rc != 0) {
            //TODO(jrohel): Why? Librpm does not provide this information.
            throw TransactionError(M_("Cannot reinstall package \"{}\""), item.get_package().get_full_nevra());
        }
        libdnf_assert(
            last_item_added_ts_element,
            "librpm has ignored explicit request to reinstall package \"{}\"",
            item.get_package().get_full_nevra());
    }

    /// Add package to be erased to transaction set.
    /// @param item  item to be erased
    void erase(TransactionItem & item) {
        auto rpmdb_id = static_cast<unsigned int>(item.get_package().get_rpmdbid());
        auto * header = get_header(rpmdb_id);
        int unused = -1;
        last_added_item = &item;
        last_item_added_ts_element = false;
        int rc = rpmtsAddEraseElement(ts, header, unused);
        headerFree(header);
        if (rc != 0) {
            //TODO(jrohel): Why? Librpm does not provide this information.
            throw TransactionError(M_("Cannot remove package \"{}\""), item.get_package().get_full_nevra());
        }
        if (!last_item_added_ts_element) {
            auto it = implicit_ts_elements.find(rpmdb_id);
            libdnf_assert(
                it != implicit_ts_elements.end(),
                "librpm has ignored explicit request to remove package \"{}\"",
                item.get_package().get_full_nevra());
            auto * te = it->second;
            rpmteSetUserdata(te, &item);
            implicit_ts_elements.erase(it);
        }
    }

    /// Fill the RPM transaction from transaction packages.
    /// @param transcation_packages The transaction packages to add.
    void fill(std::vector<base::TransactionPackage> && transaction_packages) {
        transaction_items = std::move(transaction_packages);
        for (auto & tspkg : transaction_items) {
            switch (tspkg.get_action()) {
                case libdnf::transaction::TransactionItemAction::INSTALL:
                    install(tspkg);
                    break;
                case libdnf::transaction::TransactionItemAction::REINSTALL:
                    reinstall(tspkg);
                    break;
                case libdnf::transaction::TransactionItemAction::UPGRADE:
                    upgrade(tspkg);
                    break;
                case libdnf::transaction::TransactionItemAction::DOWNGRADE:
                    downgrade(tspkg);
                    break;
                case libdnf::transaction::TransactionItemAction::REMOVE:
                case libdnf::transaction::TransactionItemAction::REPLACED:
                    erase(tspkg);
                    break;
                default:;  // TODO(lukash) handle the other cases
            }
        }
    }

    /// Perform dependency resolution on the transaction set.
    /// Any problems found by rpmtsCheck() can be examined by retrieving the problem set with rpmtsProblems(),
    /// success here only means that the resolution was successfully attempted for all packages in the set.
    /// @return  true on dependencies are ok
    bool check() noexcept { return rpmtsCheck(ts) == 0; }

    /// Return current transaction set problems.
    /// @return  current problem set (or NULL if no problems)
    rpmps get_problems() { return rpmtsProblems(ts); }

    rpmdbMatchIterator match_packages(unsigned int value) {
        return rpmtsInitIterator(ts, RPMDBI_PACKAGES, &value, sizeof(value));
    }

    rpmdbMatchIterator match_tag(rpmDbiTagVal tag, const char * value) {
        libdnf_assert(tag != RPMDBI_PACKAGES, "Matching rpmdb tag RPMDBI_PACKAGES is not allowed");
        return rpmtsInitIterator(ts, tag, value, 0);
    }

    /// Process all package elements in a transaction set.
    /// Before calling rpmtsRun be sure to have:
    ///
    ///    - setup the rpm root dir via rpmtsSetRoot().
    ///    - setup the rpm notify callback via rpmtsSetNotifyCallback().
    ///    - setup the rpm transaction flags via rpmtsSetFlags().
    ///
    /// Additionally, though not required you may want to:
    ///
    ///    - setup the rpm verify signature flags via rpmtsSetVSFlags().
    ///
    /// @param okProbs  unused
    /// @param ignoreSet	bits to filter problem types
    /// @return		0 on success, -1 on error, >0 with newProbs set
    int run() {
        rpmprobFilterFlags ignore_set = RPMPROB_FILTER_NONE;
        if (downgrade_requested) {
            ignore_set |= RPMPROB_FILTER_OLDPACKAGE;
        }
        rpmtsSetNotifyStyle(ts, 1);
        rpmtsSetNotifyCallback(ts, ts_callback, &callbacks_holder);
        auto rc = rpmtsRun(ts, nullptr, ignore_set);
        rpmtsSetNotifyCallback(ts, nullptr, nullptr);

        return rc;
    }

    /// Get header of package at offset in the rpmdbi database
    Header get_header(unsigned int rec_offset) {
        Header hdr = nullptr;

        // rec_offset must be unsigned int
        auto * iter = rpmtsInitIterator(ts, RPMDBI_PACKAGES, &rec_offset, sizeof(rec_offset));
        if (!iter) {
            throw TransactionError(M_("Cannot init rpm database iterator"));
        }
        hdr = rpmdbNextIterator(iter);
        if (!hdr) {
            throw TransactionError(M_("Package was not found in rpm database"));
        }
        headerLink(hdr);
        rpmdbFreeIterator(iter);
        return hdr;
    }

private:
    friend class Transaction;

    struct CallbacksHolder {
        std::unique_ptr<TransactionCallbacks> callbacks;
        Impl * transaction;
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

    RpmLogGuard rpm_log_guard;

    /// Add package to be installed to transaction set.
    /// The transaction set is checked for duplicate package names.
    /// If found, the package with the "newest" EVR will be replaced.
    /// @param item  item to be erased
    /// @param action  one of TransactionItemAction::UPGRADE,
    ///     TransactionItemAction::DOWNGRADE, TransactionItemAction::INSTALL
    void install_up_down(TransactionItem & item, libdnf::transaction::TransactionItemAction action);

    static Nevra trans_element_to_nevra(rpmte te) {
        Nevra nevra;
        nevra.set_name(rpmteN(te));
        const char * epoch = rpmteE(te);
        nevra.set_epoch(epoch ? epoch : "0");
        nevra.set_version(rpmteV(te));
        nevra.set_release(rpmteR(te));
        nevra.set_arch(rpmteA(te));
        return nevra;
    };

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
    static int ts_change_callback(int event, rpmte te, rpmte other, void * data) {
        auto * transaction = static_cast<Impl *>(data);

        if (!other) {
            // explicit action caused by last_added_item
            rpmteSetUserdata(te, transaction->last_added_item);
            transaction->last_item_added_ts_element = true;
        } else {
            // action caused by librpm itself
            auto trigger_nevra = transaction->last_added_item->get_package().get_full_nevra();
            auto te_rpmdb_record_number = rpmteDBOffset(te);
            auto te_nevra = fmt::format(
                "{}-{}:{}-{}.{}", rpmteN(te), rpmteE(te) ? rpmteE(te) : "0", rpmteV(te), rpmteR(te), rpmteA(te));
            auto & logger = *transaction->base->get_logger();
            const char * te_type = "unknown";
            switch (rpmteType(te)) {
                case TR_ADDED:
                    te_type = "install package";
                    break;
                case TR_REMOVED:
                    te_type = "remove package";
                    break;
                case TR_RPMDB:
                    te_type = "package from_rpmdb";
                    break;
                //case TR_RESTORED:
                //TODO(jrohel): Newly added to librpm. What exactly does it do?
                //    te_type = "package will be restored";
                //    break;
                default:;
            }
            libdnf_assert(
                te_rpmdb_record_number != 0,
                "Implicit element {} type {} with zero record number (caused by {})",
                te_nevra,
                te_type,
                trigger_nevra);
            switch (event) {
                case RPMTS_EVENT_ADD: {
                    transaction->implicit_ts_elements.insert({te_rpmdb_record_number, te});
                    logger.debug(
                        "Implicitly added element {} type {} (caused by {})", te_nevra, te_type, trigger_nevra);
                    break;
                }
                case RPMTS_EVENT_DEL: {
                    libdnf_throw_assertion(
                        "Implicitly removed element {} type {} (caused by {})", te_nevra, te_type, trigger_nevra);
                }
            }
        }
        return 0;
    }

    static TransactionCallbacks::ScriptType rpm_tag_to_script_type(rpmTag_e tag) noexcept {
        switch (tag) {
            case RPMTAG_PREIN:
                return TransactionCallbacks::ScriptType::PRE_INSTALL;
            case RPMTAG_POSTIN:
                return TransactionCallbacks::ScriptType::POST_INSTALL;
            case RPMTAG_PREUN:
                return TransactionCallbacks::ScriptType::PRE_UNINSTALL;
            case RPMTAG_POSTUN:
                return TransactionCallbacks::ScriptType::POST_UNINSTALL;
            case RPMTAG_PRETRANS:
                return TransactionCallbacks::ScriptType::PRE_TRANSACTION;
            case RPMTAG_POSTTRANS:
                return TransactionCallbacks::ScriptType::POST_TRANSACTION;
            case RPMTAG_TRIGGERPREIN:
                return TransactionCallbacks::ScriptType::TRIGGER_PRE_INSTALL;
            case RPMTAG_TRIGGERIN:
                return TransactionCallbacks::ScriptType::TRIGGER_INSTALL;
            case RPMTAG_TRIGGERUN:
                return TransactionCallbacks::ScriptType::TRIGGER_UNINSTALL;
            case RPMTAG_TRIGGERPOSTUN:
                return TransactionCallbacks::ScriptType::TRIGGER_POST_UNINSTALL;
            default:
                return TransactionCallbacks::ScriptType::UNKNOWN;
        }
    }

#define libdnf_assert_transaction_item_set() libdnf_assert(item != nullptr, "TransactionItem is not set")

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
        rpmCallbackData data) {
        void * rc = nullptr;
        const auto & callbacks_holder = *static_cast<CallbacksHolder *>(data);
        auto & transaction = *callbacks_holder.transaction;
        auto & logger = *transaction.base->get_logger();
        auto * const callbacks = callbacks_holder.callbacks.get();
        auto * const trans_element = static_cast<rpmte>(const_cast<void *>(te));
        auto * const item = trans_element ? static_cast<TransactionItem *>(rpmteUserdata(trans_element)) : nullptr;

        switch (what) {
            case RPMCALLBACK_INST_PROGRESS:
                libdnf_assert_transaction_item_set();
                if (callbacks) {
                    callbacks->install_progress(*item, amount, total);
                }
                break;
            case RPMCALLBACK_INST_START:
                // Install? Maybe upgrade/downgrade/...obsolete?
                libdnf_assert_transaction_item_set();
                if (callbacks) {
                    callbacks->install_start(*item, total);
                }
                break;
            case RPMCALLBACK_INST_OPEN_FILE: {
                libdnf_assert_transaction_item_set();
                auto file_path = item->get_package().get_package_path();
                if (file_path.empty()) {
                    return nullptr;
                }
                transaction.fd_in_cb = Fopen(file_path.c_str(), "r.ufdio");
                rc = transaction.fd_in_cb;
            } break;
            case RPMCALLBACK_INST_CLOSE_FILE:
                if (transaction.fd_in_cb) {
                    Fclose(transaction.fd_in_cb);
                    transaction.fd_in_cb = nullptr;
                }
                break;
            case RPMCALLBACK_TRANS_PROGRESS:
                if (callbacks) {
                    callbacks->transaction_progress(amount, total);
                }
                break;
            case RPMCALLBACK_TRANS_START:
                if (callbacks) {
                    callbacks->transaction_start(total);
                }
                break;
            case RPMCALLBACK_TRANS_STOP:
                if (callbacks) {
                    callbacks->transaction_stop(total);
                }
                break;
            case RPMCALLBACK_UNINST_PROGRESS:
                libdnf_assert_transaction_item_set();
                if (callbacks) {
                    callbacks->uninstall_progress(*item, amount, total);
                }
                break;
            case RPMCALLBACK_UNINST_START:
                libdnf_assert_transaction_item_set();
                if (callbacks) {
                    callbacks->uninstall_start(*item, total);
                }
                break;
            case RPMCALLBACK_UNINST_STOP:
                libdnf_assert_transaction_item_set();
                if (callbacks) {
                    callbacks->uninstall_stop(*item, amount, total);
                }
                break;
            case RPMCALLBACK_REPACKAGE_PROGRESS:  // obsolete, unused
            case RPMCALLBACK_REPACKAGE_START:     // obsolete, unused
            case RPMCALLBACK_REPACKAGE_STOP:      // obsolete, unused
                logger.info("Warning: got RPMCALLBACK_REPACKAGE_* obsolete callback");
                break;
            case RPMCALLBACK_UNPACK_ERROR:
                libdnf_assert_transaction_item_set();
                if (callbacks) {
                    callbacks->unpack_error(*item);
                }
                break;
            case RPMCALLBACK_CPIO_ERROR:
                // Not found usage in librpm.
                libdnf_assert_transaction_item_set();
                if (callbacks) {
                    callbacks->cpio_error(*item);
                }
                break;
            case RPMCALLBACK_SCRIPT_ERROR:
                // amount is script tag
                // total is return code - if (!RPMSCRIPT_FLAG_CRITICAL) return_code = RPMRC_OK
                if (callbacks) {
                    callbacks->script_error(
                        item,
                        trans_element_to_nevra(trans_element),
                        rpm_tag_to_script_type(static_cast<rpmTag_e>(amount)),
                        total);
                }
                break;
            case RPMCALLBACK_SCRIPT_START:
                // amount is script tag
                if (callbacks) {
                    callbacks->script_start(
                        item,
                        trans_element_to_nevra(trans_element),
                        rpm_tag_to_script_type(static_cast<rpmTag_e>(amount)));
                }
                break;
            case RPMCALLBACK_SCRIPT_STOP:
                // amount is script tag
                // total is return code - if (error && !RPMSCRIPT_FLAG_CRITICAL) return_code = RPMRC_NOTFOUND
                if (callbacks) {
                    callbacks->script_stop(
                        item,
                        trans_element_to_nevra(trans_element),
                        rpm_tag_to_script_type(static_cast<rpmTag_e>(amount)),
                        total);
                }
                break;
            case RPMCALLBACK_INST_STOP:
                libdnf_assert_transaction_item_set();
                if (callbacks) {
                    callbacks->install_stop(*item, amount, total);
                }
                break;
            case RPMCALLBACK_ELEM_PROGRESS:
                libdnf_assert_transaction_item_set();
                if (callbacks) {
                    callbacks->elem_progress(*item, amount, total);
                }
                break;
            case RPMCALLBACK_VERIFY_PROGRESS:
                if (callbacks) {
                    callbacks->verify_progress(amount, total);
                }
                break;
            case RPMCALLBACK_VERIFY_START:
                if (callbacks) {
                    callbacks->verify_start(total);
                }
                break;
            case RPMCALLBACK_VERIFY_STOP:
                if (callbacks) {
                    callbacks->verify_stop(total);
                }
                break;
            case RPMCALLBACK_UNKNOWN:
                logger.warning("Unknown RPM Transaction callback type: RPMCALLBACK_UNKNOWN");
        }

        return rc;
    }
};

#undef libdnf_assert_transaction_item_set

Transaction::Impl::Impl(const BaseWeakPtr & base, rpmVSFlags vsflags) : base(base), rpm_log_guard(base) {
    ts = rpmtsCreate();
    auto & config = base->get_config();
    set_root_dir(config.installroot().get_value().c_str());
    set_signature_verify_flags(vsflags);
    rpmtsSetChangeCallback(ts, ts_change_callback, this);
}

Transaction::Impl::Impl(const BaseWeakPtr & base)
    : Impl(base, static_cast<rpmVSFlags>(rpmExpandNumeric("%{?__vsflags}"))) {}

Transaction::Impl::~Impl() {
    rpmtsFree(ts);
    if (script_fd) {
        Fclose(script_fd);
    }
}

Transaction::Transaction(const BaseWeakPtr & base) : p_impl(new Impl(base)) {}

Transaction::Transaction(Base & base) : Transaction(base.get_weak_ptr()) {}

void Transaction::Impl::install_up_down(TransactionItem & item, libdnf::transaction::TransactionItemAction action) {
    std::string msg_action;
    bool upgrade{true};
    if (action == libdnf::transaction::TransactionItemAction::UPGRADE) {
        msg_action = "upgrade";
    } else if (action == libdnf::transaction::TransactionItemAction::DOWNGRADE) {
        downgrade_requested = true;
        msg_action = "downgrade";
    } else if (action == libdnf::transaction::TransactionItemAction::INSTALL) {
        upgrade = false;
        msg_action = "install";
    } else {
        libdnf_throw_assertion("Unsupported action: {}", utils::to_underlying(action));
    }
    auto file_path = item.get_package().get_package_path();
    auto * header = read_pkg_header(file_path);
    last_added_item = &item;
    last_item_added_ts_element = false;
    auto rc = rpmtsAddInstallElement(ts, header, &item, upgrade ? 1 : 0, nullptr);
    headerFree(header);
    if (rc != 0) {
        //TODO(jrohel): Why? Librpm does not provide this information.
        throw TransactionError(M_("Cannot {} package \"{}\""), msg_action, item.get_package().get_full_nevra());
    }
    libdnf_assert(
        last_item_added_ts_element,
        "librpm has ignored explicit request to {} package \"{}\"",
        msg_action,
        item.get_package().get_full_nevra());
}

Transaction::~Transaction() = default;

void Transaction::set_root_dir(const char * root_dir) {
    p_impl->set_root_dir(root_dir);
}

const char * Transaction::get_root_dir() const {
    return p_impl->get_root_dir();
}

rpmtransFlags Transaction::get_flags() const {
    return p_impl->get_flags();
}

rpmtransFlags Transaction::set_flags(rpmtransFlags flags) {
    return p_impl->set_flags(flags);
}

rpmVSFlags Transaction::get_signature_verify_flags() const {
    return p_impl->get_signature_verify_flags();
}

rpmVSFlags Transaction::set_signature_verify_flags(rpmVSFlags verify_flags) {
    return p_impl->set_signature_verify_flags(verify_flags);
}

rpmVSFlags Transaction::get_pkg_verify_flags() const {
    return p_impl->get_pkg_verify_flags();
}

rpmVSFlags Transaction::set_pkg_verify_flags(rpmVSFlags verify_flags) {
    return p_impl->set_pkg_verify_flags(verify_flags);
}

int Transaction::get_pkg_verify_level() const {
    return p_impl->get_pkg_verify_level();
}

int Transaction::set_pkg_verify_level(int verify_level) {
    return p_impl->set_pkg_verify_level(verify_level);
}

std::string Transaction::get_db_cookie() const {
    return p_impl->get_db_cookie();
}

rpm_tid_t Transaction::get_id() const {
    return p_impl->get_id();
}

void Transaction::set_callbacks(std::unique_ptr<TransactionCallbacks> && callbacks) {
    p_impl->callbacks_holder.callbacks = std::move(callbacks);
}

void Transaction::fill(const base::Transaction & transaction) {
    p_impl->fill(transaction.get_transaction_packages());
    libdnf_assert(p_impl->implicit_ts_elements.empty(), "The rpm transaction contains more elements than requested");
}

bool Transaction::check() {
    return p_impl->check();
}

int Transaction::run() {
    return p_impl->run();
}

RpmProblemSet Transaction::get_problems() {
    auto * problem_set = p_impl->get_problems();
    auto rpm_problem_set_impl = std::make_unique<RpmProblemSet::Impl>(problem_set);
    return RpmProblemSet(std::move(rpm_problem_set_impl));
}

void Transaction::set_script_out_fd(int fd) {
    if (fd == -1) {
        p_impl->set_script_fd(nullptr);
        return;
    }
    auto * script_fd = fdDup(fd);
    if (!script_fd) {
        throw TransactionError(
            M_("Failed to set scriptlet output file, cannot duplicate file descriptor: {}"),
            std::string(Fstrerror(script_fd)));
    }
    p_impl->set_script_fd(script_fd);
}

void Transaction::set_script_out_file(const std::string & file_path) {
    auto * script_fd = Fopen(file_path.c_str(), "w+b");
    if (!script_fd) {
        throw TransactionError(
            M_("Failed to set scriptlet output file, cannot open file \"{}\": {}"),
            file_path,
            std::string(Fstrerror(script_fd)));
    }
    p_impl->set_script_fd(script_fd);
}

BaseWeakPtr Transaction::get_base() const {
    return p_impl->base;
}

}  // namespace libdnf::rpm
