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

#ifndef LIBDNF5_RPM_RPM_SIGNATURE_HPP
#define LIBDNF5_RPM_RPM_SIGNATURE_HPP

#include "libdnf5/base/base.hpp"
#include "libdnf5/common/exception.hpp"
#include "libdnf5/rpm/package.hpp"

#include <functional>
#include <string>

namespace libdnf5::rpm {

class SignatureCheckError : public Error {
public:
    using Error::Error;
    const char * get_domain_name() const noexcept override { return "libdnf5::rpm"; }
    const char * get_name() const noexcept override { return "SignatureCheckError"; }
};

class KeyImportError : public Error {
public:
    using Error::Error;
    const char * get_domain_name() const noexcept override { return "libdnf5::rpm"; }
    const char * get_name() const noexcept override { return "KeyImportError"; }
};

using RpmKeyPktPtr = std::unique_ptr<uint8_t, std::function<void(uint8_t * pkt)>>;

class KeyInfo {
public:
    const std::string & get_key_id() const noexcept;
    std::string get_short_key_id() const;
    const std::vector<std::string> & get_user_ids() const noexcept;
    const std::string & get_fingerprint() const noexcept;
    const std::string & get_url() const noexcept;
    const std::string & get_path() const noexcept;
    const std::string & get_raw_key() const noexcept;
    const long int & get_timestamp() const noexcept;

    KeyInfo(
        const std::string & key_url,
        const std::string & key_path,
        const std::string & key_id,
        const std::vector<std::string> & user_ids,
        const std::string & fingerprint,
        long int timestamp,
        const std::string & raw_key);

    ~KeyInfo();

    KeyInfo(const KeyInfo & src);
    KeyInfo & operator=(const KeyInfo & src);

    KeyInfo(KeyInfo && src) noexcept;
    KeyInfo & operator=(KeyInfo && src) noexcept;

protected:
    void add_user_id(const char * user_id);

private:
    class Impl;
    std::unique_ptr<Impl> p_impl;
};

class RpmSignature {
public:
    enum class CheckResult { OK, SKIPPED, FAILED_KEY_MISSING, FAILED_NOT_TRUSTED, FAILED_NOT_SIGNED, FAILED };

    explicit RpmSignature(const libdnf5::BaseWeakPtr & base);
    explicit RpmSignature(Base & base);
    ~RpmSignature();
    RpmSignature(const RpmSignature & src);
    RpmSignature & operator=(const RpmSignature & src);

    RpmSignature(RpmSignature && src) noexcept;
    RpmSignature & operator=(RpmSignature && src) noexcept;

    /// Check signature of the `package` using public keys stored in rpm database.
    /// @param package: package to check.
    /// @return CheckResult::OK - the check passed
    ///         CheckResult::SKIPPED - the check was skipped
    ///         CheckResult::FAILED_KEY_MISSING - no corresponding key found in rpmdb
    ///         CheckResult::FAILED_NOT_TRUSTED - signature is valid but the key is not trusted
    ///         CheckResult::FAILED_NOT_SIGNED - package is not signed but signature is required
    ///         CheckResult::FAILED - check failed for another reason
    CheckResult check_package_signature(const Package & pkg) const;

    /// Import public key into rpm database.
    /// @param key: GPG key to be imported into rpm database.
    bool import_key(const KeyInfo & key) const;

    /// Check public key presence in rpm database
    bool key_present(const KeyInfo & key) const;

    /// Download the key file if needed and return a vector of keys contained in it
    /// @param key_url: URL of the public key to be imported.
    std::vector<KeyInfo> parse_key_file(const std::string & key_url);

    /// Return string representation of the CheckResult enum
    static std::string check_result_to_string(CheckResult result);

private:
    class Impl;
    std::unique_ptr<Impl> p_impl;
};

}  // namespace libdnf5::rpm

#endif  // LIBDNF5_RPM_RPM_SIGNATURE_HPP
