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

#include <libdnf5-cli/utils/userconfirm.hpp>
#include <libdnf5/base/base.hpp>
#include <libdnf5/base/transaction.hpp>
#include <libdnf5/plugin/iplugin.hpp>
#include <libdnf5/utils/bgettext/bgettext-lib.h>
#include <rpm/header.h>
#include <rpm/rpmdb.h>
#include <rpm/rpmts.h>
#include <string.h>

#include <chrono>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

using namespace libdnf5;

namespace {

constexpr const char * PLUGIN_NAME = "expired-pgp-keys";
constexpr plugin::Version PLUGIN_VERSION{1, 0, 0};

constexpr const char * attrs[]{"author.name", "author.email", "description", nullptr};
constexpr const char * attrs_value[]{"Jan Kolarik", "jkolarik@redhat.com", "Expired PGP Keys Plugin."};

/// @brief Find expired PGP keys and suggest their removal.
///        This is a workaround to solve https://github.com/rpm-software-management/dnf5/issues/1192.
class ExpiredPgpKeys final : public plugin::IPlugin {
public:
    ExpiredPgpKeys(libdnf5::plugin::IPluginData & data, libdnf5::ConfigParser &) : IPlugin(data) {}
    virtual ~ExpiredPgpKeys() = default;

    PluginAPIVersion get_api_version() const noexcept override { return PLUGIN_API_VERSION; }

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

    void pre_transaction(const libdnf5::base::Transaction & transaction) override {
        process_expired_pgp_keys(transaction);
    }

private:
    void process_expired_pgp_keys(const libdnf5::base::Transaction & transaction) const;
};

/// Check that GPG is installed to enable querying expired keys later.
static bool is_gpg_installed() {
    auto ts = rpmtsCreate();
    rpmdbMatchIterator mi;
    mi = rpmtsInitIterator(ts, RPMDBI_PROVIDENAME, "gpg", 0);
    bool found = rpmdbNextIterator(mi) != NULL;
    rpmdbFreeIterator(mi);
    rpmtsFree(ts);
    return found;
}

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
static int64_t get_key_expire_timestamp(Header hdr) {
    // open gpg process to retrieve information about the key
    const char * key = headerGetString(hdr, RPMTAG_DESCRIPTION);
    std::unique_ptr<FILE, decltype(&pclose)> pipe(
        popen(libdnf5::utils::sformat("echo \"{}\" | gpg --show-keys --with-colon", key).c_str(), "r"), pclose);
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
    auto expired_date_string = libdnf5::utils::string::split(libdnf5::utils::string::split(output, "\n")[0], ":")[6];
    if (expired_date_string.empty() || expired_date_string.find_first_not_of("0123456789") != std::string::npos) {
        return -1;
    }

    return std::stoull(expired_date_string);
}

/// Returns a list of expired PGP keys, each represented as a tuple (`hdr`, `date`):
/// * `hdr`: An RPM header object representing the key.
/// * `date`: A `datetime` object indicating the key's expiration date.
static std::vector<std::pair<Header, int64_t>> list_expired_keys() {
    std::vector<std::pair<Header, int64_t>> expired_keys;

    auto current_date = std::chrono::system_clock::now();
    auto current_timestamp = std::chrono::duration_cast<std::chrono::seconds>(current_date.time_since_epoch()).count();

    auto ts = rpmtsCreate();
    rpmdbMatchIterator mi;
    mi = rpmtsInitIterator(ts, RPMDBI_NAME, "gpg-pubkey", 0);
    Header hdr;
    while ((hdr = rpmdbNextIterator(mi)) != nullptr) {
        auto key_timestamp = get_key_expire_timestamp(hdr);
        if (key_timestamp > 0 && key_timestamp < current_timestamp) {
            expired_keys.push_back({hdr, key_timestamp});
            headerLink(hdr);
        }
    }

    rpmdbFreeIterator(mi);
    rpmtsFree(ts);

    return expired_keys;
}

/// Remove the system package corresponding to the PGP key from the given RPM header.
static bool remove_pgp_key(Header hdr) {
    auto ts = rpmtsCreate();
    rpmtsAddEraseElement(ts, hdr, -1);
    bool result = rpmtsRun(ts, nullptr, RPMPROB_FILTER_NONE) == 0;
    rpmtsFree(ts);
    return result;
}

void ExpiredPgpKeys::process_expired_pgp_keys(const libdnf5::base::Transaction & transaction) const {
    const auto & config = get_base().get_config();

    if (!config.get_gpgcheck_option().get_value()) {
        return;
    }

    if (!is_gpg_installed()) {
        return;
    }

    if (!any_inbound_action(transaction)) {
        return;
    }

    for (auto & [hdr, expiration] : list_expired_keys()) {
        std::cout << libdnf5::utils::sformat(
                         _("The following PGP key has expired on {}:"),
                         libdnf5::utils::string::format_epoch(expiration))
                  << std::endl;
        std::cout << libdnf5::utils::sformat("    {}", headerGetString(hdr, RPMTAG_SUMMARY)) << std::endl;
        std::cout << _("For more information about the key:") << std::endl;
        std::cout << libdnf5::utils::sformat("    rpm -qi {}", headerGetAsString(hdr, RPMTAG_NVR)) << std::endl;

        std::cout << _("As a result, installing packages signed with this key will fail.") << std::endl;
        std::cout << _("It is recommended to remove the expired key to allow importing") << std::endl;
        std::cout << _("an updated key. This might leave already installed packages unverifiable.") << std::endl
                  << std::endl;

        std::cout << _("The system will now proceed with removing the key.") << std::endl;

        if (libdnf5::cli::utils::userconfirm::userconfirm(config)) {
            std::cout << std::endl;
            if (remove_pgp_key(hdr)) {
                std::cout << _("Key successfully removed.") << std::endl;
            } else {
                std::cout << _("Failed to remove the key.") << std::endl;
            }
            std::cout << std::endl;
        }

        headerFree(hdr);
    }
}

}  // namespace

PluginAPIVersion libdnf_plugin_get_api_version(void) {
    return PLUGIN_API_VERSION;
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
    return nullptr;
}

void libdnf_plugin_delete_instance(plugin::IPlugin * plugin_object) {
    delete plugin_object;
}
