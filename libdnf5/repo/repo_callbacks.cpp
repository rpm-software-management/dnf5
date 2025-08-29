// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "libdnf5/repo/repo_callbacks.hpp"

namespace libdnf5::repo {

RepoCallbacks::RepoCallbacks() = default;

RepoCallbacks::~RepoCallbacks() = default;

bool RepoCallbacks::repokey_import(const libdnf5::rpm::KeyInfo &) {
    return true;
}

void RepoCallbacks::repokey_imported(const libdnf5::rpm::KeyInfo &) {}

RepoCallbacks2_1::RepoCallbacks2_1() = default;

RepoCallbacks2_1::~RepoCallbacks2_1() = default;

bool RepoCallbacks2_1::repokey_remove(const libdnf5::rpm::KeyInfo &, const libdnf5::Message &) {
    return true;
}

void RepoCallbacks2_1::repokey_removed(const libdnf5::rpm::KeyInfo &) {}

}  // namespace libdnf5::repo
