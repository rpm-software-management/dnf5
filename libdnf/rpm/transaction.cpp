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


#include "libdnf/rpm/transaction.hpp"

#include "../libdnf/rpm/solv/package_private.hpp"
#include "../libdnf/utils/bgettext/bgettext-lib.h"
#include "package_sack_impl.hpp"
#include "package_set_impl.hpp"
#include "../repo/repo_impl.hpp"

#include "libdnf/transaction/transaction_item_action.hpp"

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
            throw Exception(std::string("Invalid root directory: ") + root_dir);
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
            throw Exception("read_pkg_header: Can't open file: " + file_path);
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
                throw Exception("read_pkg_header: failed");
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
        auto file_path = item.get_pkg().get_package_path();
        auto * header = read_pkg_header(file_path);
        auto rc = rpmtsAddReinstallElement(ts, header, &item);
        if (rc != 0) {
            std::string msg = "Can't reinstall package \"" + file_path + "\"";
            throw Exception(msg);
        }
    }

    /// Add package to be erased to transaction set.
    /// @param item  item to be erased
    void erase(TransactionItem & item) {
        auto rpmdb_id = static_cast<unsigned int>(item.get_pkg().get_rpmdbid());
        auto * header = get_header(rpmdb_id);
        int unused = -1;
        int rc = rpmtsAddEraseElement(ts, header, unused);
        headerFree(header);
        if (rc != 0) {
            throw Exception("Can't remove package");
        }
        auto [iter, inserted] = items.insert({rpmdb_id, &item});
        if (!inserted) {
            throw Exception("The package already added to be erased in rpm::Transaction");
        }
    };

    // Set transaction notify callback.
    void register_cb(TransactionCB * cb) { cb_info.cb = cb; }

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
        if (tag == RPMDBI_PACKAGES) {
            throw libdnf::LogicError("rpm::Transaction::match(): not allowed tag RPMDBI_PACKAGES");
        }
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
        if (cb_info.cb) {
            rpmtsSetNotifyCallback(ts, ts_callback, &cb_info);
        }
        auto rc = rpmtsRun(ts, nullptr, ignore_set);
        if (cb_info.cb) {
            rpmtsSetNotifyCallback(ts, nullptr, nullptr);
        }

        return rc;
    }

    /// Get header of package at offset in the rpmdbi database
    Header get_header(unsigned int rec_offset) {
        Header hdr = nullptr;

        // rec_offset must be unsigned int
        auto * iter = rpmtsInitIterator(ts, RPMDBI_PACKAGES, &rec_offset, sizeof(rec_offset));
        if (!iter) {
            throw Exception(_("Fatal error, run database recovery"));
        }
        hdr = rpmdbNextIterator(iter);
        if (!hdr) {
            throw Exception(_("failed to find package"));
        }
        headerLink(hdr);
        rpmdbFreeIterator(iter);
        return hdr;
    }

private:
    friend class Transaction;

    struct CallbackInfo {
        TransactionCB * cb;
        Impl * transaction;
    };

    BaseWeakPtr base;
    rpmts ts;
    FD_t script_fd{nullptr};
    CallbackInfo cb_info{nullptr, this};
    FD_t fd_in_cb{nullptr};  // file descriptor used by transaction in callback (install/reinstall package)
    std::map<unsigned int, TransactionItem *> items{};
    bool downgrade_requested{false};

    /// Add package to be installed to transaction set.
    /// The transaction set is checked for duplicate package names.
    /// If found, the package with the "newest" EVR will be replaced.
    /// @param item  item to be erased
    /// @param action  one of TransactionItemAction::UPGRADE,
    ///     TransactionItemAction::DOWNGRADE, TransactionItemAction::INSTALL
    void install_up_down(TransactionItem & item, libdnf::transaction::TransactionItemAction action);

    /// Function triggered by rpmtsNotify()
    ///
    /// @param hd  related header or NULL
    /// @param what  kind of notification
    /// @param amount  number of bytes/packages already processed or tag of the scriptlet involved
    ///                or 0 or some other number
    /// @param total  total number of bytes/packages to be processed or return code of the scriptlet or 0
    /// @param key  result of rpmteKey() of related rpmte or 0
    /// @param data  user data as passed to rpmtsSetNotifyCallback()
    static void * ts_callback(
        const void * hd,
        const rpmCallbackType what,
        const rpm_loff_t amount,
        const rpm_loff_t total,
        const void * pkg_key,
        rpmCallbackData data) {
        void * rc = nullptr;
        auto * cb_info = static_cast<CallbackInfo *>(data);
        auto * transaction = cb_info->transaction;
        auto & log = *transaction->base->get_logger();
        auto & cb = *cb_info->cb;
        auto * hdr = const_cast<headerToken_s *>(static_cast<const headerToken_s *>(hd));
        const auto * item = static_cast<const TransactionItem *>(pkg_key);
        if (!item && hdr) {
            auto iter = transaction->items.find(headerGetInstance(hdr));
            if (iter != transaction->items.end()) {
                item = iter->second;
            }
        }

        switch (what) {
            case RPMCALLBACK_INST_PROGRESS:
                cb.install_progress(item, RpmHeader(hdr), amount, total);
                break;
            case RPMCALLBACK_INST_START:
                // Install? Maybe upgrade/downgrade/...obsolete?
                cb.install_start(item, RpmHeader(hdr), total);
                break;
            case RPMCALLBACK_INST_OPEN_FILE: {
                auto file_path = item->get_pkg().get_package_path();
                if (file_path.empty()) {
                    return nullptr;
                }
                transaction->fd_in_cb = Fopen(file_path.c_str(), "r.ufdio");
                rc = transaction->fd_in_cb;
            } break;
            case RPMCALLBACK_INST_CLOSE_FILE:
                if (transaction->fd_in_cb) {
                    Fclose(transaction->fd_in_cb);
                    transaction->fd_in_cb = nullptr;
                }
                break;
            case RPMCALLBACK_TRANS_PROGRESS:
                cb.transaction_progress(amount, total);
                break;
            case RPMCALLBACK_TRANS_START:
                cb.transaction_start(total);
                break;
            case RPMCALLBACK_TRANS_STOP:
                cb.transaction_stop(total);
                break;
            case RPMCALLBACK_UNINST_PROGRESS:
                cb.uninstall_progress(item, RpmHeader(hdr), amount, total);
                break;
            case RPMCALLBACK_UNINST_START:
                cb.uninstall_start(item, RpmHeader(hdr), total);
                break;
            case RPMCALLBACK_UNINST_STOP:
                cb.uninstall_stop(item, RpmHeader(hdr), amount, total);
                break;
            case RPMCALLBACK_REPACKAGE_PROGRESS:  // obsolete, unused
            case RPMCALLBACK_REPACKAGE_START:     // obsolete, unused
            case RPMCALLBACK_REPACKAGE_STOP:      // obsolete, unused
                log.info("Warning: got RPMCALLBACK_REPACKAGE_* obsolete callback");
                break;
            case RPMCALLBACK_UNPACK_ERROR:
                cb.unpack_error(item, RpmHeader(hdr));
                break;
            case RPMCALLBACK_CPIO_ERROR:
                // Not found usage in librpm.
                cb.cpio_error(item, RpmHeader(hdr));
                break;
            case RPMCALLBACK_SCRIPT_ERROR:
                // amount is script tag
                // total is return code - if (!RPMSCRIPT_FLAG_CRITICAL) return_code = RPMRC_OK
                cb.script_error(item, RpmHeader(hdr), amount, total);
                break;
            case RPMCALLBACK_SCRIPT_START:
                // amount is script tag
                // TODO(jrohel): Define enum
                //   RPMTAG_PREIN; "%pre";
                //   RPMTAG_POSTIN; "%post";
                //   RPMTAG_PREUN; "%preun";
                //   RPMTAG_POSTUN; "%postun";
                //   RPMTAG_PRETRANS; "%pretrans";
                //   RPMTAG_POSTTRANS; "%posttrans";
                //   RPMTAG_VERIFYSCRIPT; "%verifyscript";
                //   RPMTAG_TRIGGERSCRIPTS; "%triggerprein";
                //   RPMTAG_TRIGGERSCRIPTS; "%triggerin";
                //   RPMTAG_TRIGGERSCRIPTS; "%triggerun";
                //   RPMTAG_TRIGGERSCRIPTS; "%triggerpostun";
                //   RPMTAG_FILETRIGGERSCRIPTS; "%filetriggerin";
                //   RPMTAG_FILETRIGGERSCRIPTS; "%filetriggerun";
                //   RPMTAG_FILETRIGGERSCRIPTS; "%filetriggerpostun";
                //   RPMTAG_TRANSFILETRIGGERSCRIPTS; "%transfiletriggerin";
                //   RPMTAG_TRANSFILETRIGGERSCRIPTS; "%transfiletriggerun";
                //   RPMTAG_TRANSFILETRIGGERSCRIPTS; "%transfiletriggerpostun";
                cb.script_start(item, RpmHeader(hdr), amount);
                break;
            case RPMCALLBACK_SCRIPT_STOP:
                // amount is script tag
                // total is return code - if (error && !RPMSCRIPT_FLAG_CRITICAL) return_code = RPMRC_NOTFOUND
                cb.script_stop(item, RpmHeader(hdr), amount, total);
                break;
            case RPMCALLBACK_INST_STOP:
                cb.install_stop(item, RpmHeader(hdr), amount, total);
                break;
            case RPMCALLBACK_ELEM_PROGRESS:
                cb.elem_progress(item, RpmHeader(hdr), amount, total);
                break;
            case RPMCALLBACK_VERIFY_PROGRESS:
                cb.verify_progress(amount, total);
                break;
            case RPMCALLBACK_VERIFY_START:
                cb.verify_start(total);
                break;
            case RPMCALLBACK_VERIFY_STOP:
                cb.verify_stop(total);
                break;
            case RPMCALLBACK_UNKNOWN:
                log.warning("Unknown RPM Transaction callback type: RPMCALLBACK_UNKNOWN");
        }

        return rc;
    }
};


Transaction::Impl::Impl(const BaseWeakPtr & base, rpmVSFlags vsflags) : base(base) {
    ts = rpmtsCreate();
    auto & config = base->get_config();
    set_root_dir(config.installroot().get_value().c_str());
    set_signature_verify_flags(vsflags);
}

Transaction::Impl::Impl(const BaseWeakPtr & base) : Impl(base, static_cast<rpmVSFlags>(rpmExpandNumeric("%{?__vsflags}"))) {}

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
        throw LogicError("Unsupported action");
    }
    auto file_path = item.get_pkg().get_package_path();
    auto * header = read_pkg_header(file_path);
    auto rc = rpmtsAddInstallElement(ts, header, &item, upgrade ? 1 : 0, nullptr);
    if (rc != 0) {
        std::string msg = "Can't " + msg_action + " package \"" + file_path + "\"";
        throw Exception(msg);
    }
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

rpm_tid_t Transaction::get_id() const {
    return p_impl->get_id();
}

void Transaction::register_cb(TransactionCB * cb) {
    p_impl->register_cb(cb);
}

void Transaction::install(TransactionItem & item) {
    p_impl->install(item);
}

void Transaction::upgrade(TransactionItem & item) {
    p_impl->upgrade(item);
}

void Transaction::downgrade(TransactionItem & item) {
    p_impl->downgrade(item);
}

void Transaction::reinstall(TransactionItem & item) {
    p_impl->reinstall(item);
}

void Transaction::erase(TransactionItem & item) {
    p_impl->erase(item);
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
        throw Exception("fdDup()");
    }
    p_impl->set_script_fd(script_fd);
}

void Transaction::set_script_out_file(const std::string & file_path) {
    auto * script_fd = Fopen(file_path.c_str(), "w+b");
    if (!script_fd) {
        throw Exception("Fopen(): " + file_path);
    }
    p_impl->set_script_fd(script_fd);
}

BaseWeakPtr Transaction::get_base() const {
    return p_impl->base;
}

}  // namespace libdnf::rpm
