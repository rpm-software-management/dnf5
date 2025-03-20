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

#include "utils/string.hpp"

#include "libdnf5/utils/fs/temp.hpp"

#include <libdnf5/base/base.hpp>
#include <libdnf5/base/transaction.hpp>
#include <libdnf5/common/message.hpp>
#include <libdnf5/plugin/iplugin.hpp>
#include <libdnf5/rpm/rpm_signature.hpp>
#include <libdnf5/utils/bgettext/bgettext-mark-domain.h>
#include <libdnf5/utils/format_locale.hpp>
#include <rpm/header.h>
#include <rpm/rpmdb.h>
#include <rpm/rpmts.h>
#include <string.h>

#include <chrono>
#include <cstdio>
#include <memory>
#include <string>

using namespace libdnf5;

namespace {

constexpr const char * PLUGIN_NAME = "expired-pgp-keys";
constexpr plugin::Version PLUGIN_VERSION{1, 0, 0};
constexpr PluginAPIVersion REQUIRED_PLUGIN_API_VERSION{.major = 2, .minor = 1};

constexpr const char * attrs[]{"author.name", "author.email", "description", nullptr};
constexpr const char * attrs_value[]{"Jan Kolarik", "jkolarik@redhat.com", "Expired PGP Keys Plugin."};

/// @brief Find expired PGP keys and suggest their removal.
///        This is a workaround to solve https://github.com/rpm-software-management/dnf5/issues/1192.
class ExpiredPgpKeys final : public plugin::IPlugin2_1 {
public:
    ExpiredPgpKeys(libdnf5::plugin::IPluginData & data, libdnf5::ConfigParser &) : IPlugin2_1(data) {}
    virtual ~ExpiredPgpKeys() = default;

    PluginAPIVersion get_api_version() const noexcept override { return REQUIRED_PLUGIN_API_VERSION; }

    const char * get_name() const noexcept override { return PLUGIN_NAME; }

    plugin::Version get_version() const noexcept override { return PLUGIN_VERSION; }

    const char * const * get_attributes() const noexcept override { return attrs; }

    const char * get_attribute(const char * attribute) const noexcept override {
        for (size_t i = 0; attrs[i]; ++i) {
            if (std::strcmp(attribute, attrs[i]) == 0) {
                return attrs_value[i];
            }
        }
        return nullptr;
    }

    void goal_resolved(const libdnf5::base::Transaction & transaction) override {
        process_expired_pgp_keys(transaction);
    }

private:
    void process_expired_pgp_keys(const libdnf5::base::Transaction & transaction) const;
};

class ExpiryInfoMessage : public libdnf5::Message {
public:
    ExpiryInfoMessage(int64_t expiration_timestamp) : expiration_timestamp(expiration_timestamp) {}

    std::string format(bool translate, const libdnf5::utils::Locale * locale) const override {
        return libdnf5::utils::format(
            locale, translate, M_("Expired on {}"), 1, libdnf5::utils::string::format_epoch(expiration_timestamp));
    }

private:
    int64_t expiration_timestamp;
};

/// Check if the transaction contains any inbound actions.
/// This determines if new software is to be installed, which might require downloading a new PGP signing key.
static bool any_inbound_action(const libdnf5::base::Transaction & transaction) {
    for (const auto & package : transaction.get_transaction_packages()) {
        if (transaction_item_action_is_inbound(package.get_action())) {
            return true;
        }
    }
    return false;
}

/// Retrieve the PGP key expiration timestamp, or return -1 if the expiration is not available.
static int64_t get_key_expire_timestamp(std::string raw_key) {
    // open gpg process to retrieve information about the key
    std::unique_ptr<FILE, int (*)(FILE *)> pipe(
        popen(libdnf5::utils::sformat("echo \"{}\" | gpg --show-keys --with-colon", raw_key).c_str(), "r"), pclose);
    if (!pipe) {
        return -1;
    }

    // read key information from the gpg process
    char buffer[1024];
    std::string output;
    while (fgets(buffer, sizeof(buffer), pipe.get()) != nullptr) {
        output += buffer;
    }

    // check expired time is a numeric value
    auto lines = libdnf5::utils::string::split(output, "\n");
    if (lines.empty()) {
        return -1;
    }
    auto fields = libdnf5::utils::string::split(lines.front(), ":");
    if (fields.size() <= 6) {
        return -1;
    }
    auto expired_date_string = fields[6];
    if (expired_date_string.empty() || expired_date_string.find_first_not_of("0123456789") != std::string::npos) {
        return -1;
    }

    return static_cast<int64_t>(std::stoull(expired_date_string));
}

void ExpiredPgpKeys::process_expired_pgp_keys(const libdnf5::base::Transaction & transaction) const {
    auto & logger = *get_base().get_logger();
    const auto & config = get_base().get_config();

    if (!config.get_pkg_gpgcheck_option().get_value()) {
        return;
    }

    if (!any_inbound_action(transaction)) {
        return;
    }

    auto current_date = std::chrono::system_clock::now();
    auto current_timestamp = std::chrono::duration_cast<std::chrono::seconds>(current_date.time_since_epoch()).count();

    libdnf5::rpm::RpmSignature rpm_signature(get_base());

    // Obtain callbacks for getting confirmation from a user.
    // TODO: Where to get callbacks without repositories? E.g. if all are
    // disabled? For now use calbacks of the first repository. The callbacks
    // API should become independent from libdnf5::repo.
    libdnf5::repo::RepoQuery enabled_repos(get_base());
    enabled_repos.filter_enabled(true);
    enabled_repos.filter_type(libdnf5::repo::Repo::Type::AVAILABLE);
    libdnf5::repo::RepoCallbacks2_1 * callbacks = nullptr;
    for (auto const & repo : enabled_repos) {
        callbacks = dynamic_cast<libdnf5::repo::RepoCallbacks2_1 *>(repo->get_callbacks().get());
        break;
    }

    // Iterate over all installed OpenPGP keys.
    auto ts = rpmtsCreate();
    auto root_dir = config.get_installroot_option().get_value();
    if (rpmtsSetRootDir(ts, root_dir.c_str()) != 0) {
        logger.error("Expired PGP Keys Plugin: Failed to set rpm transaction root dir \"{}\".", root_dir);
        rpmtsFree(ts);
        return;
    }
    Header h;
    rpmdbMatchIterator mi = rpmtsInitIterator(ts, RPMDBI_NAME, "gpg-pubkey", 0);
    std::vector<libdnf5::rpm::KeyInfo> keys_to_remove;

    while ((h = rpmdbNextIterator(mi)) != nullptr) {
        char * raw_key = headerGetAsString(h, RPMTAG_DESCRIPTION);
        if (!raw_key)
            continue;

        // Serialize raw key into a file becuse parse_key_file() requires a file.
        // (Or use lr_gpg_import_key_from_memory() directly?)
        auto key_tfile = libdnf5::utils::fs::TempFile("key");
        auto & key_ffile = key_tfile.open_as_file("w+");
        key_ffile.write(raw_key, strlen(raw_key));
        key_ffile.flush();
        free(raw_key);

        // Parse the serialized key into a vector of KeyInfo objects (first
        // signing subkey).
        // Since RPM database stores each primary key with all its subkeys as
        // a single gpg-pubkey package, parse_key_file() method always returns
        // a vector of at most one KeyInfo object. The vector is empty if the
        // primary key had no signing subkey.
        // XXX: That effectivelly ignores expiration time of other subkeys.
        auto parsed_keys = rpm_signature.parse_key_file("file://" + key_tfile.get_path().string());
        if (parsed_keys.empty())
            continue;

        // Check expiration time of the only subkey.
        auto & key_info = parsed_keys.front();
        auto key_timestamp = get_key_expire_timestamp(key_info.get_raw_key());
        if (key_timestamp > 0 && key_timestamp < current_timestamp) {
            if (callbacks && !callbacks->repokey_remove(key_info, ExpiryInfoMessage(key_timestamp))) {
                // User declined removing this key.
                continue;
            }
            if (rpmtsAddEraseElement(ts, h, -1) != 0) {
                logger.error(
                    "Expired PGP Keys Plugin: Failed to mark 0x{} key for removal.", key_info.get_short_key_id());
            } else {
                keys_to_remove.emplace_back(key_info);
            }
        }
    }

    if (!keys_to_remove.empty()) {
        if (rpmtsRun(ts, nullptr, RPMPROB_FILTER_NONE)) {
            for (auto & key_info : keys_to_remove) {
                logger.error("Expired PGP Keys Plugin: Failed to remove the 0x{} key.", key_info.get_short_key_id());
            }
        } else {
            for (auto & key_info : keys_to_remove) {
                logger.debug("Expired PGP Keys Plugin: 0x{} key removed.", key_info.get_short_key_id());
                if (callbacks) {
                    callbacks->repokey_removed(key_info);
                }
            }
        }
    }

    rpmdbFreeIterator(mi);
    rpmtsFree(ts);
}


std::exception_ptr last_exception;

}  // namespace

PluginAPIVersion libdnf_plugin_get_api_version(void) {
    return REQUIRED_PLUGIN_API_VERSION;
}

const char * libdnf_plugin_get_name(void) {
    return PLUGIN_NAME;
}

plugin::Version libdnf_plugin_get_version(void) {
    return PLUGIN_VERSION;
}

plugin::IPlugin * libdnf_plugin_new_instance(
    [[maybe_unused]] LibraryVersion library_version,
    libdnf5::plugin::IPluginData & data,
    libdnf5::ConfigParser & parser) try {
    return new ExpiredPgpKeys(data, parser);
} catch (...) {
    last_exception = std::current_exception();
    return nullptr;
}

void libdnf_plugin_delete_instance(plugin::IPlugin * plugin_object) {
    delete plugin_object;
}

std::exception_ptr * libdnf_plugin_get_last_exception(void) {
    return &last_exception;
}
