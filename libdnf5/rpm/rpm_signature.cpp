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

#include "libdnf5/rpm/rpm_signature.hpp"

#include "repo/repo_pgp.hpp"
#include "rpm/rpm_log_guard.hpp"
#include "utils/string.hpp"
#include "utils/url.hpp"

#include "libdnf5/repo/file_downloader.hpp"
#include "libdnf5/repo/repo.hpp"
#include "libdnf5/utils/bgettext/bgettext-lib.h"
#include "libdnf5/utils/bgettext/bgettext-mark-domain.h"
#include "libdnf5/utils/fs/temp.hpp"

#include <rpm/rpmcli.h>
#include <rpm/rpmdb.h>
#include <rpm/rpmkeyring.h>
#include <rpm/rpmlog.h>
#include <rpm/rpmpgp.h>
#include <rpm/rpmts.h>

namespace libdnf5::rpm {

namespace {

const std::map<rpm::RpmSignature::CheckResult, BgettextMessage> RPM_SIGNATURE_CHECK_RESULT_DICT = {
    {rpm::RpmSignature::CheckResult::FAILED_KEY_MISSING, M_("No corresponding key was found.")},
    {rpm::RpmSignature::CheckResult::FAILED_NOT_TRUSTED, M_("The signature is valid, but the key is not trusted.")},
    {rpm::RpmSignature::CheckResult::FAILED_NOT_SIGNED, M_("The package is not signed.")},
    {rpm::RpmSignature::CheckResult::FAILED, M_("Problem occurred when opening the package.")},
};

}  // namespace

using RpmTransactionPtr = std::unique_ptr<rpmts_s, std::function<void(rpmts_s * ts)>>;
// deleters for unique_ptrs that take ownership of rpm objects
// once the ownership is transferred, rpm objects get automatically freed when out of scope
auto pkt_deleter = [](uint8_t * pkt) { free(pkt); };
auto ts_deleter = [](rpmts_s * ts) { rpmtsFree(ts); };

static RpmTransactionPtr create_transaction(const BaseWeakPtr & base) {
    RpmTransactionPtr ts_ptr(rpmtsCreate(), ts_deleter);
    auto & config = base->get_config();
    auto root_dir = config.get_installroot_option().get_value();
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

class KeyInfo::Impl {
public:
    Impl(
        std::string key_url,
        std::string key_path,
        std::string key_id,
        std::vector<std::string> user_ids,
        std::string fingerprint,
        long int timestamp,
        std::string raw_key);

private:
    friend KeyInfo;
    std::string key_url;
    std::string key_path;
    std::string key_id;
    std::vector<std::string> user_ids;
    std::string fingerprint;
    long int timestamp;
    std::string raw_key;
};

KeyInfo::Impl::Impl(
    std::string key_url,
    std::string key_path,
    std::string key_id,
    std::vector<std::string> user_ids,
    std::string fingerprint,
    long int timestamp,
    std::string raw_key)
    : key_url(std::move(key_url)),
      key_path(std::move(key_path)),
      key_id(std::move(key_id)),
      user_ids(std::move(user_ids)),
      fingerprint(std::move(fingerprint)),
      timestamp(timestamp),
      raw_key(std::move(raw_key)) {}

KeyInfo::KeyInfo(
    const std::string & key_url,
    const std::string & key_path,
    const std::string & key_id,
    const std::vector<std::string> & user_ids,
    const std::string & fingerprint,
    long int timestamp,
    const std::string & raw_key)
    : p_impl(std::make_unique<Impl>(key_url, key_path, key_id, user_ids, fingerprint, timestamp, raw_key)) {}

KeyInfo::~KeyInfo() = default;

KeyInfo::KeyInfo(const KeyInfo & src) : p_impl(new Impl(*src.p_impl)) {}
KeyInfo::KeyInfo(KeyInfo && src) noexcept = default;

KeyInfo & KeyInfo::operator=(const KeyInfo & src) {
    if (this != &src) {
        if (p_impl) {
            *p_impl = *src.p_impl;
        } else {
            p_impl = std::make_unique<Impl>(*src.p_impl);
        }
    }

    return *this;
}
KeyInfo & KeyInfo::operator=(KeyInfo && src) noexcept = default;

std::string KeyInfo::get_short_key_id() const {
    auto short_key_id = p_impl->key_id.size() > 8 ? p_impl->key_id.substr(p_impl->key_id.size() - 8) : p_impl->key_id;
    return short_key_id;
}

class RpmSignature::Impl {
public:
    explicit Impl(const libdnf5::BaseWeakPtr & base) : base(base) {}

private:
    friend RpmSignature;
    BaseWeakPtr base;
};

RpmSignature::RpmSignature(const libdnf5::BaseWeakPtr & base) : p_impl(std::make_unique<Impl>(base)) {}
RpmSignature::RpmSignature(Base & base) : RpmSignature(base.get_weak_ptr()) {}
RpmSignature::~RpmSignature() = default;

RpmSignature::RpmSignature(const RpmSignature & src) : p_impl(new Impl(*src.p_impl)) {}
RpmSignature::RpmSignature(RpmSignature && src) noexcept = default;

RpmSignature & RpmSignature::operator=(const RpmSignature & src) {
    if (this != &src) {
        if (p_impl) {
            *p_impl = *src.p_impl;
        } else {
            p_impl = std::make_unique<Impl>(*src.p_impl);
        }
    }

    return *this;
}
RpmSignature & RpmSignature::operator=(RpmSignature && src) noexcept = default;


RpmSignature::CheckResult RpmSignature::check_package_signature(const rpm::Package & pkg) const {
    // is package gpg check even required?
    auto repo = pkg.get_repo();
    if (repo->get_type() == libdnf5::repo::Repo::Type::COMMANDLINE) {
        if (!p_impl->base->get_config().get_localpkg_gpgcheck_option().get_value()) {
            return CheckResult::SKIPPED;
        }
    } else {
        auto & repo_config = repo->get_config();
        if (!repo_config.get_gpgcheck_option().get_value()) {
            return CheckResult::SKIPPED;
        }
    }

    // rpmcliVerifySignatures is the only API rpm provides for signature verification.
    // Unfortunately to distinguish key_missing/not_signed/verification_failed cases
    // we need to temporarily increase log level to RPMLOG_INFO, collect the log
    // messages and parse them.
    // This code is only slightly better than running `rpmkeys --checksig` tool
    // and parsing it's output :(

    // This guard acquires the rpm log mutex and collects all rpm log messages into
    // the vector of strings.
    libdnf5::rpm::RpmLogGuardStrings rpm_log_guard;

    auto ts_ptr = create_transaction(p_impl->base);
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
    libdnf5::rpm::RpmLogGuard rpm_log_guard{p_impl->base};
    auto ts_ptr = create_transaction(p_impl->base);
    return rpmdb_lookup(ts_ptr, key);
}

bool RpmSignature::import_key(const KeyInfo & key) const {
    libdnf5::rpm::RpmLogGuardStrings rpm_log_guard;

    auto ts_ptr = create_transaction(p_impl->base);
    if (!rpmdb_lookup(ts_ptr, key)) {
        uint8_t * pkt = nullptr;
        size_t pkt_len{0};
        if (pgpParsePkts(key.get_raw_key().c_str(), &pkt, &pkt_len) != PGPARMOR_PUBKEY) {
            free(pkt);
            throw KeyImportError(M_("\"{}\": key is not an armored public key."), key.get_url());
        }
        auto pkt_ptr = RpmKeyPktPtr{pkt, pkt_deleter};
        if (rpmtsImportPubkey(ts_ptr.get(), pkt_ptr.get(), pkt_len) != RPMRC_OK) {
            auto rpm_logs = utils::string::join(rpm_log_guard.get_rpm_logs(), "\n");
            throw KeyImportError(M_("Failed to import public key \"{}\" to rpmdb: {}"), key.get_url(), rpm_logs);
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
            downloaded_key = std::make_unique<libdnf5::utils::fs::TempFile>("rpmkey");
            libdnf5::repo::FileDownloader downloader(p_impl->base);
            downloader.add(key_url, downloaded_key->get_path());
            downloader.download();
            key_path = downloaded_key->get_path();
        }
    } else {
        key_path = key_url;
    }

    std::vector<KeyInfo> keys;
    utils::fs::File key_file(key_path, "r");
    for (auto & key_info : repo::RepoPgp::rawkey2infos(key_file.get_fd(), key_url, key_path)) {
        keys.emplace_back(key_info);
    }

    return keys;
}

std::string RpmSignature::check_result_to_string(CheckResult result) {
    switch (result) {
        case CheckResult::OK:
        case CheckResult::SKIPPED:
            return {};
        case CheckResult::FAILED_KEY_MISSING:
        case CheckResult::FAILED_NOT_TRUSTED:
        case CheckResult::FAILED_NOT_SIGNED:
        case CheckResult::FAILED:
            return TM_(RPM_SIGNATURE_CHECK_RESULT_DICT.at(result), 1);
    }
    return {};
}

void KeyInfo::add_user_id(const char * user_id) {
    p_impl->user_ids.emplace_back(user_id);
}

const std::string & KeyInfo::get_key_id() const noexcept {
    return p_impl->key_id;
}
const std::vector<std::string> & KeyInfo::get_user_ids() const noexcept {
    return p_impl->user_ids;
}
const std::string & KeyInfo::get_fingerprint() const noexcept {
    return p_impl->fingerprint;
}
const std::string & KeyInfo::get_url() const noexcept {
    return p_impl->key_url;
}
const std::string & KeyInfo::get_path() const noexcept {
    return p_impl->key_path;
}
const std::string & KeyInfo::get_raw_key() const noexcept {
    return p_impl->raw_key;
}
const long int & KeyInfo::get_timestamp() const noexcept {
    return p_impl->timestamp;
}

}  // namespace libdnf5::rpm
