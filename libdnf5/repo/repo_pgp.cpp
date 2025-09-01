// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 2.1 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.

#include "repo_pgp.hpp"

#include "libdnf5/base/base.hpp"
#include "libdnf5/repo/repo_errors.hpp"
#include "libdnf5/utils/bgettext/bgettext-mark-domain.h"
#include "libdnf5/utils/fs/temp.hpp"

#include <memory>

namespace libdnf5::repo {

Key::Key(const LrGpgKey * key, const LrGpgSubkey * subkey, const std::string & url, const std::string & path)
    : KeyInfo(
          url,
          path,
          lr_gpg_subkey_get_id(subkey),
          {},
          lr_gpg_subkey_get_fingerprint(subkey),
          lr_gpg_subkey_get_timestamp(subkey),
          lr_gpg_key_get_raw_key(key)) {
    for (auto * const * item = lr_gpg_key_get_userids(key); *item; ++item) {
        add_user_id(*item);
    }
}


[[noreturn]] static void throw_repo_pgp_error(const BgettextMessage & msg, GError * err) {
    std::string err_msg = err->message;
    g_error_free(err);
    throw RepoPgpError(msg, err_msg);
}


std::vector<Key> RepoPgp::rawkey2infos(int fd, const std::string & url, const std::string & path) {
    std::vector<Key> key_infos;

    libdnf5::utils::fs::TempDir tmpdir("tmpdir");

    GError * err = NULL;
    if (!lr_gpg_import_key_from_fd(fd, tmpdir.get_path().c_str(), &err)) {
        throw_repo_pgp_error(M_("Failed to import OpenPGP keys into temporary keyring: {}"), err);
    }

    std::unique_ptr<LrGpgKey, decltype(&lr_gpg_keys_free)> lr_keys{
        lr_gpg_list_keys(TRUE, tmpdir.get_path().c_str(), &err), &lr_gpg_keys_free};
    if (err) {
        throw_repo_pgp_error(M_("Failed to list OpenPGP keys: {}"), err);
    }

    for (const auto * lr_key = lr_keys.get(); lr_key; lr_key = lr_gpg_key_get_next(lr_key)) {
        for (const auto * lr_subkey = lr_gpg_key_get_subkeys(lr_key); lr_subkey;
             lr_subkey = lr_gpg_subkey_get_next(lr_subkey)) {
            // get first signing subkey
            if (lr_gpg_subkey_get_can_sign(lr_subkey)) {
                key_infos.emplace_back(lr_key, lr_subkey, url, path);
                break;
            }
        }
    }

    return key_infos;
}


std::vector<std::string> RepoPgp::load_keys_ids_from_keyring() {
    std::vector<std::string> keys_ids;

    auto keyring_dir = get_keyring_dir();
    if (std::filesystem::is_directory(keyring_dir)) {
        GError * err = NULL;
        std::unique_ptr<LrGpgKey, decltype(&lr_gpg_keys_free)> lr_keys{
            lr_gpg_list_keys(FALSE, keyring_dir.c_str(), &err), &lr_gpg_keys_free};
        if (err) {
            throw_repo_pgp_error(M_("Failed to list OpenPGP keys: {}"), err);
        }

        for (const auto * lr_key = lr_keys.get(); lr_key; lr_key = lr_gpg_key_get_next(lr_key)) {
            for (const auto * lr_subkey = lr_gpg_key_get_subkeys(lr_key); lr_subkey;
                 lr_subkey = lr_gpg_subkey_get_next(lr_subkey)) {
                // get first signing subkey
                if (lr_gpg_subkey_get_can_sign(lr_subkey)) {
                    keys_ids.emplace_back(lr_gpg_subkey_get_id(lr_subkey));
                    break;
                }
            }
        }
    }

    return keys_ids;
}


RepoPgp::RepoPgp(const BaseWeakPtr & base, const ConfigRepo & config) : base(base), config(config) {}


void RepoPgp::import_key(int fd, const std::string & url) {
    auto & logger = *base->get_logger();

    auto key_infos = rawkey2infos(fd, url);

    auto known_keys = load_keys_ids_from_keyring();
    for (auto & key_info : key_infos) {
        if (std::find(known_keys.begin(), known_keys.end(), key_info.get_key_id()) != known_keys.end()) {
            logger.debug(
                "OpenPGP key 0x{} for repository {} already imported.", key_info.get_key_id(), config.get_id());
            continue;
        }

        if (callbacks && !callbacks->repokey_import(key_info)) {
            continue;
        }

        auto keyring_dir = get_keyring_dir();

        if (!std::filesystem::is_directory(keyring_dir)) {
            std::filesystem::create_directories(keyring_dir);
        }

        GError * err = NULL;
        if (!lr_gpg_import_key_from_memory(
                key_info.get_raw_key().c_str(), key_info.get_raw_key().size(), keyring_dir.c_str(), &err)) {
            throw_repo_pgp_error(M_("Failed to import OpenPGP keys: {}"), err);
        }

        if (callbacks) {
            callbacks->repokey_imported(key_info);
        }
        logger.debug("Imported OpenPGP key 0x{} for repository {}.", key_info.get_key_id(), config.get_id());
    }
}

}  // namespace libdnf5::repo
