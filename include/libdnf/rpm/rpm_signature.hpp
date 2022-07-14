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

#ifndef LIBDNF_RPM_RPM_SIGNATURE_HPP
#define LIBDNF_RPM_RPM_SIGNATURE_HPP

#include "libdnf/base/base.hpp"
#include "libdnf/common/exception.hpp"
#include "libdnf/rpm/package.hpp"

#include <functional>
#include <string>

namespace libdnf::rpm {

class SignatureCheckError : public Error {
public:
    using Error::Error;
    const char * get_domain_name() const noexcept override { return "libdnf::rpm"; }
    const char * get_name() const noexcept override { return "SignatureCheckError"; }
};

class KeyImportError : public Error {
public:
    using Error::Error;
    const char * get_domain_name() const noexcept override { return "libdnf::rpm"; }
    const char * get_name() const noexcept override { return "KeyImportError"; }
};

using RpmKeyPktPtr = std::unique_ptr<uint8_t, std::function<void(uint8_t * pkt)>>;

class KeyInfo {
public:
    std::string get_key_id() const { return key_id; }
    std::string get_short_key_id() const;
    std::string get_user_id() const { return user_id; }
    std::string get_fingerprint() const { return fingerprint; }
    std::string get_url() const { return key_url; }
    std::string get_path() const { return key_path; }

private:
    friend class RpmSignature;
    KeyInfo(
        const std::string & key_url,
        const std::string & key_path,
        const std::string & key_id,
        const std::string & user_id,
        const std::string & fingerprint,
        std::vector<char> raw_key);
    std::string key_url;
    std::string key_path;
    std::string key_id;
    std::string user_id;
    std::string fingerprint;
    std::vector<char> raw_key;
};

class RpmSignature {
public:
    enum class CheckResult { OK, FAILED_KEY_MISSING, FAILED_NOT_TRUSTED, FAILED_NOT_SIGNED, FAILED };

    explicit RpmSignature(const BaseWeakPtr & base) : base(base) {}
    explicit RpmSignature(Base & base) : RpmSignature(base.get_weak_ptr()) {}
    ~RpmSignature(){};

    /// Check signature of the `package` using public keys stored in rpm database.
    /// @param package: package to check.
    /// @return CheckResult::OK - the check passed
    ///         CheckResult::FAILED_KEY_MISSING - no corresponding key found in rpmdb
    ///         CheckResult::FAILED_NOT_TRUSTED - signature is valid but the key is not trusted
    ///         CheckResult::FAILED_NOT_SIGNED - package is not signed but signature is required
    ///         CheckResult::FAILED - check failed for another reason
    CheckResult check_package_signature(Package package) const;

    /// Import public key into rpm database.
    /// @param key: GPG key to be imported into rpm database.
    bool import_key(const KeyInfo & key) const;

    /// Check public key presence in rpm database
    bool key_present(const KeyInfo & key) const;

    /// Download the key file if needed and return a vector of keys contained in it
    /// @param key_url: URL of the public key to be imported.
    std::vector<KeyInfo> parse_key_file(const std::string & key_url);

private:
    BaseWeakPtr base;
};

}  // namespace libdnf::rpm

#endif  // LIBDNF_RPM_RPM_SIGNATURE_HPP
