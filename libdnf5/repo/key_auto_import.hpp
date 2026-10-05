// Copyright Contributors to the DNF5 project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef LIBDNF5_REPO_KEY_AUTO_IMPORT_HPP
#define LIBDNF5_REPO_KEY_AUTO_IMPORT_HPP

#include "libdnf5/logger/logger.hpp"
#include "libdnf5/repo/config_repo.hpp"
#include "libdnf5/rpm/rpm_signature.hpp"

#include <filesystem>
#include <string>


namespace libdnf5::repo {

/// Directory of OpenPGP keys that auto_import_local_keys imports without a
/// confirmation. Set at build time with the AUTO_IMPORT_KEYS_DIR CMake option.
const std::filesystem::path & get_auto_import_keys_dir();

/// Decide whether the OpenPGP key at `key_url` may be imported without asking
/// for a confirmation. True only when all of the following hold:
///  - auto_import_local_keys is enabled for the repository,
///  - assumeno is not set (assumeno never changes the system),
///  - `key_url` is spelled "file:///path", the one local spelling both key
///    readers (RepoSack and RpmSignature) open as the same file, and
///  - the canonical path of the key is inside `keys_dir`, so "..", symlinks
///    that leave the directory, and sibling directories with a shared prefix
///    are rejected.
/// Any key that is not accepted goes through the usual confirmation callback.
bool is_key_auto_importable(
    const ConfigRepo & config,
    const std::string & key_url,
    const std::filesystem::path & keys_dir = get_auto_import_keys_dir());

/// Log an import allowed by is_key_auto_importable() with the data needed to
/// audit it: key id, fingerprint, source URL and repository id.
void log_auto_import(
    Logger & logger, const rpm::KeyInfo & key_info, const std::string & key_url, const std::string & repo_id);

}  // namespace libdnf5::repo

#endif  // LIBDNF5_REPO_KEY_AUTO_IMPORT_HPP
