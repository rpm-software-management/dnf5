// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef LIBDNF5_REPO_REPO_PGP_HPP
#define LIBDNF5_REPO_REPO_PGP_HPP

#include "libdnf5/base/base_weak.hpp"
#include "libdnf5/common/exception.hpp"
#include "libdnf5/repo/config_repo.hpp"
#include "libdnf5/repo/repo_callbacks.hpp"
#include "libdnf5/rpm/rpm_signature.hpp"

#include <librepo/librepo.h>

#include <filesystem>
#include <string>
#include <vector>


namespace libdnf5::repo {

class Key : public libdnf5::rpm::KeyInfo {
public:
    Key(const LrGpgKey * key, const LrGpgSubkey * subkey, const std::string & url, const std::string & path);
};

/// Wraps OpenPGP in a higher-level interface.
/// @exception RepoPgpError (public) Thrown on any OpenPGP-related error.
class RepoPgp {
public:
    RepoPgp(const BaseWeakPtr & base, const ConfigRepo & config);

    void set_callbacks(RepoCallbacks * callbacks) noexcept { this->callbacks = callbacks; }

    std::filesystem::path get_keyring_dir() const { return std::filesystem::path(config.get_cachedir()) / "pubring"; }

    void import_key(int fd, const std::string & url);
    static std::vector<Key> rawkey2infos(int fd, const std::string & url, const std::string & path = "");

private:
    std::vector<std::string> load_keys_ids_from_keyring();
    BaseWeakPtr base;
    const ConfigRepo & config;
    RepoCallbacks * callbacks = nullptr;
};

}  // namespace libdnf5::repo

#endif  // LIBDNF5_REPO_REPO_PGP_HPP
