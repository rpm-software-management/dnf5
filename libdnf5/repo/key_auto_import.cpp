// Copyright Contributors to the DNF5 project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "key_auto_import.hpp"

#include "conf/config.h"
#include "utils/string.hpp"

#include <system_error>
#include <vector>


namespace libdnf5::repo {

namespace {

// Returns the file a "file:///path" URL points to, or an empty path for any
// other spelling. This is the one local spelling both key readers (RepoSack
// and RpmSignature) open as the same file. "file:/path" and a bare path are
// read differently by the two, so they are left to the confirmation.
std::filesystem::path local_key_path(const std::string & key_url) {
    if (key_url.starts_with("file:///")) {
        return key_url.substr(7);
    }
    return {};
}

// True when `path` has `dir` as a proper prefix in path components, so
// "/etc/pki/rpm-gpg-other/key" is not inside "/etc/pki/rpm-gpg".
bool is_inside_directory(const std::filesystem::path & path, const std::filesystem::path & dir) {
    const auto relative = path.lexically_relative(dir);
    if (relative.empty() || relative == ".") {
        return false;
    }
    return *relative.begin() != "..";
}

}  // namespace


std::vector<std::filesystem::path> parse_keys_dirs(const std::string & dirs) {
    std::vector<std::filesystem::path> result;
    for (const auto & entry : utils::string::split(dirs, ":")) {
        // An empty entry comes from a leading, trailing or doubled colon. A
        // relative entry would be resolved against the working directory of
        // the caller, which is not a place to trust keys from. Both are skipped.
        std::filesystem::path dir{entry};
        if (dir.is_absolute()) {
            result.push_back(std::move(dir));
        }
    }
    return result;
}


const std::vector<std::filesystem::path> & get_auto_import_keys_dirs() {
    static const std::vector<std::filesystem::path> keys_dirs = parse_keys_dirs(LIBDNF5_AUTO_IMPORT_KEYS_DIRS);
    return keys_dirs;
}


bool is_key_auto_importable(
    const ConfigRepo & config, const std::string & key_url, const std::vector<std::filesystem::path> & keys_dirs) {
    if (!config.get_auto_import_local_keys_option().get_value()) {
        return false;
    }
    if (config.get_main_config().get_assumeno_option().get_value()) {
        return false;
    }

    const auto key_path = local_key_path(key_url);
    if (key_path.empty()) {
        return false;
    }

    // Resolve symlinks and ".." on both sides. The strict canonical() is used
    // on purpose: a key that does not resolve (for example, it does not exist)
    // is never trusted automatically.
    std::error_code ec;
    const auto canonical_key = std::filesystem::canonical(key_path, ec);
    if (ec) {
        return false;
    }

    for (const auto & keys_dir : keys_dirs) {
        // A directory that cannot be resolved (for example, it does not exist
        // on this host) cannot hold the key. It is skipped so the other
        // directories are still checked.
        std::error_code dir_ec;
        const auto canonical_dir = std::filesystem::canonical(keys_dir, dir_ec);
        if (dir_ec) {
            continue;
        }
        if (is_inside_directory(canonical_key, canonical_dir)) {
            return true;
        }
    }
    return false;
}


void log_auto_import(
    Logger & logger, const rpm::KeyInfo & key_info, const std::string & key_url, const std::string & repo_id) {
    logger.info(
        "Automatically importing OpenPGP key 0x{} (fingerprint {}) from {} for repository {}.",
        key_info.get_key_id(),
        key_info.get_fingerprint(),
        key_url,
        repo_id);
}

}  // namespace libdnf5::repo
