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

#include "libdnf/rpm/rpm_signature.hpp"

#include "repo/repo_pgp.hpp"
#include "rpm/rpm_log_guard.hpp"
#include "utils/bgettext/bgettext-lib.h"
#include "utils/bgettext/bgettext-mark-domain.h"
#include "utils/fs/temp.hpp"
#include "utils/url.hpp"

#include "libdnf/repo/file_downloader.hpp"
#include "libdnf/repo/repo.hpp"

#include <rpm/rpmcli.h>
#include <rpm/rpmdb.h>
#include <rpm/rpmkeyring.h>
#include <rpm/rpmlog.h>
#include <rpm/rpmpgp.h>
#include <rpm/rpmts.h>

namespace libdnf::rpm {

using RpmTransactionPtr = std::unique_ptr<rpmts_s, std::function<void(rpmts_s * ts)>>;
// deleters for unique_ptrs that take ownership of rpm objects
// once the ownership is transfered, rpm objects get automatically freed when out of scope
auto pkt_deleter = [](uint8_t * pkt) { free(pkt); };
auto ts_deleter = [](rpmts_s * ts) { rpmtsFree(ts); };

static RpmTransactionPtr create_transaction(const BaseWeakPtr & base) {
    RpmTransactionPtr ts_ptr(rpmtsCreate(), ts_deleter);
    auto & config = base->get_config();
    auto root_dir = config.installroot().get_value();
    if (rpmtsSetRootDir(ts_ptr.get(), root_dir.c_str()) != 0) {
        throw SignatureCheckError(M_("Failed to set rpm transaction rootDir \"{}\"."), std::string(root_dir));
    }
    return ts_ptr;
}

static bool rpmdb_lookup(const RpmTransactionPtr & ts_ptr, const KeyInfo & key) {
    bool retval{false};
    Header h;
    rpmdbMatchIterator mi;
    mi = rpmtsInitIterator(ts_ptr.get(), RPMDBI_NAME, "gpg-pubkey", 0);
    auto key_id = key.get_short_key_id();
    while ((h = rpmdbNextIterator(mi)) != nullptr) {
        if (headerGetAsString(h, RPMTAG_VERSION) == key_id) {
            retval = true;
            break;
        }
    }
    rpmdbFreeIterator(mi);
    return retval;
}


KeyInfo::KeyInfo(
    const std::string & key_url,
    const std::string & key_path,
    const std::string & key_id,
    const std::vector<std::string> & user_ids,
    const std::string & fingerprint,
    std::string raw_key)
    : key_url(key_url),
      key_path(key_path),
      key_id(key_id),
      user_ids(user_ids),
      fingerprint(fingerprint),
      raw_key(raw_key) {}

std::string KeyInfo::get_short_key_id() const {
    auto short_key_id = key_id.size() > 8 ? key_id.substr(key_id.size() - 8) : key_id;
    return short_key_id;
}

RpmSignature::CheckResult RpmSignature::check_package_signature(rpm::Package pkg) const {
    // is package gpg check even required?
    auto repo = pkg.get_repo();
    if (repo->get_type() == libdnf::repo::Repo::Type::COMMANDLINE) {
        if (!base->get_config().localpkg_gpgcheck().get_value()) {
            return CheckResult::OK;
        }
    } else {
        auto & repo_config = repo->get_config();
        if (!repo_config.gpgcheck().get_value()) {
            return CheckResult::OK;
        }
    }

    // rpmcliVerifySignatures is the only API rpm provides for signature verification.
    // Unfortunatelly to distinguish key_missing/not_signed/verification_failed cases
    // we need to temporarily increase log level to RPMLOG_INFO, collect the log
    // messages and parse them.
    // This code is only slightly better than running `rpmkeys --checksig` tool
    // and parsing it's output :(

    // This guard acquires the rpm log mutex and collects all rpm log messages into
    // the vector of strings.
    libdnf::rpm::RpmLogGuardStrings rpm_log_guard;

    auto ts_ptr = create_transaction(base);
    auto oldmask = rpmlogSetMask(RPMLOG_UPTO(RPMLOG_PRI(RPMLOG_INFO)));

    rpmtsSetVfyLevel(ts_ptr.get(), RPMSIG_SIGNATURE_TYPE);
    std::string path = pkg.get_package_path();
    char * const path_array[2] = {&path[0], NULL};
    auto rc = rpmcliVerifySignatures(ts_ptr.get(), path_array);

    rpmlogSetMask(oldmask);

    if (rc == RPMRC_OK) {
        return CheckResult::OK;
    }

    // This is brittle and heavily depends on rpm not changing log messages.
    // Here is an example of log messages after verifying a signed package
    // but without public key present in rpmdb:
    //   /path/to/rpm/dummy-signed-1.0.1-0.x86_64.rpm:
    //       Header V4 EdDSA/SHA512 Signature, key ID 773dd1ba: NOKEY
    //       Header RSA signature: NOTFOUND
    //       Header SHA256 digest: OK
    //       Header SHA1 digest: OK
    //       Payload SHA256 digest: OK
    //       RSA signature: NOTFOUND
    //       DSA signature: NOTFOUND
    //       MD5 digest: OK
    bool missing_key{false};
    bool not_trusted{false};
    bool not_signed{false};
    for (const auto & line : rpm_log_guard.get_rpm_logs()) {
        std::string_view line_v{line};
        if (line_v.starts_with(path)) {
            continue;
        }
        if (line.find(": BAD") != std::string::npos) {
            return CheckResult::FAILED;
        }
        if (line_v.ends_with(": NOKEY")) {
            missing_key = true;
        } else if (line_v.ends_with(": NOTTRUSTED")) {
            not_trusted = true;
        } else if (line_v.ends_with(": NOTFOUND")) {
            not_signed = true;
        } else if (!line_v.ends_with(": OK")) {
            return CheckResult::FAILED;
        }
    }
    if (not_trusted) {
        return CheckResult::FAILED_NOT_TRUSTED;
    } else if (missing_key) {
        return CheckResult::FAILED_KEY_MISSING;
    } else if (not_signed) {
        return CheckResult::FAILED_NOT_SIGNED;
    }
    return CheckResult::FAILED;
}

bool RpmSignature::key_present(const KeyInfo & key) const {
    libdnf::rpm::RpmLogGuard rpm_log_guard{base};
    auto ts_ptr = create_transaction(base);
    return rpmdb_lookup(ts_ptr, key);
}

bool RpmSignature::import_key(const KeyInfo & key) const {
    libdnf::rpm::RpmLogGuard rpm_log_guard{base};

    auto ts_ptr = create_transaction(base);
    if (!rpmdb_lookup(ts_ptr, key)) {
        uint8_t * pkt = nullptr;
        size_t pkt_len{0};
        if (pgpParsePkts(key.raw_key.c_str(), &pkt, &pkt_len) != PGPARMOR_PUBKEY) {
            free(pkt);
            throw KeyImportError(M_("\"{}\": key is not an armored public key."), key.get_url());
        }
        auto pkt_ptr = RpmKeyPktPtr{pkt, pkt_deleter};
        if (rpmtsImportPubkey(ts_ptr.get(), pkt_ptr.get(), pkt_len) != RPMRC_OK) {
            throw KeyImportError(M_("Failed to import public key \"{}\" to rpmdb."), key.get_url());
        }
        return true;
    }
    return false;
}

std::vector<KeyInfo> RpmSignature::parse_key_file(const std::string & key_url) {
    std::string key_path;
    std::unique_ptr<utils::fs::TempFile> downloaded_key;
    if (utils::url::is_url(key_url)) {
        if (key_url.starts_with("file://")) {
            key_path = key_url.substr(7);
        } else {
            // download the remote key
            downloaded_key = std::make_unique<libdnf::utils::fs::TempFile>("rpmkey");
            libdnf::repo::FileDownloader downloader(base);
            downloader.add(key_url, downloaded_key->get_path());
            downloader.download(true, true);
            key_path = downloaded_key->get_path();
        }
    } else {
        key_path = key_url;
    }

    std::vector<KeyInfo> keys;
    utils::fs::File key_file(key_path, "r");
    for (auto & key_info : repo::RepoPgp::rawkey2infos(key_file.get_fd())) {
        KeyInfo key{
            key_url,
            key_path,
            key_info.get_id(),
            key_info.get_user_ids(),
            key_info.get_fingerprint(),
            key_info.get_raw_key()};
        keys.emplace_back(std::move(key));
    }

    return keys;
}

}  //  namespace libdnf::rpm
