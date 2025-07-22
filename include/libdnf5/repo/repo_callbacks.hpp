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

#ifndef LIBDNF5_REPO_REPO_CALLBACKS_HPP
#define LIBDNF5_REPO_REPO_CALLBACKS_HPP

#include "libdnf5/common/message.hpp"
#include "libdnf5/defs.h"

#include <string>
#include <vector>

namespace libdnf5::rpm {
class KeyInfo;
}


namespace libdnf5::repo {

/// Base class for repository callbacks.
/// To implement callbacks, inherit from this class and override virtual methods.
class LIBDNF_API RepoCallbacks {
public:
    explicit RepoCallbacks();
    RepoCallbacks(const RepoCallbacks &) = delete;
    RepoCallbacks(RepoCallbacks &&) = delete;
    virtual ~RepoCallbacks();

    RepoCallbacks & operator=(const RepoCallbacks &) = delete;
    RepoCallbacks & operator=(RepoCallbacks &&) = delete;

    /// OpenPGP key import callback. Allows one to confirm or deny the import.
    /// @param key_info The key that is about to be imported
    /// @return `true` to import the key, `false` to not import
    virtual bool repokey_import(const libdnf5::rpm::KeyInfo & key_info);
    /// Called on successful repo key import.
    /// @param key_info The key that was successfully imported
    virtual void repokey_imported(const libdnf5::rpm::KeyInfo & key_info);
};

/// @brief Extended repository callbacks with additional methods for support of key removal.
class LIBDNF_API RepoCallbacks2_1 : public RepoCallbacks {
public:
    explicit RepoCallbacks2_1();
    ~RepoCallbacks2_1();

    /// OpenPGP key remove callback. Allows one to confirm or deny the removal.
    /// @param key_info The key that is about to be removed
    /// @param removal_info Additional information about the key removal
    /// @return `true` to remove the key, `false` to not remove
    virtual bool repokey_remove(const libdnf5::rpm::KeyInfo & key_info, const libdnf5::Message & removal_info);
    /// Called on successful repo key removal.
    /// @param key_info The key that was successfully removed
    virtual void repokey_removed(const libdnf5::rpm::KeyInfo & key_info);
};

}  // namespace libdnf5::repo

#endif  // LIBDNF5_REPO_REPO_CALLBACKS_HPP
