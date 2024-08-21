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


#include "transaction.hpp"

#include "libdnf5/base/transaction.hpp"
#include "libdnf5/common/exception.hpp"
#include "libdnf5/rpm/package_query.hpp"
#include "libdnf5/transaction/transaction_item_action.hpp"
#include "libdnf5/utils/bgettext/bgettext-mark-domain.h"
#include "libdnf5/utils/to_underlying.hpp"

#include <fcntl.h>
#include <fmt/format.h>
#include <libdnf5/logger/log_router.hpp>
#include <rpm/rpmbuild.h>
#include <rpm/rpmdb.h>
#include <rpm/rpmlib.h>
#include <rpm/rpmpgp.h>
#include <rpm/rpmtag.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstring>
#include <exception>
#include <map>
#include <mutex>
#include <string_view>
#include <thread>

namespace libdnf5::rpm {

RpmHeader & RpmHeader::operator=(const RpmHeader & src) {
    if (&src != this) {
        headerFree(header);
        header = headerLink(src.header);
    }
    return *this;
}

RpmHeader & RpmHeader::operator=(RpmHeader && src) {
    if (&src != this) {
        headerFree(header);
        header = src.header;
        src.header = nullptr;
    }
    return *this;
}

std::string RpmHeader::get_evr() const {
    auto * tmp = headerGetAsString(header, RPMTAG_EVR);
    std::string ret = tmp;
    rfree(tmp);
    return ret;
}

std::string RpmHeader::get_nevra() const {
    auto * tmp = headerGetAsString(header, RPMTAG_NEVRA);
    std::string ret = tmp;
    rfree(tmp);
    return ret;
}

std::string RpmHeader::get_full_nevra() const {
    std::stringstream out;
    out << get_name() << '-' << get_epoch() << ':' << get_version() << '-' << get_release() << '.' << get_arch();
    return out.str();
}

RpmProblemSet::Iterator & RpmProblemSet::Iterator::operator++() {
    if (!rpmpsiNext(iter)) {
        free();
    }
    return *this;
};

Transaction::Transaction(const BaseWeakPtr & base) : base(base), rpm_log_guard(base) {
    ts = rpmtsCreate();
    auto & config = base->get_config();
    set_root_dir(config.get_installroot_option().get_value().c_str());
    auto vsflags = static_cast<rpmVSFlags>(rpmExpandNumeric("%{?__vsflags}"));
    set_signature_verify_flags(vsflags);
    rpmtsSetChangeCallback(ts, ts_change_callback, this);
}

Transaction::Transaction(Base & base) : Transaction(base.get_weak_ptr()) {}

Transaction::~Transaction() {
    rpmtsFree(ts);
    if (script_fd) {
        Fclose(script_fd);
    }
}


void Transaction::set_root_dir(const char * root_dir) {
    auto rc = rpmtsSetRootDir(ts, root_dir);
    if (rc != 0) {
        //TODO(jrohel): Why? Librpm does not provide this information.
        throw TransactionError(M_("Cannot set root directory \"{}\""), std::string(root_dir));
    }
}

std::string Transaction::get_db_cookie() const {
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

void Transaction::fill(const base::Transaction & transaction) {
    transaction_items = transaction.get_transaction_packages();

    // Auxiliary map name->package with the latest versions of currently
    // installed installonly packages.
    // Used to detect installation of a installonly package with lower version
    // that is currently installed.
    std::map<std::string, libdnf5::rpm::Package> installonly_versions;
    rpm::PackageQuery installonly_query(base);
    installonly_query.filter_installed();
    installonly_query.filter_latest_evr();
    installonly_query.filter_installonly();
    for (const auto & pkg : installonly_query) {
        installonly_versions.insert(std::make_pair(pkg.get_name(), pkg));
    }

    for (auto & tspkg : transaction_items) {
        switch (tspkg.get_action()) {
            case libdnf5::transaction::TransactionItemAction::INSTALL:
                install(tspkg);
                // Inbound installonly packages always have `INSTALL` action,
                // there is no `DOWNGRADE`. We need to detect downgrade
                // manually to correctly add RPMPROB_FILTER_OLDPACKAGE to rpm
                // transaction later during run() execution.
                if (!downgrade_requested) {
                    auto pkg = tspkg.get_package();
                    auto installonly_it = installonly_versions.find(pkg.get_name());
                    if (installonly_it != installonly_versions.end()) {
                        if (libdnf5::rpm::cmp_nevra(pkg, installonly_it->second) > 0) {
                            downgrade_requested = true;
                        }
                    }
                }
                break;
            case libdnf5::transaction::TransactionItemAction::UPGRADE:
                upgrade(tspkg);
                break;
            case libdnf5::transaction::TransactionItemAction::DOWNGRADE:
                downgrade(tspkg);
                break;
            case libdnf5::transaction::TransactionItemAction::REINSTALL:
                reinstall(tspkg);
                break;
            case libdnf5::transaction::TransactionItemAction::REMOVE:
            case libdnf5::transaction::TransactionItemAction::REPLACED:
                erase(tspkg);
                break;
            case libdnf5::transaction::TransactionItemAction::REASON_CHANGE:
            case libdnf5::transaction::TransactionItemAction::ENABLE:
            case libdnf5::transaction::TransactionItemAction::DISABLE:
            case libdnf5::transaction::TransactionItemAction::RESET:
            case libdnf5::transaction::TransactionItemAction::SWITCH:
                break;
        }
    }
    libdnf_assert(implicit_ts_elements.empty(), "The rpm transaction contains more elements than requested");

    // generate ordering for the rpm transaction
    if (rpmtsOrder(ts)) {
        throw TransactionError(M_("Failed to order the rpm transaction."));
    };
}

void Transaction::process_scriptlets_output(int fd, Transaction * transaction) {
    auto logger = transaction->base->get_logger().get();
    try {
        char buf[512];
        do {
            auto len = read(fd, buf, sizeof(buf));
            if (len > 0) {
                std::string_view str(buf, static_cast<size_t>(len));
                {
                    std::lock_guard<std::mutex> lock(transaction->last_script_output_mutex);
                    transaction->last_script_output.append(str);
                }
                std::string_view::size_type start = 0;
                do {
                    auto end = str.find('\n', start);
                    logger->info("[scriptlet] {}", str.substr(start, end - start));
                    if (end == std::string_view::npos) {
                        break;
                    }
                    start = end + 1;
                } while (start < str.size());
            } else {
                if (len == -1) {
                    logger->error("Transaction::Run: Cannot read scriptlet output from pipe: {}", std::strerror(errno));
                }
                break;
            }
        } while (true);
    } catch (const std::exception & ex) {
        // The thread must not throw exceptions.
        logger->error("Transaction::Run: Exception while processing scriptlet output: {}", ex.what());
    }
    close(fd);
}

int Transaction::run() {
    rpmprobFilterFlags ignore_set = RPMPROB_FILTER_NONE;
    if (downgrade_requested) {
        ignore_set |= RPMPROB_FILTER_OLDPACKAGE;
    }
    if (base->get_config().get_ignorearch_option().get_value()) {
        ignore_set |= RPMPROB_FILTER_IGNOREARCH;
    }
    // do not process the scriptlets output for test transactions
    bool catch_scriptlet_output = (get_flags() & RPMTRANS_FLAG_TEST) != RPMTRANS_FLAG_TEST;
    auto logger = base->get_logger().get();
    int pipe_out_from_scriptlets[2];
    std::thread thread_processes_scriptlets_output;
    if (catch_scriptlet_output) {
        if (pipe(pipe_out_from_scriptlets) == -1) {
            logger->error("Transaction::Run: Cannot create pipe: {}", std::strerror(errno));
            return -1;
        }

        // This thread processes the output of RPM scriptlets.
        thread_processes_scriptlets_output = std::thread(process_scriptlets_output, pipe_out_from_scriptlets[0], this);

        // Set file descriptor for output of scriptlets in transaction.
        set_script_out_fd(pipe_out_from_scriptlets[1]);
        // set_script_out_fd() copies the file descriptor using dup(). Closing the original fd.
        close(pipe_out_from_scriptlets[1]);
    }
    rpmtsSetNotifyStyle(ts, 1);
    rpmtsSetNotifyCallback(ts, ts_callback, &callbacks_holder);
    auto * const callbacks = callbacks_holder.callbacks.get();
    if (callbacks) {
        int nelements = rpmtsNElements(ts);
        libdnf_assert(nelements >= 0, "librpm rpmtsNElements() returned negative number of transaction elements.");
        callbacks->before_begin(static_cast<uint64_t>(nelements));
    }
    auto rc = rpmtsRun(ts, nullptr, ignore_set);
    if (callbacks) {
        callbacks->after_complete(rc == 0);
    }
    rpmtsSetNotifyCallback(ts, nullptr, nullptr);

    if (catch_scriptlet_output) {
        // Reset/close file descriptor for output of RPM scriptlets. Required to end thread_processes_scriptlets_output.
        set_script_out_fd(-1);
        thread_processes_scriptlets_output.join();
    }

    return rc;
}

void Transaction::set_script_out_fd(int fd) {
    FD_t script_fd;

    if (fd != -1) {
        script_fd = fdDup(fd);
        if (!script_fd) {
            throw TransactionError(
                M_("Failed to set scriptlet output file, cannot duplicate file descriptor: {}"),
                std::string(Fstrerror(script_fd)));
        }
    } else {
        script_fd = nullptr;
    }

    rpmtsSetScriptFd(ts, script_fd);

    if (this->script_fd) {
        Fclose(this->script_fd);
    }
    this->script_fd = script_fd;
}

void Transaction::set_script_out_file(const std::string & file_path) {
    auto * script_fd = Fopen(file_path.c_str(), "w+b");
    if (!script_fd) {
        throw TransactionError(
            M_("Failed to set scriptlet output file, cannot open file \"{}\": {}"),
            file_path,
            std::string(Fstrerror(script_fd)));
    }
    rpmtsSetScriptFd(ts, script_fd);

    if (this->script_fd) {
        Fclose(this->script_fd);
    }
    this->script_fd = script_fd;
}

BaseWeakPtr Transaction::get_base() const {
    return base;
}

Header Transaction::read_pkg_header(const std::string & file_path) const {
    FD_t fd = Fopen(file_path.c_str(), "r.ufdio");

    if (!fd) {
        throw TransactionError(
            M_("Failed to read package header, cannot open file \"{}\": {}"), file_path, std::string(Fstrerror(fd)));
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

Header Transaction::get_header(unsigned int rec_offset) {
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

void Transaction::reinstall(TransactionItem & item) {
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

void Transaction::erase(TransactionItem & item) {
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

void Transaction::install_up_down(TransactionItem & item, libdnf5::transaction::TransactionItemAction action) {
    BgettextMessage msg_error;
    std::string msg_action;
    bool upgrade{true};
    if (action == libdnf5::transaction::TransactionItemAction::UPGRADE) {
        msg_error = M_("Cannot upgrade package \"{}\"");
        msg_action = "upgrade";
    } else if (action == libdnf5::transaction::TransactionItemAction::DOWNGRADE) {
        downgrade_requested = true;
        msg_error = M_("Cannot downgrade package \"{}\"");
        msg_action = "downgrade";
    } else if (action == libdnf5::transaction::TransactionItemAction::INSTALL) {
        upgrade = false;
        msg_error = M_("Cannot install package \"{}\"");
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
        throw TransactionError(msg_error, item.get_package().get_full_nevra());
    }
    libdnf_assert(
        last_item_added_ts_element,
        "librpm has ignored explicit request to {} package \"{}\"",
        msg_action,
        item.get_package().get_full_nevra());
}

Nevra Transaction::trans_element_to_nevra(rpmte te) {
    Nevra nevra;
    nevra.set_name(rpmteN(te));
    const char * epoch = rpmteE(te);
    nevra.set_epoch(epoch ? epoch : "0");
    nevra.set_version(rpmteV(te));
    nevra.set_release(rpmteR(te));
    nevra.set_arch(rpmteA(te));
    return nevra;
};

int Transaction::ts_change_callback(int event, rpmte te, rpmte other, void * data) {
    auto * transaction = static_cast<Transaction *>(data);

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
                logger.debug("Implicitly added element {} type {} (caused by {})", te_nevra, te_type, trigger_nevra);
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

TransactionCallbacks::ScriptType Transaction::rpm_tag_to_script_type(rpmTag_e tag) noexcept {
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

void * Transaction::ts_callback(
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
            logger.info(
                "RPM callback install start \"{}\" total {}",
                to_full_nevra_string(trans_element_to_nevra(trans_element)),
                total);
            if (callbacks) {
                callbacks->install_start(*item, total);
            }
            break;
        case RPMCALLBACK_INST_OPEN_FILE: {
            libdnf_assert_transaction_item_set();
            auto file_path = item->get_package().get_package_path();
            logger.info("RPM callback open file \"{}\"", file_path);
            if (file_path.empty()) {
                return nullptr;
            }
            transaction.fd_in_cb = Fopen(file_path.c_str(), "r.ufdio");
            rc = transaction.fd_in_cb;
            break;
        }
        case RPMCALLBACK_INST_CLOSE_FILE:
            logger.info("RPM callback close file");
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
            logger.info("RPM callback transaction start, total {}", total);
            if (callbacks) {
                callbacks->transaction_start(total);
            }
            break;
        case RPMCALLBACK_TRANS_STOP:
            logger.info("RPM callback transaction stop, total {}", total);
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
            logger.info(
                "RPM callback uninstall start \"{}\" total {}",
                to_full_nevra_string(trans_element_to_nevra(trans_element)),
                total);
            if (callbacks) {
                callbacks->uninstall_start(*item, total);
            }
            break;
        case RPMCALLBACK_UNINST_STOP:
            libdnf_assert_transaction_item_set();
            logger.info(
                "RPM callback uninstall stop \"{}\" amount {} total {}",
                to_full_nevra_string(trans_element_to_nevra(trans_element)),
                amount,
                total);
            if (callbacks) {
                callbacks->uninstall_stop(*item, amount, total);
            }
            break;
        case RPMCALLBACK_UNPACK_ERROR:
            libdnf_assert_transaction_item_set();
            logger.error(
                "RPM callback unpack error \"{}\"", to_full_nevra_string(trans_element_to_nevra(trans_element)));
            if (callbacks) {
                callbacks->unpack_error(*item);
            }
            break;
        case RPMCALLBACK_CPIO_ERROR:
            // Not found usage in librpm.
            libdnf_assert_transaction_item_set();
            logger.error("RPM callback CPIO error \"{}\"", to_full_nevra_string(trans_element_to_nevra(trans_element)));
            if (callbacks) {
                callbacks->cpio_error(*item);
            }
            break;
        case RPMCALLBACK_SCRIPT_ERROR: {
            // amount is script tag
            auto script_type = rpm_tag_to_script_type(static_cast<rpmTag_e>(amount));
            auto nevra = trans_element_to_nevra(trans_element);
            // total is return code - if (!RPMSCRIPT_FLAG_CRITICAL) return_code = RPMRC_OK
            // therefore in case RPMRC_OK is passed, nothing critical happened
            // don't confuse the user by telling an error occurred, but exit code is "success"
            auto return_code = static_cast<rpmRC_e>(total);
            if (return_code != RPMRC_OK) {
                logger.error(
                    "RPM callback error in {} scriptlet \"{}\" return code {}",
                    TransactionCallbacks::script_type_to_string(script_type),
                    to_full_nevra_string(nevra),
                    total);
                if (callbacks) {
                    std::lock_guard<std::mutex> lock(transaction.last_script_output_mutex);
                    callbacks->script_output(item, nevra, script_type, return_code, transaction.last_script_output);
                    callbacks->script_error(item, nevra, script_type, return_code);
                }
            } else {
                logger.warning(
                    "RPM callback non-critical error in {} scriptlet \"{}\"",
                    TransactionCallbacks::script_type_to_string(script_type),
                    to_full_nevra_string(nevra));
            }
            break;
        }
        case RPMCALLBACK_SCRIPT_START: {
            // amount is script tag
            auto script_type = rpm_tag_to_script_type(static_cast<rpmTag_e>(amount));
            auto nevra = trans_element_to_nevra(trans_element);
            logger.info(
                "RPM callback start {} scriptlet \"{}\"",
                TransactionCallbacks::script_type_to_string(script_type),
                to_full_nevra_string(nevra));
            if (callbacks) {
                callbacks->script_start(item, nevra, script_type);
            }
            std::lock_guard<std::mutex> lock(transaction.last_script_output_mutex);
            transaction.last_script_output = "";
            break;
        }
        case RPMCALLBACK_SCRIPT_STOP: {
            // amount is script tag
            // total is return code - if (error && !RPMSCRIPT_FLAG_CRITICAL) return_code = RPMRC_NOTFOUND
            auto script_type = rpm_tag_to_script_type(static_cast<rpmTag_e>(amount));
            auto nevra = trans_element_to_nevra(trans_element);
            logger.info(
                "RPM callback stop {} scriptlet \"{}\" return code {}",
                TransactionCallbacks::script_type_to_string(script_type),
                to_full_nevra_string(nevra),
                total);
            if (callbacks) {
                callbacks->script_output(item, nevra, script_type, total, transaction.last_script_output);
                callbacks->script_stop(item, nevra, script_type, total);
            }
            break;
        }
        case RPMCALLBACK_INST_STOP:
            libdnf_assert_transaction_item_set();
            logger.info(
                "RPM callback install stop \"{}\" amount {} total {}",
                to_full_nevra_string(trans_element_to_nevra(trans_element)),
                amount,
                total);
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
            logger.info("RPM callback verify start, total {}", total);
            if (callbacks) {
                callbacks->verify_start(total);
            }
            break;
        case RPMCALLBACK_VERIFY_STOP:
            logger.info("RPM callback verify stop, total {}", total);
            if (callbacks) {
                callbacks->verify_stop(total);
            }
            break;
        default:
            logger.warning("Unknown RPM Transaction callback type {}", what);
            break;
    }

    return rc;
}

#undef libdnf_assert_transaction_item_set

}  // namespace libdnf5::rpm
